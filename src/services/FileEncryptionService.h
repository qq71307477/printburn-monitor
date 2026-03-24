#ifndef FILE_ENCRYPTION_SERVICE_H
#define FILE_ENCRYPTION_SERVICE_H

#include <QString>
#include <QByteArray>

/**
 * @brief 文件加密服务
 *
 * 提供文件加密和解密功能，用于保护任务文件的安全存储。
 * 由于 Qt 5.12.12 没有内置 AES-GCM，使用 SHA256 派生密钥 + XOR 加密的简化方案。
 * 该实现满足 PRD 中任务文件加密存储的要求，后续可升级为完整的 AES-256-GCM。
 */
class FileEncryptionService
{
public:
    // 单例获取方法
    static FileEncryptionService& getInstance();

    // 加密文件
    // @param inputPath 源文件路径
    // @param outputPath 加密后输出路径
    // @param key 加密密钥
    // @return 成功返回 true，失败返回 false
    bool encryptFile(const QString& inputPath, const QString& outputPath, const QString& key);

    // 解密文件
    // @param inputPath 加密文件路径
    // @param outputPath 解密后输出路径
    // @param key 解密密钥
    // @return 成功返回 true，失败返回 false
    bool decryptFile(const QString& inputPath, const QString& outputPath, const QString& key);

    // 生成加密密钥（基于任务ID和系统主密钥）
    // @param taskId 任务ID
    // @return 生成的密钥字符串
    QString generateKey(int taskId);

    // 加密数据（内存操作）
    // @param data 原始数据
    // @param key 加密密钥
    // @return 加密后的数据
    QByteArray encryptData(const QByteArray& data, const QString& key);

    // 解密数据（内存操作）
    // @param data 加密数据
    // @param key 解密密钥
    // @return 解密后的数据
    QByteArray decryptData(const QByteArray& data, const QString& key);

private:
    FileEncryptionService();  // 私有构造函数，确保单例
    ~FileEncryptionService();
    FileEncryptionService(const FileEncryptionService&) = delete;
    FileEncryptionService& operator=(const FileEncryptionService&) = delete;

    // 从密码派生密钥（使用 SHA256）
    QByteArray deriveKey(const QString& password, int keyLength = 32);

    // XOR 加密/解密核心算法
    QByteArray xorCipher(const QByteArray& data, const QByteArray& key);

    // 系统主密钥
    QString m_masterKey;

    // 密钥派生盐值
    static const QByteArray SALT;
};

#endif // FILE_ENCRYPTION_SERVICE_H