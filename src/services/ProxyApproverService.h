#ifndef PROXY_APPROVER_SERVICE_H
#define PROXY_APPROVER_SERVICE_H

#include <QString>
#include <QList>
#include <QPair>
#include <QDate>
#include "models/proxy_approver_model.h"

class ProxyApproverService
{
public:
    // Singleton pattern
    static ProxyApproverService& getInstance();

    // Core operations as requested
    int setProxy(int ownerUserId, int proxyUserId, int securityLevelId, const QString& taskType, const QDate& startDate, const QDate& endDate);
    bool removeProxy(int proxyId);
    bool deleteProxy(int proxyId) { return removeProxy(proxyId); }
    QList<ProxyApprover> getProxiesByOwner(int ownerUserId) const;
    QList<ProxyApprover> getProxiesByProxy(int proxyUserId) const;
    ProxyApprover getActiveProxyForTask(int ownerUserId, const QString& taskType, int securityLevelId) const;
    bool isProxyActive(int proxyId) const;
    bool enableProxy(int proxyId, bool enabled);

    // Additional convenience methods
    bool createProxy(ProxyApprover& proxy);
    bool updateProxy(const ProxyApprover& proxy);
    ProxyApprover getProxy(int id) const;
    QList<ProxyApprover> getAllProxies() const;

    // Validation
    QString validateProxy(const ProxyApprover& proxy) const;
    bool hasConflict(const ProxyApprover& proxy, int excludeId = 0) const;

    // Get users for selection (for UI dropdown)
    QList<QPair<int, QString>> getUsersForSelection() const;

    // Get security levels for selection
    QList<QPair<int, QString>> getSecurityLevelsForSelection() const;

private:
    ProxyApproverService();
    ~ProxyApproverService();

    // Disallow copy and assignment
    ProxyApproverService(const ProxyApproverService&) = delete;
    ProxyApproverService& operator=(const ProxyApproverService&) = delete;

    // Helper to convert task type string to int
    int taskTypeToInt(const QString& taskType) const;
    QString taskTypeToString(int taskType) const;

    // Check operator permission
    bool checkOperatorPermission(int operatorId, const QString& operation) const;

    // Log operation
    bool logOperation(int proxyId, int operatorId, const QString& operation, const QString& details = "") const;
};

#endif // PROXY_APPROVER_SERVICE_H