#ifndef PROXY_APPROVER_MODEL_H
#define PROXY_APPROVER_MODEL_H

#include <QString>
#include <QDate>
#include <QDateTime>

class ProxyApprover {
public:
    ProxyApprover()
        : id(0)
        , owner_user_id(0)
        , proxy_user_id(0)
        , min_security_level_id(0)
        , max_security_level_id(9999)
        , task_type(2)  // 0=打印, 1=刻录, 2=全部
        , enabled(true)
    {}

    // Getters
    int getId() const { return id; }
    int getOwnerUserId() const { return owner_user_id; }
    int getProxyUserId() const { return proxy_user_id; }
    int getMinSecurityLevelId() const { return min_security_level_id; }
    int getMaxSecurityLevelId() const { return max_security_level_id; }
    int getTaskType() const { return task_type; }
    QDate getStartDate() const { return start_date; }
    QDate getEndDate() const { return end_date; }
    bool isEnabled() const { return enabled; }
    QDateTime getCreatedAt() const { return created_at; }
    QDateTime getUpdatedAt() const { return updated_at; }

    QString getTaskTypeName() const {
        switch (task_type) {
            case 0: return QString::fromUtf8("打印");
            case 1: return QString::fromUtf8("刻录");
            case 2: return QString::fromUtf8("全部");
            default: return QString::fromUtf8("未知");
        }
    }

    QString getSecurityLevelRange() const {
        return QString::fromUtf8("%1 - %2").arg(min_security_level_id).arg(max_security_level_id);
    }

    // Setters
    void setId(int value) { id = value; }
    void setOwnerUserId(int value) { owner_user_id = value; }
    void setProxyUserId(int value) { proxy_user_id = value; }
    void setMinSecurityLevelId(int value) { min_security_level_id = value; }
    void setMaxSecurityLevelId(int value) { max_security_level_id = value; }
    void setTaskType(int value) { task_type = value; }
    void setStartDate(const QDate& value) { start_date = value; }
    void setEndDate(const QDate& value) { end_date = value; }
    void setEnabled(bool value) { enabled = value; }
    void setCreatedAt(const QDateTime& value) { created_at = value; }
    void setUpdatedAt(const QDateTime& value) { updated_at = value; }

    // Legacy struct members for backward compatibility
    int id;
    int owner_user_id;          // 被代理人ID（原始审批人）
    int proxy_user_id;          // 代理人ID
    int min_security_level_id;  // 最小密级ID
    int max_security_level_id;  // 最大密级ID
    int task_type;              // 0=打印, 1=刻录, 2=全部
    QDate start_date;           // 代理开始日期
    QDate end_date;             // 代理结束日期
    bool enabled;               // 是否启用
    QDateTime created_at;
    QDateTime updated_at;
};

#endif // PROXY_APPROVER_MODEL_H