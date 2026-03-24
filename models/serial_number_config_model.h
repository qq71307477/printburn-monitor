#ifndef SERIAL_NUMBER_CONFIG_MODEL_H
#define SERIAL_NUMBER_CONFIG_MODEL_H

#include <QString>
#include <QDate>
#include <QDateTime>

// 配置类型枚举
namespace SerialNumberConfigType {
    constexpr int PRINT = 0;  // 打印流水号
    constexpr int BURN = 1;   // 刻录流水号
}

// 重置周期枚举
namespace ResetPeriod {
    constexpr int DAILY = 0;   // 每日
    constexpr int MONTHLY = 1; // 每月
    constexpr int YEARLY = 2;  // 每年
}

class SerialNumberConfig {
public:
    SerialNumberConfig() :
        id(0),
        config_type(0),
        sequence_length(4),
        current_sequence(0),
        reset_period(0),
        is_active(true) {}

    // Getters
    int getId() const { return id; }
    int getConfigType() const { return config_type; }
    QString getPrefix() const { return prefix; }
    QString getDateFormat() const { return date_format; }
    int getSequenceLength() const { return sequence_length; }
    int getCurrentSequence() const { return current_sequence; }
    int getResetPeriod() const { return reset_period; }
    QDate getLastResetDate() const { return last_reset_date; }
    bool getIsActive() const { return is_active; }
    QDateTime getCreatedAt() const { return created_at; }
    QDateTime getUpdatedAt() const { return updated_at; }

    // Setters
    void setId(int value) { id = value; }
    void setConfigType(int value) { config_type = value; }
    void setPrefix(const QString& value) { prefix = value; }
    void setDateFormat(const QString& value) { date_format = value; }
    void setSequenceLength(int value) { sequence_length = value; }
    void setCurrentSequence(int value) { current_sequence = value; }
    void setResetPeriod(int value) { reset_period = value; }
    void setLastResetDate(const QDate& value) { last_reset_date = value; }
    void setIsActive(bool value) { is_active = value; }
    void setCreatedAt(const QDateTime& value) { created_at = value; }
    void setUpdatedAt(const QDateTime& value) { updated_at = value; }

    // Helper methods
    QString getConfigTypeName() const {
        switch (config_type) {
            case SerialNumberConfigType::PRINT: return "打印";
            case SerialNumberConfigType::BURN: return "刻录";
            default: return "未知";
        }
    }

    QString getResetPeriodName() const {
        switch (reset_period) {
            case ResetPeriod::DAILY: return "每日";
            case ResetPeriod::MONTHLY: return "每月";
            case ResetPeriod::YEARLY: return "每年";
            default: return "未知";
        }
    }

    // Legacy struct members for backward compatibility
    int id;
    int config_type;
    QString prefix;
    QString date_format;
    int sequence_length;
    int current_sequence;
    int reset_period;
    QDate last_reset_date;
    bool is_active;
    QDateTime created_at;
    QDateTime updated_at;
};

#endif // SERIAL_NUMBER_CONFIG_MODEL_H
