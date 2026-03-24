#include "FileEncryptionService.h"
#include <QFile>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

// 静态盐值，增加密钥派生的安全性
const QByteArray FileEncryptionService::SALT = QByteArray::fromHex("a1b2c3d4e5f607182930a1b2c3d4e5f6");

FileEncryptionService& FileEncryptionService::getInstance()
{
    static FileEncryptionService instance;
    return instance;
}

FileEncryptionService::FileEncryptionService()
    : m_masterKey("PrintBurnMonitor_MasterKey_2024")
{
}

FileEncryptionService::~FileEncryptionService()
{
}

QString FileEncryptionService::generateKey(int taskId)
{
    // 基于任务ID和系统主密钥生成唯一密钥
    QString combined = QString("%1_%2_%3")
        .arg(m_masterKey)
        .arg(taskId)
        .arg(QDateTime::currentDateTime().toSecsSinceEpoch());

    // 使用 SHA256 生成 32 字节（256位）密钥
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

QByteArray FileEncryptionService::deriveKey(const QString& password, int keyLength)
{
    // 使用 SHA256 和盐值派生密钥
    QByteArray combined = password.toUtf8() + SALT;

    // 多次迭代增加安全性
    QByteArray result = combined;
    for (int i = 0; i < 1000; ++i) {
        result = QCryptographicHash::hash(result, QCryptographicHash::Sha256);
    }

    // 如果需要不同长度，进行截断或扩展
    if (keyLength <= 32) {
        return result.left(keyLength);
    }

    // 对于超过 32 字节的密钥，使用额外的哈希扩展
    QByteArray extended = result;
    int iterations = (keyLength + 31) / 32;
    for (int i = 1; i < iterations; ++i) {
        QByteArray temp = result + QByteArray::number(i);
        extended.append(QCryptographicHash::hash(temp, QCryptographicHash::Sha256));
    }

    return extended.left(keyLength);
}

QByteArray FileEncryptionService::xorCipher(const QByteArray& data, const QByteArray& key)
{
    if (key.isEmpty()) {
        qWarning() << "FileEncryptionService: Key is empty";
        return QByteArray();
    }

    QByteArray result(data.size(), 0);
    int keySize = key.size();

    for (int i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ key[i % keySize];
    }

    return result;
}

QByteArray FileEncryptionService::encryptData(const QByteArray& data, const QString& key)
{
    if (data.isEmpty()) {
        qWarning() << "FileEncryptionService: Data is empty";
        return QByteArray();
    }

    // 派生加密密钥
    QByteArray derivedKey = deriveKey(key, 32);

    // 添加数据完整性校验
    QByteArray checksum = QCryptographicHash::hash(data, QCryptographicHash::Sha256).left(8);

    // 组合：校验值 + 原始数据
    QByteArray combined = checksum + data;

    // XOR 加密
    QByteArray encrypted = xorCipher(combined, derivedKey);

    // 添加版本标识（用于未来算法升级）
    QByteArray versionHeader = QByteArray::fromHex("01");  // 版本 1

    return versionHeader + encrypted;
}

QByteArray FileEncryptionService::decryptData(const QByteArray& data, const QString& key)
{
    if (data.size() < 10) {  // 最小：1字节版本 + 8字节校验 + 至少1字节数据
        qWarning() << "FileEncryptionService: Data too short for decryption";
        return QByteArray();
    }

    // 检查版本标识
    QByteArray versionHeader = data.left(1);
    if (versionHeader != QByteArray::fromHex("01")) {
        qWarning() << "FileEncryptionService: Unsupported encryption version";
        return QByteArray();
    }

    // 提取加密数据
    QByteArray encrypted = data.mid(1);

    // 派生解密密钥
    QByteArray derivedKey = deriveKey(key, 32);

    // XOR 解密
    QByteArray decrypted = xorCipher(encrypted, derivedKey);

    if (decrypted.size() < 8) {
        qWarning() << "FileEncryptionService: Decrypted data too short";
        return QByteArray();
    }

    // 分离校验值和原始数据
    QByteArray storedChecksum = decrypted.left(8);
    QByteArray originalData = decrypted.mid(8);

    // 验证数据完整性
    QByteArray computedChecksum = QCryptographicHash::hash(originalData, QCryptographicHash::Sha256).left(8);

    if (storedChecksum != computedChecksum) {
        qWarning() << "FileEncryptionService: Data integrity check failed";
        return QByteArray();
    }

    return originalData;
}

bool FileEncryptionService::encryptFile(const QString& inputPath, const QString& outputPath, const QString& key)
{
    // 检查源文件是否存在
    QFile inputFile(inputPath);
    if (!inputFile.exists()) {
        qWarning() << "FileEncryptionService: Input file does not exist:" << inputPath;
        return false;
    }

    // 打开源文件
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "FileEncryptionService: Cannot open input file for reading:" << inputPath;
        return false;
    }

    // 读取原始数据
    QByteArray originalData = inputFile.readAll();
    inputFile.close();

    if (originalData.isEmpty()) {
        qWarning() << "FileEncryptionService: Input file is empty:" << inputPath;
        return false;
    }

    // 加密数据
    QByteArray encryptedData = encryptData(originalData, key);
    if (encryptedData.isEmpty()) {
        qWarning() << "FileEncryptionService: Encryption failed for file:" << inputPath;
        return false;
    }

    // 创建输出文件
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << "FileEncryptionService: Cannot open output file for writing:" << outputPath;
        return false;
    }

    // 写入加密数据
    qint64 bytesWritten = outputFile.write(encryptedData);
    outputFile.close();

    if (bytesWritten != encryptedData.size()) {
        qWarning() << "FileEncryptionService: Failed to write all encrypted data to:" << outputPath;
        return false;
    }

    qDebug() << "FileEncryptionService: Successfully encrypted file:" << inputPath << "->" << outputPath;
    return true;
}

bool FileEncryptionService::decryptFile(const QString& inputPath, const QString& outputPath, const QString& key)
{
    // 检查加密文件是否存在
    QFile inputFile(inputPath);
    if (!inputFile.exists()) {
        qWarning() << "FileEncryptionService: Encrypted file does not exist:" << inputPath;
        return false;
    }

    // 打开加密文件
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "FileEncryptionService: Cannot open encrypted file for reading:" << inputPath;
        return false;
    }

    // 读取加密数据
    QByteArray encryptedData = inputFile.readAll();
    inputFile.close();

    if (encryptedData.isEmpty()) {
        qWarning() << "FileEncryptionService: Encrypted file is empty:" << inputPath;
        return false;
    }

    // 解密数据
    QByteArray decryptedData = decryptData(encryptedData, key);
    if (decryptedData.isEmpty()) {
        qWarning() << "FileEncryptionService: Decryption failed for file:" << inputPath;
        return false;
    }

    // 创建输出文件
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << "FileEncryptionService: Cannot open output file for writing:" << outputPath;
        return false;
    }

    // 写入解密数据
    qint64 bytesWritten = outputFile.write(decryptedData);
    outputFile.close();

    if (bytesWritten != decryptedData.size()) {
        qWarning() << "FileEncryptionService: Failed to write all decrypted data to:" << outputPath;
        return false;
    }

    qDebug() << "FileEncryptionService: Successfully decrypted file:" << inputPath << "->" << outputPath;
    return true;
}