#ifndef APPROVER_CONFIG_MODEL_H
#define APPROVER_CONFIG_MODEL_H

#include <QString>
#include <QDateTime>

class ApproverConfig {
public:
    ApproverConfig()
        : id(0)
        , task_type(0)
        , approver_role_id(0)
        , min_security_level_id(0)
        , max_security_level_id(0)
        , is_active(true)
    {}

    // Getters
    int getId() const { return id; }
    int getTaskType() const { return task_type; }
    int getApproverRoleId() const { return approver_role_id; }
    int getMinSecurityLevelId() const { return min_security_level_id; }
    int getMaxSecurityLevelId() const { return max_security_level_id; }
    bool isActive() const { return is_active; }
    QDateTime getCreatedAt() const { return created_at; }
    QDateTime getUpdatedAt() const { return updated_at; }

    QString getTaskTypeName() const {
        return task_type == 0 ? QString::fromUtf8("打印") : QString::fromUtf8("刻录");
    }

    // Setters
    void setId(int value) { id = value; }
    void setTaskType(int value) { task_type = value; }
    void setApproverRoleId(int value) { approver_role_id = value; }
    void setMinSecurityLevelId(int value) { min_security_level_id = value; }
    void setMaxSecurityLevelId(int value) { max_security_level_id = value; }
    void setActive(bool value) { is_active = value; }
    void setCreatedAt(const QDateTime& value) { created_at = value; }
    void setUpdatedAt(const QDateTime& value) { updated_at = value; }

    // Legacy struct members for backward compatibility
    int id;
    int task_type;              // 0=打印, 1=刻录
    int approver_role_id;       // 审批角色ID，外键关联 roles 表
    int min_security_level_id;  // 最小密级ID
    int max_security_level_id;  // 最大密级ID
    bool is_active;
    QDateTime created_at;
    QDateTime updated_at;
};

#endif // APPROVER_CONFIG_MODEL_H
