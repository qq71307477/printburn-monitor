#include "BarcodeService.h"
#include <QPainter>
#include <QFile>
#include <QDebug>
#include <QDate>
#include <cmath>

// 静态成员定义
const int BarcodeService::CODE128_START_B;
const int BarcodeService::CODE128_STOP;

// Code128条码模式定义
// 每个字符对应一个11模块宽度的条码模式（窄条=1，宽条=3）
const QString BarcodeService::CODE128_PATTERNS[] = {
    "11011001100", "11001101100", "11001100110", "10010011000", "10010001100", // 0-4
    "10001001100", "10011001000", "10011000100", "10001100100", "11001001000", // 5-9
    "11001000100", "11000100100", "10110011100", "10011011100", "10011001110", // 10-14
    "10111001100", "10011101100", "10011100110", "11001110010", "11001011100", // 15-19
    "11001001110", "11011100100", "11001110100", "11101101110", "11101001100", // 20-24
    "11100101100", "11100100110", "11101100100", "11100110100", "11100110010", // 25-29
    "11011011000", "11011000110", "11000110110", "10100011000", "10001011000", // 30-34
    "10001000110", "10110001000", "10001101000", "10001100010", "11010001000", // 35-39
    "11000101000", "11000100010", "10110111000", "10110001110", "10001101110", // 40-44
    "10111011000", "10111000110", "10001110110", "11101110110", "11010001110", // 45-49
    "11000101110", "11011101000", "11011100010", "11011101110", "11101011000", // 50-54
    "11101000110", "11100010110", "11101101000", "11101100010", "11100011010", // 55-59
    "11101111010", "11001000010", "11110001010", "10100110000", "10100001100", // 60-64
    "10010110000", "10010000110", "10000101100", "10000100110", "10110010000", // 65-69
    "10110000100", "10011010000", "10011000010", "10000110100", "10000110010", // 70-74
    "11000010010", "11001010000", "11110111010", "11000010100", "10001111010", // 75-79
    "10100111100", "10010111100", "10010011110", "10111100100", "10011110100", // 80-84
    "10011110010", "11110100100", "11110010100", "11110010010", "11011011110", // 85-89
    "11011110110", "11110110110", "10101111000", "10100011110", "10001011110", // 90-94
    "10111101000", "10111100010", "11110101000", "11110100010", "10111011110", // 95-99
    "10111101110", "11101011110", "11110101110", "11010000100", "11010010000", // 100-104 (Code B: 100=Code B, 104=Start B)
    "11010011100", "1100011101011"  // 105=Start C, 106=Stop
};

// 单例实现
BarcodeService& BarcodeService::getInstance()
{
    static BarcodeService instance;
    return instance;
}

BarcodeService::BarcodeService()
    : nextConfigId_(1)
{
    // 初始化默认配置
    BarcodeConfig printConfig;
    printConfig.setId(nextConfigId_++);
    printConfig.setConfigType(0);  // 打印
    printConfig.setBarcodeType(BarcodeType::BARCODE);
    printConfig.setBarcodeWidth(200);
    printConfig.setBarcodeHeight(50);
    printConfig.setQrcodeSize(100);
    printConfig.setSelectedFields({BarcodeField::SERIAL_NUMBER, BarcodeField::SUBMITTER, BarcodeField::COPIES});
    printConfig.setIsActive(true);
    printConfig.setCreatedAt(QDateTime::currentDateTime());
    printConfig.setUpdatedAt(QDateTime::currentDateTime());
    configs_.append(printConfig);

    BarcodeConfig burnConfig;
    burnConfig.setId(nextConfigId_++);
    burnConfig.setConfigType(1);  // 刻录
    burnConfig.setBarcodeType(BarcodeType::BARCODE);
    burnConfig.setBarcodeWidth(200);
    burnConfig.setBarcodeHeight(50);
    burnConfig.setQrcodeSize(100);
    burnConfig.setSelectedFields({BarcodeField::SERIAL_NUMBER, BarcodeField::SUBMITTER, BarcodeField::SECURITY_LEVEL});
    burnConfig.setIsActive(true);
    burnConfig.setCreatedAt(QDateTime::currentDateTime());
    burnConfig.setUpdatedAt(QDateTime::currentDateTime());
    configs_.append(burnConfig);
}

BarcodeService::~BarcodeService()
{
}

// ============== Code128 条形码编码 ==============

QPixmap BarcodeService::generateBarcode(const QString& content, int width, int height)
{
    if (content.isEmpty()) {
        qWarning() << "Barcode content is empty";
        return QPixmap();
    }

    // 编码为Code128格式
    QByteArray bars = encodeCode128(content);
    if (bars.isEmpty()) {
        qWarning() << "Failed to encode Code128 for content:" << content;
        return QPixmap();
    }

    return renderBarcode(bars, width, height);
}

QByteArray BarcodeService::encodeCode128(const QString& content)
{
    QByteArray result;
    QList<int> values;

    // 开始符 (Start B) - 支持ASCII字符
    values.append(CODE128_START_B);

    // 编码每个字符
    for (int i = 0; i < content.length(); ++i) {
        QChar ch = content[i];
        int code = ch.unicode();

        // Code B 字符集支持 ASCII 32-127
        if (code >= 32 && code <= 127) {
            values.append(code - 32);
        } else if (code < 32) {
            // 控制字符需要使用Code A或转义，这里简化处理
            values.append(code);
        } else {
            // 超出范围的字符，使用问号替代
            values.append('?');
        }
    }

    // 计算校验和
    int checkSum = CODE128_START_B;  // 开始符值
    for (int i = 0; i < values.size() - 1; ++i) {
        checkSum += (i + 1) * values[i + 1];
    }
    checkSum = checkSum % 103;
    values.append(checkSum);

    // 结束符 (Stop)
    values.append(CODE128_STOP);

    // 转换为条码模式字符串
    for (int value : values) {
        if (value >= 0 && value <= 106) {
            result.append(CODE128_PATTERNS[value]);
        }
    }

    return result;
}

QPixmap BarcodeService::renderBarcode(const QByteArray& bars, int width, int height)
{
    if (bars.isEmpty()) {
        return QPixmap();
    }

    // 计算条码总模块数
    int modules = bars.length();

    // 计算每个模块的宽度
    int moduleWidth = qMax(1, width / modules);
    int actualWidth = moduleWidth * modules;

    // 创建图像
    QImage image(actualWidth, height, QImage::Format_RGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setPen(Qt::NoPen);

    int x = 0;
    for (int i = 0; i < bars.length(); ++i) {
        char c = bars[i];
        if (c == '1') {
            // 黑条
            painter.setBrush(Qt::black);
            painter.drawRect(x, 0, moduleWidth, height);
        }
        // '0' 是空白，不需要绘制
        x += moduleWidth;
    }

    painter.end();

    return QPixmap::fromImage(image);
}

// ============== QR码生成（简化实现）==============

QPixmap BarcodeService::generateQRCode(const QString& content, int size)
{
    // 简化的QR码实现
    // 注意：这是一个简化版本，不能生成真正的QR码
    // 生产环境建议使用第三方库如 QZXing 或 QtQR

    return generateSimpleQR(content, size);
}

QPixmap BarcodeService::generateSimpleQR(const QString& content, int size)
{
    // 使用简单的哈希算法生成伪随机图案
    // 这不是真正的QR码，只是演示用的简化实现

    QImage image(size, size, QImage::Format_RGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    // QR码定位图案（三个角的大方块）
    int finderSize = size / 7;
    int quietZone = size / 14;

    // 左上角定位图案
    painter.drawRect(quietZone, quietZone, finderSize, finderSize);
    painter.setBrush(Qt::white);
    painter.drawRect(quietZone + finderSize/5, quietZone + finderSize/5,
                     finderSize * 3/5, finderSize * 3/5);
    painter.setBrush(Qt::black);
    painter.drawRect(quietZone + finderSize*2/5, quietZone + finderSize*2/5,
                     finderSize/5, finderSize/5);

    // 右上角定位图案
    painter.setBrush(Qt::black);
    painter.drawRect(size - quietZone - finderSize, quietZone, finderSize, finderSize);
    painter.setBrush(Qt::white);
    painter.drawRect(size - quietZone - finderSize + finderSize/5,
                     quietZone + finderSize/5,
                     finderSize * 3/5, finderSize * 3/5);
    painter.setBrush(Qt::black);
    painter.drawRect(size - quietZone - finderSize + finderSize*2/5,
                     quietZone + finderSize*2/5,
                     finderSize/5, finderSize/5);

    // 左下角定位图案
    painter.setBrush(Qt::black);
    painter.drawRect(quietZone, size - quietZone - finderSize, finderSize, finderSize);
    painter.setBrush(Qt::white);
    painter.drawRect(quietZone + finderSize/5,
                     size - quietZone - finderSize + finderSize/5,
                     finderSize * 3/5, finderSize * 3/5);
    painter.setBrush(Qt::black);
    painter.drawRect(quietZone + finderSize*2/5,
                     size - quietZone - finderSize + finderSize*2/5,
                     finderSize/5, finderSize/5);

    // 数据区域 - 使用内容的哈希值生成伪随机图案
    uint hash = qHash(content);
    int moduleSize = (size - 2 * quietZone - finderSize) / 25;
    if (moduleSize < 1) moduleSize = 1;

    painter.setBrush(Qt::black);
    for (int row = 0; row < 25; ++row) {
        for (int col = 0; col < 25; ++col) {
            // 跳过定位图案区域
            if ((row < 8 && col < 8) ||
                (row < 8 && col > 16) ||
                (row > 16 && col < 8)) {
                continue;
            }

            // 使用哈希值决定是否填充
            hash = hash * 1103515245 + 12345;
            if ((hash >> 16) & 1) {
                int x = quietZone + finderSize + col * moduleSize;
                int y = quietZone + finderSize + row * moduleSize;
                painter.drawRect(x, y, moduleSize, moduleSize);
            }
        }
    }

    painter.end();

    return QPixmap::fromImage(image);
}

// ============== 文件保存 ==============

bool BarcodeService::saveBarcodeToFile(const QString& content, const QString& filePath,
                                        int width, int height)
{
    QPixmap barcode = generateBarcode(content, width, height);
    if (barcode.isNull()) {
        return false;
    }

    // 根据文件扩展名确定格式
    QString format = "PNG";
    if (filePath.endsWith(".bmp", Qt::CaseInsensitive)) {
        format = "BMP";
    }

    return barcode.save(filePath, format.toUtf8().constData());
}

bool BarcodeService::saveQRCodeToFile(const QString& content, const QString& filePath, int size)
{
    QPixmap qrCode = generateQRCode(content, size);
    if (qrCode.isNull()) {
        return false;
    }

    // 根据文件扩展名确定格式
    QString format = "PNG";
    if (filePath.endsWith(".bmp", Qt::CaseInsensitive)) {
        format = "BMP";
    }

    return qrCode.save(filePath, format.toUtf8().constData());
}

// ============== 条码内容生成 ==============

QString BarcodeService::generateBarcodeContent(const Task& task, const BarcodeConfig& config)
{
    BarcodeTaskInfo taskInfo = extractTaskInfo(task);
    return generateBarcodeContent(taskInfo, config);
}

QString BarcodeService::generateBarcodeContent(const BarcodeTaskInfo& taskInfo,
                                                const BarcodeConfig& config)
{
    QStringList parts;

    // 流水号是必选的
    parts.append(taskInfo.serialNumber);

    // 按配置顺序添加其他字段
    QList<int> fields = config.getSelectedFields();
    for (int field : fields) {
        switch (field) {
            case BarcodeField::SERIAL_NUMBER:
                // 已添加，跳过
                break;
            case BarcodeField::CUSTOM_TEXT:
                if (!taskInfo.customText.isEmpty()) {
                    parts.append(taskInfo.customText);
                }
                break;
            case BarcodeField::DEPARTMENT:
                if (!taskInfo.department.isEmpty()) {
                    parts.append(taskInfo.department);
                }
                break;
            case BarcodeField::SUBMITTER:
                if (!taskInfo.submitter.isEmpty()) {
                    parts.append(taskInfo.submitter);
                }
                break;
            case BarcodeField::SECURITY_LEVEL:
                if (!taskInfo.securityLevel.isEmpty()) {
                    parts.append(taskInfo.securityLevel);
                }
                break;
            case BarcodeField::COPIES:
                parts.append(QString::number(taskInfo.copies));
                break;
            case BarcodeField::FILENAME:
                if (!taskInfo.filename.isEmpty()) {
                    parts.append(taskInfo.filename);
                }
                break;
        }
    }

    return parts.join("|");
}

BarcodeTaskInfo BarcodeService::extractTaskInfo(const Task& task) const
{
    BarcodeTaskInfo info;

    // 从Task对象提取信息
    // 注意：Task模型可能需要扩展以包含所有这些字段
    info.serialNumber = QString::number(task.getId());  // 使用任务ID作为临时流水号
    info.filename = task.getTitle();  // 使用标题作为文件名
    info.copies = task.getCopies();

    // 其他字段需要从扩展的Task模型或关联表中获取
    // 这里设置默认值，实际使用时需要完善
    info.customText = "";
    info.department = "";
    info.submitter = "";
    info.securityLevel = "";

    return info;
}

// ============== 配置管理 ==============

bool BarcodeService::createConfig(BarcodeConfig& config, int operatorId)
{
    if (!checkOperatorPermission(operatorId, "create_barcode_config")) {
        qDebug() << "Permission denied for creating barcode config";
        return false;
    }

    config.setId(nextConfigId_++);
    config.setCreatedAt(QDateTime::currentDateTime());
    config.setUpdatedAt(QDateTime::currentDateTime());
    configs_.append(config);

    logOperation(config.getId(), operatorId, "CREATE", "Created new barcode config");
    return true;
}

bool BarcodeService::updateConfig(const BarcodeConfig& config, int operatorId)
{
    if (!checkOperatorPermission(operatorId, "update_barcode_config")) {
        qDebug() << "Permission denied for updating barcode config";
        return false;
    }

    for (int i = 0; i < configs_.size(); ++i) {
        if (configs_[i].getId() == config.getId()) {
            configs_[i] = config;
            configs_[i].setUpdatedAt(QDateTime::currentDateTime());
            logOperation(config.getId(), operatorId, "UPDATE", "Updated barcode config");
            return true;
        }
    }

    return false;
}

bool BarcodeService::deleteConfig(int configId, int operatorId)
{
    if (!checkOperatorPermission(operatorId, "delete_barcode_config")) {
        qDebug() << "Permission denied for deleting barcode config";
        return false;
    }

    for (int i = 0; i < configs_.size(); ++i) {
        if (configs_[i].getId() == configId) {
            configs_.removeAt(i);
            logOperation(configId, operatorId, "DELETE", "Deleted barcode config");
            return true;
        }
    }

    return false;
}

BarcodeConfig BarcodeService::getConfigById(int configId) const
{
    for (const BarcodeConfig& config : configs_) {
        if (config.getId() == configId) {
            return config;
        }
    }
    return BarcodeConfig();
}

BarcodeConfig BarcodeService::getConfigByType(int configType) const
{
    for (const BarcodeConfig& config : configs_) {
        if (config.getConfigType() == configType && config.getIsActive()) {
            return config;
        }
    }
    return BarcodeConfig();
}

QList<BarcodeConfig> BarcodeService::getAllConfigs() const
{
    return configs_;
}

// ============== 辅助方法 ==============

bool BarcodeService::logOperation(int configId, int operatorId, const QString& operation,
                                   const QString& details) const
{
    // TODO: 实现日志记录
    // 这里可以调用 LogAuditService 来记录操作
    Q_UNUSED(configId)
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    Q_UNUSED(details)
    return true;
}

bool BarcodeService::checkOperatorPermission(int operatorId, const QString& operation) const
{
    // TODO: 实现权限检查
    // 暂时返回 true，允许所有操作
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    return true;
}