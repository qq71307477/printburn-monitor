#include "ProxyApproverService.h"
#include "src/common/repository/proxy_approver_repository.h"
#include "src/common/repository/user_repository.h"
#include "src/common/repository/security_level_repository.h"
#include <QDateTime>
#include <mutex>

// Static instance
static std::once_flag onceFlag;
static ProxyApproverService* instance = nullptr;

ProxyApproverService& ProxyApproverService::getInstance()
{
    std::call_once(onceFlag, []() {
        instance = new ProxyApproverService();
    });
    return *instance;
}

ProxyApproverService::ProxyApproverService()
{
}

ProxyApproverService::~ProxyApproverService()
{
}

int ProxyApproverService::taskTypeToInt(const QString& taskType) const
{
    if (taskType == "print" || taskType == "0") {
        return 0;
    } else if (taskType == "burn" || taskType == "1") {
        return 1;
    }
    return 2; // default to "all"
}

QString ProxyApproverService::taskTypeToString(int taskType) const
{
    switch (taskType) {
        case 0: return "print";
        case 1: return "burn";
        case 2: return "all";
        default: return "all";
    }
}

int ProxyApproverService::setProxy(int ownerUserId, int proxyUserId, int securityLevelId, const QString& taskType, const QDate& startDate, const QDate& endDate)
{
    // Create proxy approver object
    ProxyApprover proxy;
    proxy.setOwnerUserId(ownerUserId);
    proxy.setProxyUserId(proxyUserId);
    proxy.setMinSecurityLevelId(0);  // Default to cover all security levels
    proxy.setMaxSecurityLevelId(securityLevelId > 0 ? securityLevelId : 9999);
    proxy.setTaskType(taskTypeToInt(taskType));
    proxy.setStartDate(startDate);
    proxy.setEndDate(endDate);
    proxy.setEnabled(true);
    proxy.setCreatedAt(QDateTime::currentDateTime());
    proxy.setUpdatedAt(QDateTime::currentDateTime());

    // Validate
    QString validationError = validateProxy(proxy);
    if (!validationError.isEmpty()) {
        return -1;
    }

    // Check for conflicts
    if (hasConflict(proxy)) {
        return -1;
    }

    // Save to database
    ProxyApproverRepository repo;
    if (repo.save(proxy)) {
        return proxy.getId();
    }
    return -1;
}

bool ProxyApproverService::removeProxy(int proxyId)
{
    ProxyApproverRepository repo;
    return repo.deleteById(proxyId);
}

QList<ProxyApprover> ProxyApproverService::getProxiesByOwner(int ownerUserId) const
{
    ProxyApproverRepository repo;
    return repo.findByOwnerId(ownerUserId);
}

QList<ProxyApprover> ProxyApproverService::getProxiesByProxy(int proxyUserId) const
{
    ProxyApproverRepository repo;
    return repo.findByProxyId(proxyUserId);
}

ProxyApprover ProxyApproverService::getActiveProxyForTask(int ownerUserId, const QString& taskType, int securityLevelId) const
{
    ProxyApproverRepository repo;
    int taskTypeInt = taskTypeToInt(taskType);
    QList<ProxyApprover> proxies = repo.findActiveProxies(ownerUserId, taskTypeInt, securityLevelId);

    // Return the first active proxy found
    if (!proxies.isEmpty()) {
        return proxies.first();
    }
    return ProxyApprover();
}

bool ProxyApproverService::isProxyActive(int proxyId) const
{
    ProxyApproverRepository repo;
    ProxyApprover proxy = repo.findById(proxyId);

    if (proxy.getId() <= 0) {
        return false;
    }

    if (!proxy.isEnabled()) {
        return false;
    }

    QDate today = QDate::currentDate();
    if (!proxy.getStartDate().isNull() && proxy.getStartDate() > today) {
        return false;
    }
    if (!proxy.getEndDate().isNull() && proxy.getEndDate() < today) {
        return false;
    }

    return true;
}

bool ProxyApproverService::enableProxy(int proxyId, bool enabled)
{
    ProxyApproverRepository repo;
    ProxyApprover proxy = repo.findById(proxyId);

    if (proxy.getId() <= 0) {
        return false;
    }

    proxy.setEnabled(enabled);
    proxy.setUpdatedAt(QDateTime::currentDateTime());

    return repo.update(proxy);
}

bool ProxyApproverService::createProxy(ProxyApprover& proxy)
{
    // Validate
    QString validationError = validateProxy(proxy);
    if (!validationError.isEmpty()) {
        return false;
    }

    // Check for conflicts
    if (hasConflict(proxy)) {
        return false;
    }

    proxy.setCreatedAt(QDateTime::currentDateTime());
    proxy.setUpdatedAt(QDateTime::currentDateTime());

    ProxyApproverRepository repo;
    return repo.save(proxy);
}

bool ProxyApproverService::updateProxy(const ProxyApprover& proxy)
{
    // Validate
    QString validationError = validateProxy(proxy);
    if (!validationError.isEmpty()) {
        return false;
    }

    // Check for conflicts (excluding current proxy)
    if (hasConflict(proxy, proxy.getId())) {
        return false;
    }

    ProxyApprover updatedProxy = proxy;
    updatedProxy.setUpdatedAt(QDateTime::currentDateTime());

    ProxyApproverRepository repo;
    return repo.update(updatedProxy);
}

ProxyApprover ProxyApproverService::getProxy(int id) const
{
    ProxyApproverRepository repo;
    return repo.findById(id);
}

QList<ProxyApprover> ProxyApproverService::getAllProxies() const
{
    ProxyApproverRepository repo;
    return repo.findAll();
}

QString ProxyApproverService::validateProxy(const ProxyApprover& proxy) const
{
    // Check required fields
    if (proxy.getOwnerUserId() <= 0) {
        return QString::fromUtf8("被代理人ID无效");
    }
    if (proxy.getProxyUserId() <= 0) {
        return QString::fromUtf8("请选择代理人");
    }
    if (proxy.getOwnerUserId() == proxy.getProxyUserId()) {
        return QString::fromUtf8("不能将自己设为代理人");
    }

    // Check security level range
    if (proxy.getMinSecurityLevelId() > proxy.getMaxSecurityLevelId()) {
        return QString::fromUtf8("最小密级不能大于最大密级");
    }

    // Check date range
    if (!proxy.getStartDate().isNull() && !proxy.getEndDate().isNull()) {
        if (proxy.getStartDate() > proxy.getEndDate()) {
            return QString::fromUtf8("开始日期不能晚于结束日期");
        }
    }

    // Check if proxy user exists and is active
    UserRepository userRepo;
    User proxyUser = userRepo.findById(proxy.getProxyUserId());
    if (proxyUser.getId() <= 0) {
        return QString::fromUtf8("代理人用户不存在");
    }
    if (!proxyUser.isActive()) {
        return QString::fromUtf8("代理人用户已被禁用");
    }

    return QString();
}

bool ProxyApproverService::hasConflict(const ProxyApprover& proxy, int excludeId) const
{
    ProxyApproverRepository repo;
    return repo.hasConflict(proxy, excludeId);
}

QList<QPair<int, QString>> ProxyApproverService::getUsersForSelection() const
{
    QList<QPair<int, QString>> users;

    UserRepository userRepo;
    QList<User> allUsers = userRepo.findAll();

    for (const User& user : allUsers) {
        if (user.isActive()) {
            QString displayName = user.getUsername();
            if (!user.getFirstName().isEmpty() || !user.getLastName().isEmpty()) {
                displayName = QString::fromUtf8("%1 %2 (%3)")
                    .arg(user.getLastName())
                    .arg(user.getFirstName())
                    .arg(user.getUsername());
            }
            users.append(qMakePair(user.getId(), displayName));
        }
    }

    return users;
}

QList<QPair<int, QString>> ProxyApproverService::getSecurityLevelsForSelection() const
{
    QList<QPair<int, QString>> levels;

    SecurityLevelRepository levelRepo;
    QList<SecurityLevel> allLevels = levelRepo.findAll();

    for (const SecurityLevel& level : allLevels) {
        levels.append(qMakePair(level.getId(), level.getLevelName()));
    }

    return levels;
}

bool ProxyApproverService::checkOperatorPermission(int operatorId, const QString& operation) const
{
    // In a real implementation, this would check if the operator has the permission
    // to perform the specified operation
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    return true;
}

bool ProxyApproverService::logOperation(int proxyId, int operatorId, const QString& operation, const QString& details) const
{
    // In a real implementation, this would log the operation to the audit log
    Q_UNUSED(proxyId)
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    Q_UNUSED(details)
    return true;
}