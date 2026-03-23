#ifndef SENSITIVEWORDSSERVICE_H
#define SENSITIVEWORDSSERVICE_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

class SensitiveWordsService
{
public:
    // 单例获取方法
    static SensitiveWordsService& getInstance();

    // 检查文本中是否包含敏感词
    bool containsSensitiveWords(const QString &text) const;

    // 获取文本中的敏感词列表
    QStringList findSensitiveWords(const QString &text) const;

    // 替换敏感词（星号屏蔽）
    QString maskSensitiveWords(const QString &text) const;

    // 添加敏感词到词库
    bool addSensitiveWord(const QString &word);

    // 从词库移除敏感词
    bool removeSensitiveWord(const QString &word);

    // 获取所有敏感词
    QStringList getAllSensitiveWords() const;

    // 批量检查文本
    QList<bool> batchCheck(const QStringList &texts) const;

private:
    SensitiveWordsService();  // 私有构造函数，确保单例
    ~SensitiveWordsService();

    // 确保敏感词表存在
    bool ensureSensitiveWordsTableExists() const;

    // 从数据库或其他存储加载敏感词列表
    void loadSensitiveWords();

    // 保存敏感词列表
    bool saveSensitiveWords() const;

    // 内存中的敏感词列表
    QStringList m_sensitiveWords;
};

#endif // SENSITIVEWORDSSERVICE_H