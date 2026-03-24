#ifndef APPROVER_CONFIG_REPOSITORY_H
#define APPROVER_CONFIG_REPOSITORY_H

#include "approver_config_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <vector>
#include <memory>
#include <optional>

class ApproverConfigRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    ApproverConfigRepository();
    explicit ApproverConfigRepository(DatabaseManager* db_manager);
    ~ApproverConfigRepository() = default;

    bool create_table();

    // Qt-style methods for services
    ApproverConfig findById(int id);
    QList<ApproverConfig> findAll();
    bool create(ApproverConfig& config);
    bool update(const ApproverConfig& config);
    bool deleteById(int id);
    bool remove(int id) { return deleteById(id); }

    // Filter methods
    QList<ApproverConfig> findByTaskType(int taskType);
    QList<ApproverConfig> findBySecurityLevel(int taskType, int securityLevelId);
    QList<ApproverConfig> findByApproverRoleId(int approverRoleId);
    QList<ApproverConfig> findActive();

    // Validation
    bool hasOverlap(const ApproverConfig& config, int excludeId = 0);

    // Legacy snake_case methods for backward compatibility
    std::unique_ptr<ApproverConfig> find_by_id(int id);
    std::vector<std::unique_ptr<ApproverConfig>> find_all();
};

#endif // APPROVER_CONFIG_REPOSITORY_H
