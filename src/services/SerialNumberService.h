#ifndef SERIALNUMBERSERVICE_H
#define SERIALNUMBERSERVICE_H

#include <QString>
#include <QDate>
#include <QList>
#include "../models/serial_number_config_model.h"
#include "../common/repository/serial_number_config_repository.h"

class SerialNumberService
{
public:
    // 单例获取方法
    static SerialNumberService& getInstance();

    // 配置管理
    bool createConfig(SerialNumberConfig& config, int operatorId);
    bool updateConfig(const SerialNumberConfig& config, int operatorId);
    bool deleteConfig(int configId, int operatorId);
    SerialNumberConfig getConfigById(int configId) const;
    SerialNumberConfig getConfigByType(int configType) const;
    QList<SerialNumberConfig> getAllConfigs() const;

    // 生成流水号
    // 格式: prefix + date + sequence (sequence会根据sequence_length补零)
    // 根据reset_period检查是否需要重置序列号
    QString generateSerialNumber(int configType);

    // 预览流水号格式
    QString previewSerialNumber(const SerialNumberConfig& config) const;

    // 重置序列号（手动）
    bool resetSequence(int configId, int operatorId);

    // 获取下一个序列号（不更新数据库，用于预览）
    int getNextSequence(int configType) const;

private:
    SerialNumberService();
    ~SerialNumberService();

    // 检查是否需要重置序列号
    bool shouldResetSequence(const SerialNumberConfig& config) const;

    // 格式化序列号（补零）
    QString formatSequence(int sequence, int length) const;

    // 生成日期部分
    QString generateDatePart(const QString& dateFormat) const;

    // 记录操作日志
    bool logOperation(int configId, int operatorId, const QString& operation, const QString& details) const;

    // 检查操作者权限
    bool checkOperatorPermission(int operatorId, const QString& operation) const;

    SerialNumberConfigRepository repository_;
};

#endif // SERIALNUMBERSERVICE_H
