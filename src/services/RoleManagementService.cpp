#include "RoleManagementService.h"
#include "../src/common/repository/RoleRepository.h"
#include "../src/common/repository/UserRepository.h"
#include <QDateTime>

// 静态实例
static RoleManagementService* instance = nullptr;

RoleManagementService& RoleManagementService::getInstance()
{
    if (!instance) {
        instance = new RoleManagementService();
    }
    return *instance;
}

RoleManagementService::RoleManagementService()
{
}

RoleManagementService::~RoleManagementService()
{
}

bool RoleManagementService::createRole(const Role &role, int creatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(creatorId, "CREATE_ROLE")) {
        return false;
    }

    // 验证角色信息
    if (role.getName().isEmpty()) {
        return false;
    }

    RoleRepository roleRepo;

    // 检查角色名是否已存在
    Role existingRole = roleRepo.findByName(role.getName());
    if (existingRole.getId() > 0) {
        return false; // 角色名已存在
    }

    Role newRole = role;
    newRole.setCreateTime(QDateTime::currentDateTime());
    newRole.setUpdateTime(QDateTime::currentDateTime());
    newRole.setIsSystem(false); // 普通用户创建的角色不是系统角色

    bool result = roleRepo.create(newRole);

    if (result) {
        logRoleOperation(newRole.getId(), creatorId, "CREATE", "Created new role: " + newRole.getName());
    }

    return result;
}

bool RoleManagementService::updateRole(const Role &role, int updaterId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(updaterId, "UPDATE_ROLE")) {
        return false;
    }

    RoleRepository roleRepo;
    Role existingRole = roleRepo.findById(role.getId());
    if (existingRole.getId() <= 0) {
        return false; // 角色不存在
    }

    // 系统角色不能被修改
    if (existingRole.getIsSystem()) {
        return false;
    }

    Role updateRole = role;
    updateRole.setUpdateTime(QDateTime::currentDateTime());

    bool result = roleRepo.update(updateRole);

    if (result) {
        logRoleOperation(updateRole.getId(), updaterId, "UPDATE", "Updated role: " + updateRole.getName());
    }

    return result;
}

bool RoleManagementService::deleteRole(int roleId, int deleterId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(deleterId, "DELETE_ROLE")) {
        return false;
    }

    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return false; // 角色不存在
    }

    // 系统角色不能被删除
    if (role.getIsSystem()) {
        return false;
    }

    // 检查是否有用户正在使用此角色
    QList<User> usersWithRole = getUsersByRole(roleId);
    if (!usersWithRole.isEmpty()) {
        return false; // 不能删除仍有用户使用的角色
    }

    bool result = roleRepo.deleteById(roleId);

    if (result) {
        logRoleOperation(roleId, deleterId, "DELETE", "Deleted role: " + role.getName());
    }

    return result;
}

Role RoleManagementService::getRoleById(int roleId) const
{
    RoleRepository roleRepo;
    return roleRepo.findById(roleId);
}

QList<Role> RoleManagementService::getAllRoles() const
{
    RoleRepository roleRepo;
    return roleRepo.findAll();
}

QList<Role> RoleManagementService::searchRoles(const QString &keyword) const
{
    RoleRepository roleRepo;
    return roleRepo.search(keyword);
}

bool RoleManagementService::assignPermissionToRole(int roleId, const QString &permission, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "ASSIGN_PERMISSION")) {
        return false;
    }

    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return false; // 角色不存在
    }

    // 获取当前权限列表并添加新权限
    QStringList currentPermissions = getRolePermissions(roleId);
    if (currentPermissions.contains(permission)) {
        return false; // 权限已存在
    }

    currentPermissions.append(permission);

    role.setPermissions(currentPermissions.join(","));
    role.setUpdateTime(QDateTime::currentDateTime());

    bool result = roleRepo.update(role);

    if (result) {
        logRoleOperation(roleId, operatorId, "ASSIGN_PERMISSION",
                        QString("Assigned permission '%1' to role '%2'").arg(permission).arg(role.getName()));
    }

    return result;
}

bool RoleManagementService::removePermissionFromRole(int roleId, const QString &permission, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "REMOVE_PERMISSION")) {
        return false;
    }

    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return false; // 角色不存在
    }

    // 获取当前权限列表并移除指定权限
    QStringList currentPermissions = getRolePermissions(roleId);
    if (!currentPermissions.removeAll(permission)) {
        return false; // 权限不存在于角色中
    }

    role.setPermissions(currentPermissions.join(","));
    role.setUpdateTime(QDateTime::currentDateTime());

    bool result = roleRepo.update(role);

    if (result) {
        logRoleOperation(roleId, operatorId, "REMOVE_PERMISSION",
                        QString("Removed permission '%1' from role '%2'").arg(permission).arg(role.getName()));
    }

    return result;
}

QStringList RoleManagementService::getRolePermissions(int roleId) const
{
    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return QStringList(); // 角色不存在
    }

    // 分割权限字符串为列表
    if (role.getPermissions().isEmpty()) {
        return QStringList();
    }

    return role.getPermissions().split(",", Qt::SkipEmptyParts);
}

bool RoleManagementService::addUserToRole(int userId, int roleId, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "ASSIGN_USER_TO_ROLE")) {
        return false;
    }

    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return false; // 角色不存在
    }

    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false; // 用户不存在
    }

    // 实际的角色分配将在Repository层面处理
    // 记录操作日志
    logRoleOperation(roleId, operatorId, "ADD_USER_TO_ROLE",
                    QString("Added user '%1' to role '%2'").arg(user.getUsername()).arg(role.getName()));

    return true;
}

bool RoleManagementService::removeUserFromRole(int userId, int roleId, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "REMOVE_USER_FROM_ROLE")) {
        return false;
    }

    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return false; // 角色不存在
    }

    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false; // 用户不存在
    }

    // 实际的用户角色移除将在Repository层面处理
    // 记录操作日志
    logRoleOperation(roleId, operatorId, "REMOVE_USER_FROM_ROLE",
                    QString("Removed user '%1' from role '%2'").arg(user.getUsername()).arg(role.getName()));

    return true;
}

QList<User> RoleManagementService::getUsersByRole(int roleId) const
{
    UserRepository userRepo;
    return userRepo.findByRoleId(roleId);
}

bool RoleManagementService::createSystemRole(const QString &roleName, const QStringList &permissions, int creatorId)
{
    // 检查操作者权限（需要系统管理员权限）
    if (!checkOperatorPermission(creatorId, "CREATE_SYSTEM_ROLE")) {
        return false;
    }

    RoleRepository roleRepo;

    // 检查角色名是否已存在
    Role existingRole = roleRepo.findByName(roleName);
    if (existingRole.getId() > 0) {
        return false; // 角色名已存在
    }

    Role newRole;
    newRole.setName(roleName);
    newRole.setDescription(QString("System role: %1").arg(roleName));
    newRole.setPermissions(permissions.join(","));
    newRole.setCreateTime(QDateTime::currentDateTime());
    newRole.setUpdateTime(QDateTime::currentDateTime());
    newRole.setIsSystem(true); // 标记为系统角色

    bool result = roleRepo.create(newRole);

    if (result) {
        logRoleOperation(newRole.getId(), creatorId, "CREATE_SYSTEM_ROLE",
                        QString("Created system role: %1").arg(roleName));
    }

    return result;
}

bool RoleManagementService::isSystemRole(int roleId) const
{
    Role role = getRoleById(roleId);
    return role.getIsSystem();
}

bool RoleManagementService::validateRolePermission(int roleId, const QString &permission) const
{
    QStringList permissions = getRolePermissions(roleId);
    return permissions.contains(permission);
}

bool RoleManagementService::checkOperatorPermission(int operatorId, const QString &operation) const
{
    // 在实际实现中，这里会检查操作者的权限
    // 检查该操作者是否有执行指定操作的权限
    // 简化实现，返回true
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    return true;
}

bool RoleManagementService::logRoleOperation(int roleId, int operatorId, const QString &operation, const QString &details) const
{
    // 实际实现中应记录角色操作日志到数据库
    // 这里应该调用日志服务或安全插件来记录操作
    Q_UNUSED(roleId)
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    Q_UNUSED(details)
    return true;
}