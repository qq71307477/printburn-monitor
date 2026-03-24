#ifndef PROXY_APPROVER_REPOSITORY_H
#define PROXY_APPROVER_REPOSITORY_H

#include "proxy_approver_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <vector>
#include <memory>
#include <optional>

class ProxyApproverRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    ProxyApproverRepository();
    explicit ProxyApproverRepository(DatabaseManager* db_manager);
    ~ProxyApproverRepository() = default;

    bool create_table();

    // Qt-style methods for services
    ProxyApprover findById(int id);
    QList<ProxyApprover> findAll();
    bool create(ProxyApprover& proxy);
    bool update(const ProxyApprover& proxy);
    bool deleteById(int id);
    bool remove(int id) { return deleteById(id); }
    bool save(ProxyApprover& proxy);  // Alias for create/update

    // Filter methods
    QList<ProxyApprover> findByOwnerUserId(int ownerUserId);
    QList<ProxyApprover> findByProxyUserId(int proxyUserId);
    QList<ProxyApprover> findByOwnerId(int ownerUserId) { return findByOwnerUserId(ownerUserId); }
    QList<ProxyApprover> findByProxyId(int proxyUserId) { return findByProxyUserId(proxyUserId); }
    QList<ProxyApprover> findActiveByOwner(int ownerUserId, int taskType, int securityLevelId);
    QList<ProxyApprover> findActiveProxies(int ownerUserId, int taskType, int securityLevelId) { return findActiveByOwner(ownerUserId, taskType, securityLevelId); }
    QList<ProxyApprover> findEnabled();

    // Check if proxy is active (within date range and enabled)
    bool isActive(int proxyId);

    // Validation
    bool hasConflict(const ProxyApprover& proxy, int excludeId = 0);

    // Legacy snake_case methods for backward compatibility
    std::unique_ptr<ProxyApprover> find_by_id(int id);
    std::vector<std::unique_ptr<ProxyApprover>> find_all();
};

#endif // PROXY_APPROVER_REPOSITORY_H