#ifndef ROLE_MODEL_H
#define ROLE_MODEL_H

#include <string>
#include <ctime>
#include <QString>
#include <QDateTime>

class Role {
public:
    Role() : id(0), is_active(true), is_system(false), created_at(0), updated_at(0) {}

    // Getters
    int getId() const { return id; }
    QString getName() const { return QString::fromStdString(name); }
    QString getDescription() const { return QString::fromStdString(description); }
    QString getPermissions() const { return QString::fromStdString(permissions); }
    bool isActive() const { return is_active; }
    bool getIsSystem() const { return is_system; }
    QDateTime getCreateTime() const { return QDateTime::fromTime_t(created_at); }
    QDateTime getUpdateTime() const { return QDateTime::fromTime_t(updated_at); }

    // Setters
    void setId(int value) { id = value; }
    void setName(const QString& value) { name = value.toStdString(); }
    void setDescription(const QString& value) { description = value.toStdString(); }
    void setPermissions(const QString& value) { permissions = value.toStdString(); }
    void setActive(bool value) { is_active = value; }
    void setIsSystem(bool value) { is_system = value; }
    void setCreateTime(const QDateTime& value) { created_at = value.toTime_t(); }
    void setUpdateTime(const QDateTime& value) { updated_at = value.toTime_t(); }

    // Legacy struct members for backward compatibility
    int id;
    std::string name;
    std::string description;
    std::string permissions;
    bool is_active;
    bool is_system;
    std::time_t created_at;
    std::time_t updated_at;
};

#endif // ROLE_MODEL_H