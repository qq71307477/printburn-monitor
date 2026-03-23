#include "LogAuditService.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// 静态实例
static LogAuditService* instance = nullptr;

LogAuditService& LogAuditService::getInstance()
{
    if (!instance) {
        instance = new LogAuditService();
    }
    return *instance;
}

LogAuditService::LogAuditService()
{
    ensureAuditLogTableExists();
}

LogAuditService::~LogAuditService()
{
}

bool LogAuditService::logEvent(const QString &eventType, int userId, const QString &username,
                              const QString &ipAddress, const QString &userAgent,
                              const QString &details, const QString &severity)
{
    AuditLogEntry entry;
    entry.id = 0; // 将由数据库自动生成
    entry.eventType = eventType;
    entry.userId = userId;
    entry.username = username;
    entry.ipAddress = ipAddress;
    entry.userAgent = userAgent;
    entry.details = details;
    entry.timestamp = QDateTime::currentDateTime();
    entry.severity = severity;

    return saveLogToDatabase(entry);
}

bool LogAuditService::batchLogEvents(const QList<AuditLogEntry> &events)
{
    bool allSuccess = true;

    for (const AuditLogEntry &entry : events) {
        if (!saveLogToDatabase(entry)) {
            allSuccess = false;
        }
    }

    return allSuccess;
}

QList<AuditLogEntry> LogAuditService::queryLogs(const QString &eventType,
                                              int userId,
                                              const QDateTime &startTime,
                                              const QDateTime &endTime,
                                              const QString &severity,
                                              int limit,
                                              int offset) const
{
    QList<AuditLogEntry> logs;

    QString queryStr = "SELECT id, eventType, userId, username, ipAddress, userAgent, details, timestamp, severity FROM audit_logs WHERE 1=1";

    if (!eventType.isEmpty()) {
        queryStr += " AND eventType = ?";
    }
    if (userId > 0) {
        queryStr += " AND userId = ?";
    }
    if (startTime.isValid()) {
        queryStr += " AND timestamp >= ?";
    }
    if (endTime.isValid()) {
        queryStr += " AND timestamp <= ?";
    }
    if (!severity.isEmpty()) {
        queryStr += " AND severity = ?";
    }

    queryStr += " ORDER BY timestamp DESC LIMIT ? OFFSET ?";

    QSqlQuery query(queryStr);

    int paramIndex = 0;
    if (!eventType.isEmpty()) {
        query.bindValue(paramIndex++, eventType);
    }
    if (userId > 0) {
        query.bindValue(paramIndex++, userId);
    }
    if (startTime.isValid()) {
        query.bindValue(paramIndex++, startTime);
    }
    if (endTime.isValid()) {
        query.bindValue(paramIndex++, endTime);
    }
    if (!severity.isEmpty()) {
        query.bindValue(paramIndex++, severity);
    }
    query.bindValue(paramIndex++, limit);
    query.bindValue(paramIndex++, offset);

    if (!query.exec()) {
        return logs; // 返回空列表表示查询失败
    }

    while (query.next()) {
        AuditLogEntry entry;
        entry.id = query.value(0).toInt();
        entry.eventType = query.value(1).toString();
        entry.userId = query.value(2).toInt();
        entry.username = query.value(3).toString();
        entry.ipAddress = query.value(4).toString();
        entry.userAgent = query.value(5).toString();
        entry.details = query.value(6).toString();
        entry.timestamp = query.value(7).toDateTime();
        entry.severity = query.value(8).toString();

        logs.append(entry);
    }

    return logs;
}

QList<AuditLogEntry> LogAuditService::getLogsByUser(int userId,
                                                  const QDateTime &startTime,
                                                  const QDateTime &endTime,
                                                  int limit,
                                                  int offset) const
{
    QList<AuditLogEntry> logs;

    QString queryStr = "SELECT id, eventType, userId, username, ipAddress, userAgent, details, timestamp, severity FROM audit_logs WHERE userId = ?";

    if (startTime.isValid()) {
        queryStr += " AND timestamp >= ?";
    }
    if (endTime.isValid()) {
        queryStr += " AND timestamp <= ?";
    }

    queryStr += " ORDER BY timestamp DESC LIMIT ? OFFSET ?";

    QSqlQuery query(queryStr);
    query.bindValue(0, userId);

    int paramIndex = 1;
    if (startTime.isValid()) {
        query.bindValue(paramIndex++, startTime);
    }
    if (endTime.isValid()) {
        query.bindValue(paramIndex++, endTime);
    }
    query.bindValue(paramIndex++, limit);
    query.bindValue(paramIndex++, offset);

    if (!query.exec()) {
        return logs; // 返回空列表表示查询失败
    }

    while (query.next()) {
        AuditLogEntry entry;
        entry.id = query.value(0).toInt();
        entry.eventType = query.value(1).toString();
        entry.userId = query.value(2).toInt();
        entry.username = query.value(3).toString();
        entry.ipAddress = query.value(4).toString();
        entry.userAgent = query.value(5).toString();
        entry.details = query.value(6).toString();
        entry.timestamp = query.value(7).toDateTime();
        entry.severity = query.value(8).toString();

        logs.append(entry);
    }

    return logs;
}

QList<AuditLogEntry> LogAuditService::getLogsByEventType(const QString &eventType,
                                                       const QDateTime &startTime,
                                                       const QDateTime &endTime,
                                                       int limit,
                                                       int offset) const
{
    QList<AuditLogEntry> logs;

    QString queryStr = "SELECT id, eventType, userId, username, ipAddress, userAgent, details, timestamp, severity FROM audit_logs WHERE eventType = ?";

    if (startTime.isValid()) {
        queryStr += " AND timestamp >= ?";
    }
    if (endTime.isValid()) {
        queryStr += " AND timestamp <= ?";
    }

    queryStr += " ORDER BY timestamp DESC LIMIT ? OFFSET ?";

    QSqlQuery query(queryStr);
    query.bindValue(0, eventType);

    int paramIndex = 1;
    if (startTime.isValid()) {
        query.bindValue(paramIndex++, startTime);
    }
    if (endTime.isValid()) {
        query.bindValue(paramIndex++, endTime);
    }
    query.bindValue(paramIndex++, limit);
    query.bindValue(paramIndex++, offset);

    if (!query.exec()) {
        return logs; // 返回空列表表示查询失败
    }

    while (query.next()) {
        AuditLogEntry entry;
        entry.id = query.value(0).toInt();
        entry.eventType = query.value(1).toString();
        entry.userId = query.value(2).toInt();
        entry.username = query.value(3).toString();
        entry.ipAddress = query.value(4).toString();
        entry.userAgent = query.value(5).toString();
        entry.details = query.value(6).toString();
        entry.timestamp = query.value(7).toDateTime();
        entry.severity = query.value(8).toString();

        logs.append(entry);
    }

    return logs;
}

QVariantMap LogAuditService::getLogStatistics(const QDateTime &startTime,
                                             const QDateTime &endTime,
                                             const QString &groupBy) const
{
    QVariantMap statistics;

    QString groupByField = "DATE(timestamp)";
    if (groupBy == "hour") {
        groupByField = "strftime('%Y-%m-%d %H:00:00', timestamp)";
    } else if (groupBy == "minute") {
        groupByField = "strftime('%Y-%m-%d %H:%M:00', timestamp)";
    }

    QString queryStr = QString("SELECT %1, COUNT(*) as count, eventType FROM audit_logs WHERE timestamp BETWEEN ? AND ? GROUP BY %1, eventType ORDER BY %1")
                       .arg(groupByField);

    QSqlQuery query(queryStr);
    query.bindValue(0, startTime);
    query.bindValue(1, endTime);

    if (!query.exec()) {
        return statistics; // 返回空映射表示查询失败
    }

    QJsonArray statsArray;
    while (query.next()) {
        QJsonObject statObj;
        statObj["time"] = query.value(0).toString();
        statObj["count"] = query.value(1).toInt();
        statObj["eventType"] = query.value(2).toString();
        statsArray.append(statObj);
    }

    statistics["statistics"] = statsArray;
    statistics["totalRecords"] = statsArray.size();

    return statistics;
}

bool LogAuditService::purgeOldLogs(const QDateTime &beforeDate)
{
    QSqlQuery query("DELETE FROM audit_logs WHERE timestamp < ?");
    query.bindValue(0, beforeDate);

    if (!query.exec()) {
        return false;
    }

    // numRowsAffected() 在 Qt5/Qt6 中都是有效的 API
    // 返回值可用于日志记录，但此处只需确认操作成功
    return query.numRowsAffected() >= 0;
}

bool LogAuditService::exportLogs(const QString &filePath,
                               const QDateTime &startTime,
                               const QDateTime &endTime,
                               const QString &format) const
{
    QList<AuditLogEntry> logs = queryLogs("", -1, startTime, endTime, "", 10000, 0);

    if (logs.isEmpty()) {
        return false; // 没有日志可导出
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);

    if (format.toUpper() == "CSV") {
        // CSV格式导出
        out << "ID,Event Type,User ID,Username,IP Address,User Agent,Details,Timestamp,Severity\n";
        for (const AuditLogEntry &log : logs) {
            out << QString("%1,%2,%3,%4,%5,%6,\"%7\",%8,%9\n")
                   .arg(log.id)
                   .arg(log.eventType)
                   .arg(log.userId)
                   .arg(log.username)
                   .arg(log.ipAddress)
                   .arg(log.userAgent)
                   .arg(QString(log.details).replace("\"", "\"\"")) // 转义双引号
                   .arg(log.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                   .arg(log.severity);
        }
    } else if (format.toUpper() == "JSON") {
        // JSON格式导出
        QJsonArray jsonArray;
        for (const AuditLogEntry &log : logs) {
            QJsonObject jsonObj;
            jsonObj["id"] = log.id;
            jsonObj["eventType"] = log.eventType;
            jsonObj["userId"] = log.userId;
            jsonObj["username"] = log.username;
            jsonObj["ipAddress"] = log.ipAddress;
            jsonObj["userAgent"] = log.userAgent;
            jsonObj["details"] = log.details;
            jsonObj["timestamp"] = log.timestamp.toString(Qt::ISODate);
            jsonObj["severity"] = log.severity;
            jsonArray.append(jsonObj);
        }

        QJsonDocument jsonDoc(jsonArray);
        out << jsonDoc.toJson();
    }

    file.close();
    return true;
}

QList<AuditLogEntry> LogAuditService::getRecentLoginLogs(int userId, int limit) const
{
    QString queryStr = "SELECT id, eventType, userId, username, ipAddress, userAgent, details, timestamp, severity FROM audit_logs WHERE eventType IN ('LOGIN_SUCCESS', 'LOGIN_FAILURE', 'LOGOUT')";

    if (userId > 0) {
        queryStr += " AND userId = ?";
    }

    queryStr += " ORDER BY timestamp DESC LIMIT ?";

    QSqlQuery query(queryStr);

    int paramIndex = 0;
    if (userId > 0) {
        query.bindValue(paramIndex++, userId);
    }
    query.bindValue(paramIndex++, limit);

    QList<AuditLogEntry> logs;

    if (!query.exec()) {
        return logs; // 返回空列表表示查询失败
    }

    while (query.next()) {
        AuditLogEntry entry;
        entry.id = query.value(0).toInt();
        entry.eventType = query.value(1).toString();
        entry.userId = query.value(2).toInt();
        entry.username = query.value(3).toString();
        entry.ipAddress = query.value(4).toString();
        entry.userAgent = query.value(5).toString();
        entry.details = query.value(6).toString();
        entry.timestamp = query.value(7).toDateTime();
        entry.severity = query.value(8).toString();

        logs.append(entry);
    }

    return logs;
}

QList<AuditLogEntry> LogAuditService::getAnomalyLogs(const QDateTime &startTime,
                                                    const QDateTime &endTime,
                                                    int limit) const
{
    // 这是一个简化实现，实际系统中可能会有更复杂的异常检测逻辑
    QString queryStr = "SELECT id, eventType, userId, username, ipAddress, userAgent, details, timestamp, severity FROM audit_logs WHERE severity IN ('WARN', 'ERROR')";

    if (startTime.isValid()) {
        queryStr += " AND timestamp >= ?";
    }
    if (endTime.isValid()) {
        queryStr += " AND timestamp <= ?";
    }

    queryStr += " ORDER BY timestamp DESC LIMIT ?";

    QSqlQuery query(queryStr);

    int paramIndex = 0;
    if (startTime.isValid()) {
        query.bindValue(paramIndex++, startTime);
    }
    if (endTime.isValid()) {
        query.bindValue(paramIndex++, endTime);
    }
    query.bindValue(paramIndex++, limit);

    QList<AuditLogEntry> logs;

    if (!query.exec()) {
        return logs; // 返回空列表表示查询失败
    }

    while (query.next()) {
        AuditLogEntry entry;
        entry.id = query.value(0).toInt();
        entry.eventType = query.value(1).toString();
        entry.userId = query.value(2).toInt();
        entry.username = query.value(3).toString();
        entry.ipAddress = query.value(4).toString();
        entry.userAgent = query.value(5).toString();
        entry.details = query.value(6).toString();
        entry.timestamp = query.value(7).toDateTime();
        entry.severity = query.value(8).toString();

        logs.append(entry);
    }

    return logs;
}

bool LogAuditService::saveLogToDatabase(const AuditLogEntry &entry) const
{
    QSqlQuery query("INSERT INTO audit_logs (eventType, userId, username, ipAddress, userAgent, details, timestamp, severity) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.bindValue(0, entry.eventType);
    query.bindValue(1, entry.userId);
    query.bindValue(2, entry.username);
    query.bindValue(3, entry.ipAddress);
    query.bindValue(4, entry.userAgent);
    query.bindValue(5, entry.details);
    query.bindValue(6, entry.timestamp);
    query.bindValue(7, entry.severity);

    if (!query.exec()) {
        return false;
    }

    return true;
}

bool LogAuditService::ensureAuditLogTableExists() const
{
    QSqlQuery query(
        "CREATE TABLE IF NOT EXISTS audit_logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "eventType TEXT NOT NULL,"
        "userId INTEGER NOT NULL,"
        "username TEXT NOT NULL,"
        "ipAddress TEXT,"
        "userAgent TEXT,"
        "details TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "severity TEXT DEFAULT 'INFO'"
        ");"
    );

    if (!query.exec()) {
        return false;
    }

    // 为常用查询字段创建索引
    QSqlQuery indexQuery1("CREATE INDEX IF NOT EXISTS idx_audit_logs_timestamp ON audit_logs(timestamp);");
    indexQuery1.exec();

    QSqlQuery indexQuery2("CREATE INDEX IF NOT EXISTS idx_audit_logs_userId ON audit_logs(userId);");
    indexQuery2.exec();

    QSqlQuery indexQuery3("CREATE INDEX IF NOT EXISTS idx_audit_logs_eventType ON audit_logs(eventType);");
    indexQuery3.exec();

    return true;
}