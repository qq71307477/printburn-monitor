#ifndef APPROVER_CONFIG_SERVICE_H
#define APPROVER_CONFIG_SERVICE_H

#include <QString>
#include <QList>
#include "models/approver_config_model.h"

class ApproverConfigService
{
public:
    // 单例获取方法
    static ApproverConfigService& getInstance();

    // 审批配置管理功能
    bool createConfig(const ApproverConfig& config, int operatorId);
    bool updateConfig(const ApproverConfig& config, int operatorId);
    bool deleteConfig(int id, int operatorId);
    ApproverConfig getConfig(int id) const;
    QList<ApproverConfig> getAllConfigs() const;
    QList<ApproverConfig> getConfigsByTaskType(int taskType) const;
    QList<ApproverConfig> getConfigsForTask(int taskType, int securityLevelId) const;

    // 批量操作
    bool bulkCreateConfigs(const QList<ApproverConfig>& configs, int operatorId);
    bool bulkDeleteConfigs(const QList<int>& ids, int operatorId);

    // 激活/停用
    bool activateConfig(int id, int operatorId);
    bool deactivateConfig(int id, int operatorId);

    // 验证配置
    QString validateConfig(const ApproverConfig& config) const;
    bool hasDuplicateConfig(const ApproverConfig& config, int excludeId = 0) const;

    // 获取审批角色列表（用于UI下拉选择）
    QList<QPair<int, QString>> getApproverRoles() const;

private:
    ApproverConfigService();  // 私有构造函数，确保单例
    ~ApproverConfigService();

    // 检查操作者权限
    bool checkOperatorPermission(int operatorId, const QString& operation) const;

    // 记录操作日志
    bool logOperation(int configId, int operatorId, const QString& operation, const QString& details = "") const;

    // 更新配置状态（内部方法）
    bool updateConfigStatus(int id, bool active, int operatorId);
};

#endif // APPROVER_CONFIG_SERVICE_H
