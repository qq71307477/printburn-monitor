#include "SensitiveWordsService.h"
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

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
    // 初始化默认敏感词列表
    m_sensitiveWords = {
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

    // TODO: 从数据库或配置文件加载更多敏感词
    // 实际实现中应该从数据库加载敏感词列表
}

bool SensitiveWordsService::saveSensitiveWords() const
{
    // TODO: 保存敏感词列表到数据库或配置文件
    // 实际实现中应该保存到持久化存储
    return true;
}