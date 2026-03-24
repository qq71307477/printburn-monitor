#ifndef TASK_MODEL_H
#define TASK_MODEL_H

#include <string>
#include <ctime>
#include <QString>
#include <QDateTime>

class Task {
public:
    Task() : id(0), user_id(0), copies(0), approver_id(0), due_date(0), created_at(0), updated_at(0), completed_at(0), approval_time(0) {}

    // Getters
    int getId() const { return id; }
    int getUserId() const { return user_id; }
    QString getType() const { return QString::fromStdString(type); }
    QString getTitle() const { return QString::fromStdString(title); }
    QString getDescription() const { return QString::fromStdString(description); }
    QString getStatus() const { return QString::fromStdString(status); }
    QString getPriority() const { return QString::fromStdString(priority); }
    QString getFilePath() const { return QString::fromStdString(file_path); }
    int getCopies() const { return copies; }
    QDateTime getCreateTime() const { return QDateTime::fromTime_t(created_at); }
    QDateTime getUpdateTime() const { return QDateTime::fromTime_t(updated_at); }
    int getApproverId() const { return approver_id; }
    QString getApprovalStatus() const { return QString::fromStdString(approval_status); }
    QString getMediaType() const { return QString::fromStdString(media_type); }
    QString getSessionMode() const { return QString::fromStdString(session_mode); }
    QDateTime getApprovalTime() const { return QDateTime::fromTime_t(approval_time); }
    QString getApprovalReason() const { return QString::fromStdString(approval_reason); }
    QString getSerialNumber() const { return QString::fromStdString(serial_number); }

    // Setters
    void setId(int value) { id = value; }
    void setUserId(int value) { user_id = value; }
    void setType(const QString& value) { type = value.toStdString(); }
    void setTitle(const QString& value) { title = value.toStdString(); }
    void setDescription(const QString& value) { description = value.toStdString(); }
    void setStatus(const QString& value) { status = value.toStdString(); }
    void setPriority(const QString& value) { priority = value.toStdString(); }
    void setFilePath(const QString& value) { file_path = value.toStdString(); }
    void setCopies(int value) { copies = value; }
    void setCreateTime(const QDateTime& value) { created_at = value.toTime_t(); }
    void setUpdateTime(const QDateTime& value) { updated_at = value.toTime_t(); }
    void setApproverId(int value) { approver_id = value; }
    void setApprovalStatus(const QString& value) { approval_status = value.toStdString(); }
    void setMediaType(const QString& value) { media_type = value.toStdString(); }
    void setSessionMode(const QString& value) { session_mode = value.toStdString(); }
    void setApprovalTime(const QDateTime& value) { approval_time = value.toTime_t(); }
    void setApprovalReason(const QString& value) { approval_reason = value.toStdString(); }
    void setSerialNumber(const QString& value) { serial_number = value.toStdString(); }

    // Legacy struct members for backward compatibility
    int id;
    std::string title;
    std::string description;
    int assigned_user_id;
    int created_by_user_id;
    int device_id;
    std::string status;  // "pending", "in_progress", "completed", "cancelled"
    std::string priority;  // "low", "medium", "high", "critical"
    std::time_t due_date;
    std::time_t created_at;
    std::time_t updated_at;
    std::time_t completed_at;

private:
    // New fields for TaskService
    int user_id;
    std::string type;
    std::string file_path;
    int copies;
    int approver_id;
    std::string approval_status;
    std::string media_type;
    std::string session_mode;
    std::time_t approval_time;
    std::string approval_reason;
    std::string serial_number;
};

#endif // TASK_MODEL_H