#ifndef DEVICE_MODEL_H
#define DEVICE_MODEL_H

#include <string>
#include <ctime>
#include <QString>
#include <QDateTime>

class Device {
public:
    Device() : id(0), assigned_user_id(0), is_monitored(false), assignment_date(0), last_seen(0), created_at(0), updated_at(0) {}

    // Getters
    int getId() const { return id; }
    QString getName() const { return QString::fromStdString(name); }
    QString getSerialNumber() const { return QString::fromStdString(device_id); }
    QString getDeviceType() const { return QString::fromStdString(device_type); }
    QString getIpAddress() const { return QString::fromStdString(ip_address); }
    QString getLocation() const { return QString::fromStdString(location); }
    int getAssignedUserId() const { return assigned_user_id; }
    QString getStatus() const { return QString::fromStdString(status); }
    QString getSpecifications() const { return QString::fromStdString(specifications); }
    bool isMonitored() const { return is_monitored; }
    QDateTime getLastSeen() const { return QDateTime::fromTime_t(last_seen); }
    QDateTime getCreateTime() const { return QDateTime::fromTime_t(created_at); }
    QDateTime getLastUpdated() const { return QDateTime::fromTime_t(updated_at); }
    QDateTime getAssignmentDate() const { return QDateTime::fromTime_t(assignment_date); }

    // Setters
    void setId(int value) { id = value; }
    void setName(const QString& value) { name = value.toStdString(); }
    void setSerialNumber(const QString& value) { device_id = value.toStdString(); }
    void setDeviceType(const QString& value) { device_type = value.toStdString(); }
    void setIpAddress(const QString& value) { ip_address = value.toStdString(); }
    void setLocation(const QString& value) { location = value.toStdString(); }
    void setAssignedUserId(int value) { assigned_user_id = value; }
    void setStatus(const QString& value) { status = value.toStdString(); }
    void setSpecifications(const QString& value) { specifications = value.toStdString(); }
    void setMonitored(bool value) { is_monitored = value; }
    void setLastSeen(const QDateTime& value) { last_seen = value.toTime_t(); }
    void setCreateTime(const QDateTime& value) { created_at = value.toTime_t(); }
    void setLastUpdated(const QDateTime& value) { updated_at = value.toTime_t(); }
    void setAssignmentDate(const QDateTime& value) { assignment_date = value.toTime_t(); }

    // Legacy struct members for backward compatibility
    int id;
    std::string name;
    std::string device_id;  // Unique identifier for the device
    std::string device_type;  // printer, scanner, etc.
    std::string ip_address;
    std::string location;
    int assigned_user_id;  // Currently assigned user
    std::string status;  // "available", "in_use", "maintenance", "offline"
    std::string specifications;  // Technical specs
    bool is_monitored;  // Whether the device is under security monitoring
    std::time_t assignment_date;
    std::time_t last_seen;
    std::time_t created_at;
    std::time_t updated_at;
};

#endif // DEVICE_MODEL_H