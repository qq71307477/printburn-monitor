#include "SerialNumberService.h"
#include <QDebug>

// 单例实现
SerialNumberService& SerialNumberService::getInstance()
{
    static SerialNumberService instance;
    return instance;
}

SerialNumberService::SerialNumberService()
    : repository_()
{
    // 初始化时创建表
    repository_.create_table();

    // 确保打印和刻录的默认配置存在
    repository_.getOrCreateDefault(SerialNumberConfigType::PRINT);
    repository_.getOrCreateDefault(SerialNumberConfigType::BURN);
}

SerialNumberService::~SerialNumberService()
{
}

bool SerialNumberService::createConfig(SerialNumberConfig& config, int operatorId)
{
    if (!checkOperatorPermission(operatorId, "create_serial_config")) {
        qDebug() << "Permission denied for creating serial config";
        return false;
    }

    // 检查是否已存在相同类型的配置
    if (repository_.existsByType(config.getConfigType())) {
        qDebug() << "Serial config for type" << config.getConfigType() << "already exists";
        return false;
    }

    if (repository_.create(config)) {
        logOperation(config.getId(), operatorId, "CREATE", "Created new serial number config");
        return true;
    }
    return false;
}

bool SerialNumberService::updateConfig(const SerialNumberConfig& config, int operatorId)
{
    if (!checkOperatorPermission(operatorId, "update_serial_config")) {
        qDebug() << "Permission denied for updating serial config";
        return false;
    }

    if (repository_.update(config)) {
        logOperation(config.getId(), operatorId, "UPDATE", "Updated serial number config");
        return true;
    }
    return false;
}

bool SerialNumberService::deleteConfig(int configId, int operatorId)
{
    if (!checkOperatorPermission(operatorId, "delete_serial_config")) {
        qDebug() << "Permission denied for deleting serial config";
        return false;
    }

    if (repository_.remove(configId)) {
        logOperation(configId, operatorId, "DELETE", "Deleted serial number config");
        return true;
    }
    return false;
}

SerialNumberConfig SerialNumberService::getConfigById(int configId) const
{
    return repository_.findById(configId);
}

SerialNumberConfig SerialNumberService::getConfigByType(int configType) const
{
    return repository_.findByType(configType);
}

QList<SerialNumberConfig> SerialNumberService::getAllConfigs() const
{
    return repository_.findAll();
}

QString SerialNumberService::generateSerialNumber(int configType)
{
    SerialNumberConfig config = repository_.findByType(configType);

    if (config.getId() == 0) {
        qDebug() << "Serial config not found for type:" << configType;
        return QString();
    }

    if (!config.getIsActive()) {
        qDebug() << "Serial config is inactive for type:" << configType;
        return QString();
    }

    // 检查是否需要重置序列号
    if (shouldResetSequence(config)) {
        config.setCurrentSequence(0);
        config.setLastResetDate(QDate::currentDate());
        repository_.update(config);
    }

    // 生成序列号各部分
    QString prefix = config.getPrefix();
    QString datePart = generateDatePart(config.getDateFormat());
    int nextSequence = config.getCurrentSequence() + 1;
    QString sequencePart = formatSequence(nextSequence, config.getSequenceLength());

    // 更新当前序列号
    repository_.updateCurrentSequence(config.getId(), nextSequence);

    // 返回完整流水号
    return prefix + datePart + sequencePart;
}

QString SerialNumberService::previewSerialNumber(const SerialNumberConfig& config) const
{
    QString prefix = config.getPrefix();
    QString datePart = generateDatePart(config.getDateFormat());
    int nextSeq = config.getCurrentSequence() + 1;
    QString sequencePart = formatSequence(nextSeq, config.getSequenceLength());

    return prefix + datePart + sequencePart;
}

bool SerialNumberService::resetSequence(int configId, int operatorId)
{
    if (!checkOperatorPermission(operatorId, "reset_serial_sequence")) {
        qDebug() << "Permission denied for resetting sequence";
        return false;
    }

    SerialNumberConfig config = repository_.findById(configId);
    if (config.getId() == 0) {
        return false;
    }

    config.setCurrentSequence(0);
    config.setLastResetDate(QDate::currentDate());

    if (repository_.update(config)) {
        logOperation(configId, operatorId, "RESET_SEQUENCE", "Manually reset sequence number");
        return true;
    }
    return false;
}

int SerialNumberService::getNextSequence(int configType) const
{
    SerialNumberConfig config = repository_.findByType(configType);
    if (config.getId() == 0) {
        return -1;
    }
    return config.getCurrentSequence() + 1;
}

bool SerialNumberService::shouldResetSequence(const SerialNumberConfig& config) const
{
    QDate lastReset = config.getLastResetDate();
    QDate today = QDate::currentDate();

    if (!lastReset.isValid()) {
        return true;
    }

    switch (config.getResetPeriod()) {
        case ResetPeriod::DAILY:
            return lastReset < today;

        case ResetPeriod::MONTHLY:
            return lastReset.year() < today.year() ||
                   (lastReset.year() == today.year() && lastReset.month() < today.month());

        case ResetPeriod::YEARLY:
            return lastReset.year() < today.year();

        default:
            return false;
    }
}

QString SerialNumberService::formatSequence(int sequence, int length) const
{
    QString seqStr = QString::number(sequence);
    // 左补零到指定长度
    while (seqStr.length() < length) {
        seqStr = "0" + seqStr;
    }
    return seqStr;
}

QString SerialNumberService::generateDatePart(const QString& dateFormat) const
{
    return QDate::currentDate().toString(dateFormat);
}

bool SerialNumberService::logOperation(int configId, int operatorId, const QString& operation, const QString& details) const
{
    // TODO: 实现日志记录
    // 这里可以调用 LogAuditService 来记录操作
    Q_UNUSED(configId)
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    Q_UNUSED(details)
    return true;
}

bool SerialNumberService::checkOperatorPermission(int operatorId, const QString& operation) const
{
    // TODO: 实现权限检查
    // 暂时返回 true，允许所有操作
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    return true;
}
