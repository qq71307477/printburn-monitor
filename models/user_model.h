#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <string>
#include <ctime>
#include <QString>
#include <QDateTime>

class User {
public:
    User() : id(0), role_id(0), department_id(0), is_active(true) {}

    // Getters
    int getId() const { return id; }
    QString getUsername() const { return QString::fromStdString(username); }
    QString getPassword() const { return QString::fromStdString(password_hash); }
    QString getEmail() const { return QString::fromStdString(email); }
    int getRoleId() const { return role_id; }
    int getDepartmentId() const { return department_id; }
    QString getFirstName() const { return QString::fromStdString(first_name); }
    QString getLastName() const { return QString::fromStdString(last_name); }
    QString getPhone() const { return QString::fromStdString(phone); }
    bool isActive() const { return is_active; }
    QString getStatus() const { return is_active ? "ACTIVE" : "INACTIVE"; }
    QDateTime getCreateTime() const { return QDateTime::fromTime_t(created_at); }
    QDateTime getUpdateTime() const { return QDateTime::fromTime_t(updated_at); }

    // Setters
    void setId(int value) { id = value; }
    void setUsername(const QString& value) { username = value.toStdString(); }
    void setPassword(const QString& value) { password_hash = value.toStdString(); }
    void setEmail(const QString& value) { email = value.toStdString(); }
    void setRoleId(int value) { role_id = value; }
    void setDepartmentId(int value) { department_id = value; }
    void setFirstName(const QString& value) { first_name = value.toStdString(); }
    void setLastName(const QString& value) { last_name = value.toStdString(); }
    void setPhone(const QString& value) { phone = value.toStdString(); }
    void setActive(bool value) { is_active = value; }
    void setStatus(const QString& value) { is_active = (value.toUpper() == "ACTIVE"); }
    void setRole(const QString& value) { Q_UNUSED(value) }
    void setCreateTime(const QDateTime& value) { created_at = value.toTime_t(); }
    void setUpdateTime(const QDateTime& value) { updated_at = value.toTime_t(); }

    // Legacy struct members for backward compatibility
    int id;
    std::string username;
    std::string password_hash;
    std::string email;
    int role_id;
    int department_id;
    std::string first_name;
    std::string last_name;
    std::string phone;
    bool is_active;
    std::time_t created_at;
    std::time_t updated_at;
};

#endif // USER_MODEL_H