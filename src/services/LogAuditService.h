#ifndef LOGAUDITSERVICE_H
#define LOGAUDITSERVICE_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QVariantMap>
#include "../models/user_model.h"

struct AuditLogEntry {
    int id;
    QString eventType;      // 事件类型 (LOGIN, LOGOUT, CREATE_TASK, APPROVE_TASK等)
    QString username;       // 用户名
    int userId;             // 用户ID
    QString ipAddress;      // IP地址
    QString userAgent;      // 用户代理
    QString details;        // 详细信息
    QDateTime timestamp;    // 时间戳
    QString severity;       // 严重级别 (INFO, WARN, ERROR)
};

class LogAuditService
{
public:
    // 单例获取方法
    static LogAuditService& getInstance();

    // 记录审计日志
    bool logEvent(const QString &eventType, int userId, const QString &username,
                 const QString &ipAddress, const QString &userAgent,
                 const QString &details, const QString &severity = "INFO");

    // 批量记录审计日志
    bool batchLogEvents(const QList<AuditLogEntry> &events);

    // 查询审计日志
    QList<AuditLogEntry> queryLogs(const QString &eventType = "",
                                  int userId = -1,
                                  const QDateTime &startTime = QDateTime(),
                                  const QDateTime &endTime = QDateTime(),
                                  const QString &severity = "",
                                  int limit = 100,
                                  int offset = 0) const;

    // 按用户查询日志
    QList<AuditLogEntry> getLogsByUser(int userId,
                                      const QDateTime &startTime = QDateTime(),
                                      const QDateTime &endTime = QDateTime(),
                                      int limit = 100,
                                      int offset = 0) const;

    // 按事件类型查询日志
    QList<AuditLogEntry> getLogsByEventType(const QString &eventType,
                                           const QDateTime &startTime = QDateTime(),
                                           const QDateTime &endTime = QDateTime(),
                                           int limit = 100,
                                           int offset = 0) const;

    // 获取特定时间段内的日志统计
    QVariantMap getLogStatistics(const QDateTime &startTime,
                                const QDateTime &endTime,
                                const QString &groupBy = "day") const;

    // 清理旧日志
    bool purgeOldLogs(const QDateTime &beforeDate);

    // 导出日志
    bool exportLogs(const QString &filePath,
                   const QDateTime &startTime = QDateTime(),
                   const QDateTime &endTime = QDateTime(),
                   const QString &format = "CSV") const;

    // 获取最近的登录日志
    QList<AuditLogEntry> getRecentLoginLogs(int userId = -1, int limit = 50) const;

    // 获取异常活动日志
    QList<AuditLogEntry> getAnomalyLogs(const QDateTime &startTime = QDateTime(),
                                       const QDateTime &endTime = QDateTime(),
                                       int limit = 100) const;

private:
    LogAuditService();  // 私有构造函数，确保单例
    ~LogAuditService();

    // 保存日志到数据库
    bool saveLogToDatabase(const AuditLogEntry &entry) const;

    // 创建审计日志表（如果不存在）
    bool ensureAuditLogTableExists() const;
};

#endif // LOGAUDITSERVICE_H