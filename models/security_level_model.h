#ifndef SECURITY_LEVEL_MODEL_H
#define SECURITY_LEVEL_MODEL_H

#include <QString>
#include <QDateTime>

class SecurityLevel
{
public:
    SecurityLevel() : id(0), retention_days(30), timeout_minutes(30), is_active(true) {}

    // Getters
    int getId() const { return id; }
    QString getLevelCode() const { return level_code; }
    QString getLevelName() const { return level_name; }
    int getRetentionDays() const { return retention_days; }
    int getTimeoutMinutes() const { return timeout_minutes; }
    QString getDescription() const { return description; }
    bool isActive() const { return is_active; }
    QDateTime getCreatedAt() const { return created_at; }
    QDateTime getUpdatedAt() const { return updated_at; }

    // Setters
    void setId(int value) { id = value; }
    void setLevelCode(const QString& value) { level_code = value; }
    void setLevelName(const QString& value) { level_name = value; }
    void setRetentionDays(int value) { retention_days = value; }
    void setTimeoutMinutes(int value) { timeout_minutes = value; }
    void setDescription(const QString& value) { description = value; }
    void setActive(bool value) { is_active = value; }
    void setCreatedAt(const QDateTime& value) { created_at = value; }
    void setUpdatedAt(const QDateTime& value) { updated_at = value; }

    // Data members
    int id;
    QString level_code;
    QString level_name;
    int retention_days;
    int timeout_minutes;
    QString description;
    bool is_active;
    QDateTime created_at;
    QDateTime updated_at;
};

#endif // SECURITY_LEVEL_MODEL_H
