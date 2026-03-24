#include "ApproverConfigService.h"
#include "src/common/repository/approver_config_repository.h"
#include "src/common/repository/role_repository.h"
#include <QDateTime>
#include <mutex>

// 静态实例
static std::once_flag onceFlag;
static ApproverConfigService* instance = nullptr;

ApproverConfigService& ApproverConfigService::getInstance()
{
    std::call_once(onceFlag, []() {
        instance = new ApproverConfigService();
    });
    return *instance;
}

ApproverConfigService::ApproverConfigService()
{
}

ApproverConfigService::~ApproverConfigService()
{
}

bool ApproverConfigService::createConfig(const ApproverConfig& config, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "CREATE_APPROVER_CONFIG")) {
        return false;
    }

    // 验证配置
    QString validationError = validateConfig(config);
    if (!validationError.isEmpty()) {
        return false;
    }

    // 检查重复配置
    if (hasDuplicateConfig(config)) {
        return false;
    }

    ApproverConfig newConfig = config;
    newConfig.setCreatedAt(QDateTime::currentDateTime());
    newConfig.setUpdatedAt(QDateTime::currentDateTime());

    ApproverConfigRepository repo;
    bool result = repo.create(newConfig);

    if (result) {
        logOperation(newConfig.getId(), operatorId, "CREATE",
                    QString("Created approver config for task_type=%1, role_id=%2")
                    .arg(config.getTaskType())
                    .arg(config.getApproverRoleId()));
    }

    return result;
}

bool ApproverConfigService::updateConfig(const ApproverConfig& config, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "UPDATE_APPROVER_CONFIG")) {
        return false;
    }

    ApproverConfigRepository repo;
    ApproverConfig existingConfig = repo.findById(config.getId());
    if (existingConfig.getId() <= 0) {
        return false; // 配置不存在
    }

    // 验证配置
    QString validationError = validateConfig(config);
    if (!validationError.isEmpty()) {
        return false;
    }

    // 检查重复配置
    if (hasDuplicateConfig(config, config.getId())) {
        return false;
    }

    ApproverConfig updatedConfig = config;
    updatedConfig.setUpdatedAt(QDateTime::currentDateTime());

    bool result = repo.update(updatedConfig);

    if (result) {
        logOperation(config.getId(), operatorId, "UPDATE",
                    QString("Updated approver config: task_type=%1, role_id=%2")
                    .arg(config.getTaskType())
                    .arg(config.getApproverRoleId()));
    }

    return result;
}

bool ApproverConfigService::deleteConfig(int id, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "DELETE_APPROVER_CONFIG")) {
        return false;
    }

    ApproverConfigRepository repo;
    ApproverConfig config = repo.findById(id);
    if (config.getId() <= 0) {
        return false; // 配置不存在
    }

    bool result = repo.deleteById(id);

    if (result) {
        logOperation(id, operatorId, "DELETE",
                    QString("Deleted approver config: task_type=%1, role_id=%2")
                    .arg(config.getTaskType())
                    .arg(config.getApproverRoleId()));
    }

    return result;
}

ApproverConfig ApproverConfigService::getConfig(int id) const
{
    ApproverConfigRepository repo;
    return repo.findById(id);
}

QList<ApproverConfig> ApproverConfigService::getAllConfigs() const
{
    ApproverConfigRepository repo;
    return repo.findAll();
}

QList<ApproverConfig> ApproverConfigService::getConfigsByTaskType(int taskType) const
{
    ApproverConfigRepository repo;
    return repo.findByTaskType(taskType);
}

QList<ApproverConfig> ApproverConfigService::getConfigsForTask(int taskType, int securityLevelId) const
{
    ApproverConfigRepository repo;
    return repo.findBySecurityLevel(taskType, securityLevelId);
}

bool ApproverConfigService::bulkCreateConfigs(const QList<ApproverConfig>& configs, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "BULK_CREATE_APPROVER_CONFIG")) {
        return false;
    }

    bool allSuccess = true;
    ApproverConfigRepository repo;

    for (const ApproverConfig& config : configs) {
        // 验证配置
        QString validationError = validateConfig(config);
        if (!validationError.isEmpty()) {
            allSuccess = false;
            continue;
        }

        // 检查重复配置
        if (hasDuplicateConfig(config)) {
            allSuccess = false;
            continue;
        }

        ApproverConfig newConfig = config;
        newConfig.setCreatedAt(QDateTime::currentDateTime());
        newConfig.setUpdatedAt(QDateTime::currentDateTime());

        if (!repo.create(newConfig)) {
            allSuccess = false;
        } else {
            logOperation(newConfig.getId(), operatorId, "BULK_CREATE",
                        QString("Bulk created approver config: task_type=%1, role_id=%2")
                        .arg(config.getTaskType())
                        .arg(config.getApproverRoleId()));
        }
    }

    return allSuccess;
}

bool ApproverConfigService::bulkDeleteConfigs(const QList<int>& ids, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "BULK_DELETE_APPROVER_CONFIG")) {
        return false;
    }

    bool allSuccess = true;
    ApproverConfigRepository repo;

    for (int id : ids) {
        ApproverConfig config = repo.findById(id);
        if (config.getId() <= 0) {
            allSuccess = false;
            continue;
        }

        if (!repo.deleteById(id)) {
            allSuccess = false;
        } else {
            logOperation(id, operatorId, "BULK_DELETE",
                        QString("Bulk deleted approver config: task_type=%1, role_id=%2")
                        .arg(config.getTaskType())
                        .arg(config.getApproverRoleId()));
        }
    }

    return allSuccess;
}

bool ApproverConfigService::activateConfig(int id, int operatorId)
{
    return updateConfigStatus(id, true, operatorId);
}

bool ApproverConfigService::deactivateConfig(int id, int operatorId)
{
    return updateConfigStatus(id, false, operatorId);
}

QString ApproverConfigService::validateConfig(const ApproverConfig& config) const
{
    // 验证任务类型
    if (config.getTaskType() != 0 && config.getTaskType() != 1) {
        return QString::fromUtf8("无效的任务类型");
    }

    // 验证审批角色ID
    if (config.getApproverRoleId() <= 0) {
        return QString::fromUtf8("请选择审批角色");
    }

    // 验证密级范围
    if (config.getMinSecurityLevelId() < 0) {
        return QString::fromUtf8("最小密级不能为负数");
    }

    if (config.getMaxSecurityLevelId() < 0) {
        return QString::fromUtf8("最大密级不能为负数");
    }

    if (config.getMinSecurityLevelId() > config.getMaxSecurityLevelId()) {
        return QString::fromUtf8("最小密级不能大于最大密级");
    }

    // 验证角色是否存在
    RoleRepository roleRepo;
    Role role = roleRepo.findById(config.getApproverRoleId());
    if (role.getId() <= 0) {
        return QString::fromUtf8("指定的审批角色不存在");
    }

    return QString(); // 验证通过
}

bool ApproverConfigService::hasDuplicateConfig(const ApproverConfig& config, int excludeId) const
{
    ApproverConfigRepository repo;
    return repo.hasOverlap(config, excludeId);
}

QList<QPair<int, QString>> ApproverConfigService::getApproverRoles() const
{
    QList<QPair<int, QString>> roles;

    RoleRepository roleRepo;
    QList<Role> allRoles = roleRepo.findAll();

    for (const Role& role : allRoles) {
        if (role.isActive()) {
            roles.append(qMakePair(role.getId(), role.getName()));
        }
    }

    return roles;
}

bool ApproverConfigService::checkOperatorPermission(int operatorId, const QString& operation) const
{
    // 在实际实现中，这里会检查操作者的权限
    // 检查该操作者是否有执行指定操作的权限
    // 简化实现，返回true
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    return true;
}

bool ApproverConfigService::logOperation(int configId, int operatorId, const QString& operation, const QString& details) const
{
    // 实际实现中应记录操作日志到数据库
    // 这里应该调用日志服务或安全插件来记录操作
    Q_UNUSED(configId)
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    Q_UNUSED(details)
    return true;
}

bool ApproverConfigService::updateConfigStatus(int id, bool active, int operatorId)
{
    QString operationStr = active ? "ACTIVATE" : "DEACTIVATE";
    QString permissionStr = active ? "ACTIVATE_APPROVER_CONFIG" : "DEACTIVATE_APPROVER_CONFIG";

    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, permissionStr)) {
        return false;
    }

    ApproverConfigRepository repo;
    ApproverConfig config = repo.findById(id);
    if (config.getId() <= 0) {
        return false;
    }

    config.setActive(active);
    config.setUpdatedAt(QDateTime::currentDateTime());

    bool result = repo.update(config);

    if (result) {
        logOperation(id, operatorId, operationStr,
                    QString("Changed approver config status to %1: task_type=%2")
                    .arg(active ? "ACTIVE" : "INACTIVE")
                    .arg(config.getTaskType()));
    }

    return result;
}
