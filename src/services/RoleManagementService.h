#ifndef ROLEMANAGEMENTSERVICE_H
#define ROLEMANAGEMENTSERVICE_H

#include <QString>
#include <QList>
#include <QVariantMap>
#include "../models/role_model.h"     // 角色模型
#include "../models/user_model.h"     // 用户模型

class RoleManagementService
{
public:
    // 单例获取方法
    static RoleManagementService& getInstance();

    // 角色管理功能
    bool createRole(const Role &role, int creatorId);
    bool updateRole(const Role &role, int updaterId);
    bool deleteRole(int roleId, int deleterId);
    Role getRoleById(int roleId) const;
    QList<Role> getAllRoles() const;
    QList<Role> searchRoles(const QString &keyword) const;

    // 权限分配
    bool assignPermissionToRole(int roleId, const QString &permission, int operatorId);
    bool removePermissionFromRole(int roleId, const QString &permission, int operatorId);
    QStringList getRolePermissions(int roleId) const;

    // 用户-角色分配
    bool addUserToRole(int userId, int roleId, int operatorId);
    bool removeUserFromRole(int userId, int roleId, int operatorId);
    QList<User> getUsersByRole(int roleId) const;

    // 内置角色管理
    bool createSystemRole(const QString &roleName, const QStringList &permissions, int creatorId);
    bool isSystemRole(int roleId) const;

    // 验证角色权限
    bool validateRolePermission(int roleId, const QString &permission) const;

private:
    RoleManagementService();  // 私有构造函数，确保单例
    ~RoleManagementService();

    // 检查操作者权限
    bool checkOperatorPermission(int operatorId, const QString &operation) const;

    // 记录角色管理操作日志
    bool logRoleOperation(int roleId, int operatorId, const QString &operation, const QString &details = "") const;
};

#endif // ROLEMANAGEMENTSERVICE_H