#ifndef BARCODESERVICE_H
#define BARCODESERVICE_H

#include <QString>
#include <QPixmap>
#include <QImage>
#include <QList>
#include "../models/barcode_config_model.h"
#include "../models/task_model.h"

// 条码生成任务信息结构
struct BarcodeTaskInfo {
    QString serialNumber;       // 流水号
    QString customText;         // 自定义文本
    QString department;         // 提交人部门
    QString submitter;          // 提交人
    QString securityLevel;      // 密级
    int copies;                 // 份数
    QString filename;           // 文件名
};

class BarcodeService
{
public:
    // 单例获取方法
    static BarcodeService& getInstance();

    // 条形码生成 (Code128)
    // content: 要编码的内容
    // width: 条形码宽度（像素）
    // height: 条形码高度（像素）
    QPixmap generateBarcode(const QString& content, int width = 200, int height = 50);

    // 二维码生成
    // content: 要编码的内容
    // size: 二维码尺寸（像素，正方形）
    QPixmap generateQRCode(const QString& content, int size = 100);

    // 保存条形码到文件
    // 支持PNG和BMP格式
    bool saveBarcodeToFile(const QString& content, const QString& filePath,
                           int width = 200, int height = 50);

    // 保存二维码到文件
    // 支持PNG和BMP格式
    bool saveQRCodeToFile(const QString& content, const QString& filePath, int size = 100);

    // 根据任务和配置生成条码内容字符串
    QString generateBarcodeContent(const Task& task, const BarcodeConfig& config);
    QString generateBarcodeContent(const BarcodeTaskInfo& taskInfo, const BarcodeConfig& config);

    // 从任务对象提取条码任务信息
    BarcodeTaskInfo extractTaskInfo(const Task& task) const;

    // 配置管理
    bool createConfig(BarcodeConfig& config, int operatorId);
    bool updateConfig(const BarcodeConfig& config, int operatorId);
    bool deleteConfig(int configId, int operatorId);
    BarcodeConfig getConfigById(int configId) const;
    BarcodeConfig getConfigByType(int configType) const;
    QList<BarcodeConfig> getAllConfigs() const;

private:
    BarcodeService();
    ~BarcodeService();

    // 禁止拷贝
    BarcodeService(const BarcodeService&) = delete;
    BarcodeService& operator=(const BarcodeService&) = delete;

    // Code128编码相关
    QByteArray encodeCode128(const QString& content);
    QPixmap renderBarcode(const QByteArray& bars, int width, int height);

    // Code128编码表
    static const QString CODE128_PATTERNS[];
    static const int CODE128_START_B = 104;
    static const int CODE128_STOP = 106;

    // QR码简单实现（生成简单的黑白格子）
    QPixmap generateSimpleQR(const QString& content, int size);

    // 记录操作日志
    bool logOperation(int configId, int operatorId, const QString& operation,
                      const QString& details) const;

    // 检查操作者权限
    bool checkOperatorPermission(int operatorId, const QString& operation) const;

    // 配置存储（内存中，后续可对接数据库）
    QList<BarcodeConfig> configs_;
    int nextConfigId_;
};

#endif // BARCODESERVICE_H