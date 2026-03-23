#include "SensitiveWordsService.h"
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// 静态实例
static SensitiveWordsService* instance = nullptr;

SensitiveWordsService& SensitiveWordsService::getInstance()
{
    if (!instance) {
        instance = new SensitiveWordsService();
    }
    return *instance;
}

SensitiveWordsService::SensitiveWordsService()
{
    ensureSensitiveWordsTableExists();
    loadSensitiveWords();
}

SensitiveWordsService::~SensitiveWordsService()
{
    // 析构函数
}

bool SensitiveWordsService::containsSensitiveWords(const QString &text) const
{
    for (const QString &word : m_sensitiveWords) {
        if (text.contains(word, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

QStringList SensitiveWordsService::findSensitiveWords(const QString &text) const
{
    QStringList foundWords;
    for (const QString &word : m_sensitiveWords) {
        if (text.contains(word, Qt::CaseInsensitive) && !foundWords.contains(word, Qt::CaseInsensitive)) {
            foundWords.append(word);
        }
    }
    return foundWords;
}

QString SensitiveWordsService::maskSensitiveWords(const QString &text) const
{
    QString result = text;
    for (const QString &word : m_sensitiveWords) {
        // 使用正则表达式进行替换，忽略大小写
        QRegularExpression regex(QRegularExpression::escape(word), QRegularExpression::CaseInsensitiveOption);
        QString mask(word.length(), '*');
        result.replace(regex, mask);
    }
    return result;
}

bool SensitiveWordsService::addSensitiveWord(const QString &word)
{
    if (word.isEmpty() || m_sensitiveWords.contains(word, Qt::CaseInsensitive)) {
        return false;
    }

    m_sensitiveWords.append(word);
    return saveSensitiveWords(); // 保存到持久化存储
}

bool SensitiveWordsService::removeSensitiveWord(const QString &word)
{
    int index = -1;
    for (int i = 0; i < m_sensitiveWords.size(); ++i) {
        if (m_sensitiveWords.at(i).compare(word, Qt::CaseInsensitive) == 0) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        m_sensitiveWords.removeAt(index);
        return saveSensitiveWords(); // 保存到持久化存储
    }

    return false;
}

QStringList SensitiveWordsService::getAllSensitiveWords() const
{
    return m_sensitiveWords;
}

QList<bool> SensitiveWordsService::batchCheck(const QStringList &texts) const
{
    QList<bool> results;
    for (const QString &text : texts) {
        results.append(containsSensitiveWords(text));
    }
    return results;
}

void SensitiveWordsService::loadSensitiveWords()
{
    // 默认敏感词列表
    QStringList defaultWords = {
        "机密",
        "绝密",
        "内部资料",
        "国家秘密",
        "秘密",
        "受限信息",
        "禁止传播",
        "严禁外传",
        "私人信息",
        "隐私信息"
    };

    // 从数据库加载敏感词
    QSqlQuery query("SELECT word FROM sensitive_words ORDER BY id");
    if (query.exec()) {
        m_sensitiveWords.clear();
        while (query.next()) {
            QString word = query.value(0).toString();
            if (!word.isEmpty()) {
                m_sensitiveWords.append(word);
            }
        }
    }

    // 如果数据库为空，初始化默认敏感词
    if (m_sensitiveWords.isEmpty()) {
        m_sensitiveWords = defaultWords;
        // 将默认敏感词插入数据库
        for (const QString &word : defaultWords) {
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO sensitive_words (word) VALUES (?)");
            insertQuery.addBindValue(word);
            insertQuery.exec();
        }
    }
}

bool SensitiveWordsService::saveSensitiveWords() const
{
    // 清空数据库中的敏感词
    QSqlQuery clearQuery;
    if (!clearQuery.exec("DELETE FROM sensitive_words")) {
        qWarning() << "Failed to clear sensitive_words table:" << clearQuery.lastError().text();
        return false;
    }

    // 插入所有敏感词
    for (const QString &word : m_sensitiveWords) {
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO sensitive_words (word) VALUES (?)");
        insertQuery.addBindValue(word);
        if (!insertQuery.exec()) {
            qWarning() << "Failed to insert sensitive word:" << word << insertQuery.lastError().text();
            return false;
        }
    }

    return true;
}

bool SensitiveWordsService::ensureSensitiveWordsTableExists() const
{
    QSqlQuery query(
        "CREATE TABLE IF NOT EXISTS sensitive_words ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word TEXT NOT NULL UNIQUE"
        ");"
    );

    if (!query.exec()) {
        qWarning() << "Failed to create sensitive_words table:" << query.lastError().text();
        return false;
    }

    return true;
}