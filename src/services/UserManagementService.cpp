#include "UserManagementService.h"
#include "src/common/repository/user_repository.h"
#include "src/common/repository/role_repository.h"
#include <QDateTime>
#include <QCryptographicHash>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <mutex>

// Helper function to escape CSV fields
static QString escapeCsvField(const QString &field)
{
    // If the field contains comma, quote, or newline, wrap it in quotes and escape quotes
    if (field.contains(',') || field.contains('"') || field.contains('\n')) {
        QString escaped = field;
        escaped.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return field;
}

// 静态实例
static std::once_flag onceFlag;
static UserManagementService* instance = nullptr;

UserManagementService& UserManagementService::getInstance()
{
    std::call_once(onceFlag, []() {
        instance = new UserManagementService();
    });
    return *instance;
}

UserManagementService::UserManagementService()
{
}

UserManagementService::~UserManagementService()
{
}

bool UserManagementService::createUser(const User &user, int creatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(creatorId, "CREATE_USER")) {
        return false;
    }

    // 验证用户信息
    if (user.getUsername().isEmpty() || user.getPassword().isEmpty()) {
        return false;
    }

    // 检查用户名是否已存在
    UserRepository userRepo;
    User existingUser = userRepo.findByUsername(user.getUsername());
    if (existingUser.getId() > 0) {
        return false; // 用户名已存在
    }

    // 加密密码
    QString hashedPassword = QString(QCryptographicHash::hash(user.getPassword().toUtf8(), QCryptographicHash::Sha256).toHex());

    User newUser = user;
    newUser.setPassword(hashedPassword);
    newUser.setCreateTime(QDateTime::currentDateTime());
    newUser.setUpdateTime(QDateTime::currentDateTime());
    newUser.setStatus("ACTIVE"); // 新用户默认激活

    bool result = userRepo.create(newUser);

    if (result) {
        logUserOperation(newUser.getId(), creatorId, "CREATE", "Created new user: " + newUser.getUsername());
    }

    return result;
}

bool UserManagementService::updateUser(const User &user, int updaterId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(updaterId, "UPDATE_USER")) {
        return false;
    }

    // 检查当前用户是否是本人或者拥有更高权限
    if (user.getId() != updaterId && !checkOperatorPermission(updaterId, "ADMIN")) {
        return false;
    }

    UserRepository userRepo;
    User existingUser = userRepo.findById(user.getId());
    if (existingUser.getId() <= 0) {
        return false; // 用户不存在
    }

    // 更新用户信息
    User updateUser = user;
    updateUser.setUpdateTime(QDateTime::currentDateTime());

    // 如果密码被修改，则加密
    if (!user.getPassword().isEmpty() && user.getPassword() != existingUser.getPassword()) {
        QString hashedPassword = QString(QCryptographicHash::hash(user.getPassword().toUtf8(), QCryptographicHash::Sha256).toHex());
        updateUser.setPassword(hashedPassword);
    }

    bool result = userRepo.update(updateUser);

    if (result) {
        logUserOperation(updateUser.getId(), updaterId, "UPDATE", "Updated user: " + updateUser.getUsername());
    }

    return result;
}

bool UserManagementService::deleteUser(int userId, int deleterId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(deleterId, "DELETE_USER")) {
        return false;
    }

    // 不能删除自己
    if (userId == deleterId) {
        return false;
    }

    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false; // 用户不存在
    }

    // 不能删除系统内置用户
    if (user.getUsername() == "admin") {
        return false;
    }

    bool result = userRepo.deleteById(userId);

    if (result) {
        logUserOperation(userId, deleterId, "DELETE", "Deleted user: " + user.getUsername());
    }

    return result;
}

User UserManagementService::getUserById(int userId) const
{
    UserRepository userRepo;
    return userRepo.findById(userId);
}

QList<User> UserManagementService::getAllUsers() const
{
    UserRepository userRepo;
    return userRepo.findAll();
}

QList<User> UserManagementService::getUsersByDepartment(int departmentId) const
{
    UserRepository userRepo;
    return userRepo.findByDepartmentId(departmentId);
}

QList<User> UserManagementService::searchUsers(const QString &keyword) const
{
    UserRepository userRepo;
    return userRepo.search(keyword);
}

bool UserManagementService::assignRoleToUser(int userId, int roleId, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "ASSIGN_ROLE")) {
        return false;
    }

    // 检查用户和角色是否存在
    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false;
    }

    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return false;
    }

    // 检查用户是否已拥有该角色
    if (userRepo.hasRole(userId, roleId)) {
        return true; // 已拥有该角色，视为成功
    }

    // 添加用户-角色关联
    if (!userRepo.addUserRole(userId, roleId)) {
        return false;
    }

    // 记录操作日志
    logUserOperation(userId, operatorId, "ASSIGN_ROLE",
                    QString("Assigned role '%1' to user '%2'").arg(role.getName()).arg(user.getUsername()));

    return true;
}

bool UserManagementService::removeRoleFromUser(int userId, int roleId, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "REMOVE_ROLE")) {
        return false;
    }

    // 检查用户和角色是否存在
    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false;
    }

    RoleRepository roleRepo;
    Role role = roleRepo.findById(roleId);
    if (role.getId() <= 0) {
        return false;
    }

    // 检查用户是否拥有该角色
    if (!userRepo.hasRole(userId, roleId)) {
        return true; // 用户没有该角色，视为成功
    }

    // 移除用户-角色关联
    if (!userRepo.removeUserRole(userId, roleId)) {
        return false;
    }

    // 记录操作日志
    logUserOperation(userId, operatorId, "REMOVE_ROLE",
                    QString("Removed role '%1' from user '%2'").arg(role.getName()).arg(user.getUsername()));

    return true;
}

QList<Role> UserManagementService::getUserRoles(int userId) const
{
    RoleRepository roleRepo;
    return roleRepo.findByUserId(userId);
}

bool UserManagementService::bulkImportUsers(const QList<User> &users, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "BULK_IMPORT")) {
        return false;
    }

    bool allSuccess = true;
    UserRepository userRepo;

    for (const User &user : users) {
        // 验证必填字段
        if (user.getUsername().isEmpty() || user.getPassword().isEmpty()) {
            allSuccess = false;
            continue;
        }

        // 检查用户名是否已存在
        User existingUser = userRepo.findByUsername(user.getUsername());
        if (existingUser.getId() > 0) {
            allSuccess = false;
            continue; // 跳过已存在的用户
        }

        // 加密密码
        QString hashedPassword = QString(QCryptographicHash::hash(user.getPassword().toUtf8(), QCryptographicHash::Sha256).toHex());

        User newUser = user;
        newUser.setPassword(hashedPassword);
        newUser.setCreateTime(QDateTime::currentDateTime());
        newUser.setUpdateTime(QDateTime::currentDateTime());
        newUser.setStatus("ACTIVE");

        if (!userRepo.create(newUser)) {
            allSuccess = false;
        } else {
            logUserOperation(newUser.getId(), operatorId, "BULK_CREATE",
                           "Bulk imported user: " + newUser.getUsername());
        }
    }

    return allSuccess;
}

bool UserManagementService::bulkExportUsers(const QList<int> &userIds, const QString &exportPath) const
{
    if (exportPath.isEmpty()) {
        return false;
    }

    UserRepository userRepo;
    QList<User> usersToExport;

    // 获取要导出的用户列表
    if (userIds.isEmpty()) {
        // 如果没有指定用户ID，导出所有用户
        usersToExport = userRepo.findAll();
    } else {
        // 导出指定用户
        for (int userId : userIds) {
            User user = userRepo.findById(userId);
            if (user.getId() > 0) {
                usersToExport.append(user);
            }
        }
    }

    if (usersToExport.isEmpty()) {
        return false;
    }

    // 根据文件扩展名决定导出格式
    if (exportPath.endsWith(".json", Qt::CaseInsensitive)) {
        return exportUsersToJson(usersToExport, exportPath);
    } else {
        // 默认导出为 CSV 格式
        return exportUsersToCsv(usersToExport, exportPath);
    }
}

bool UserManagementService::bulkResetPasswords(const QList<int> &userIds, const QString &newPassword, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "RESET_PASSWORD")) {
        return false;
    }

    bool allSuccess = true;
    UserRepository userRepo;

    QString hashedPassword = QString(QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex());

    for (int userId : userIds) {
        User user = userRepo.findById(userId);
        if (user.getId() <= 0) {
            allSuccess = false;
            continue;
        }

        user.setPassword(hashedPassword);
        user.setUpdateTime(QDateTime::currentDateTime());

        if (!userRepo.update(user)) {
            allSuccess = false;
        } else {
            logUserOperation(userId, operatorId, "BULK_RESET_PASSWORD",
                           "Bulk reset password for user: " + user.getUsername());
        }
    }

    return allSuccess;
}

bool UserManagementService::activateUser(int userId, int operatorId)
{
    return updateUserStatus(userId, "ACTIVE", operatorId);
}

bool UserManagementService::deactivateUser(int userId, int operatorId)
{
    return updateUserStatus(userId, "INACTIVE", operatorId);
}

bool UserManagementService::resetUserPassword(int userId, const QString &newPassword, int operatorId)
{
    // 检查操作者权限
    if (!checkOperatorPermission(operatorId, "RESET_PASSWORD")) {
        return false;
    }

    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false;
    }

    QString hashedPassword = QString(QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex());

    user.setPassword(hashedPassword);
    user.setUpdateTime(QDateTime::currentDateTime());

    bool result = userRepo.update(user);

    if (result) {
        logUserOperation(userId, operatorId, "RESET_PASSWORD",
                        "Reset password for user: " + user.getUsername());
    }

    return result;
}

bool UserManagementService::validateUserPermission(int userId, const QString &permission) const
{
    if (permission.isEmpty()) {
        return false;
    }

    // 检查用户的角色是否包含指定权限
    QList<Role> userRoles = getUserRoles(userId);

    if (userRoles.isEmpty()) {
        return false;
    }

    for (const Role &role : userRoles) {
        // 检查角色是否激活
        if (!role.isActive()) {
            continue;
        }

        QString permissionsStr = role.getPermissions();
        if (permissionsStr.isEmpty()) {
            continue;
        }

        // 权限字符串格式为逗号分隔的权限列表，如 "CREATE_USER,UPDATE_USER,DELETE_USER"
        QStringList permissionList = permissionsStr.split(',', QString::SkipEmptyParts);
        for (QString &perm : permissionList) {
            perm = perm.trimmed();
            if (perm == permission || perm == "*") {
                return true; // 找到匹配的权限或通配符权限
            }
        }
    }

    return false; // 未找到匹配的权限
}

bool UserManagementService::checkOperatorPermission(int operatorId, const QString &operation) const
{
    // 在实际实现中，这里会检查操作者的权限
    // 检查该操作者是否有执行指定操作的权限
    // 简化实现，返回true
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    return true;
}

bool UserManagementService::logUserOperation(int userId, int operatorId, const QString &operation, const QString &details) const
{
    // 实际实现中应记录用户操作日志到数据库
    // 这里应该调用日志服务或安全插件来记录操作
    Q_UNUSED(userId)
    Q_UNUSED(operatorId)
    Q_UNUSED(operation)
    Q_UNUSED(details)
    return true;
}

bool UserManagementService::updateUserStatus(int userId, const QString &newStatus, int operatorId)
{
    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false;
    }

    user.setStatus(newStatus);
    user.setUpdateTime(QDateTime::currentDateTime());

    bool result = userRepo.update(user);

    if (result) {
        logUserOperation(userId, operatorId, "UPDATE_STATUS",
                        QString("Changed user status to '%1': ").arg(newStatus) + user.getUsername());
    }

    return result;
}

bool UserManagementService::exportUsersToCsv(const QList<User> &users, const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 写入 CSV 头部
    out << "id,username,email,first_name,last_name,phone,department_id,status,created_at,updated_at\n";

    // 写入用户数据
    for (const User &user : users) {
        out << user.getId() << ","
            << escapeCsvField(user.getUsername()) << ","
            << escapeCsvField(user.getEmail()) << ","
            << escapeCsvField(user.getFirstName()) << ","
            << escapeCsvField(user.getLastName()) << ","
            << escapeCsvField(user.getPhone()) << ","
            << user.getDepartmentId() << ","
            << escapeCsvField(user.getStatus()) << ","
            << escapeCsvField(user.getCreateTime().toString(Qt::ISODate)) << ","
            << escapeCsvField(user.getUpdateTime().toString(Qt::ISODate)) << "\n";
    }

    file.close();
    return true;
}

bool UserManagementService::exportUsersToJson(const QList<User> &users, const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray jsonArray;

    for (const User &user : users) {
        QJsonObject userObj;
        userObj["id"] = user.getId();
        userObj["username"] = user.getUsername();
        userObj["email"] = user.getEmail();
        userObj["first_name"] = user.getFirstName();
        userObj["last_name"] = user.getLastName();
        userObj["phone"] = user.getPhone();
        userObj["department_id"] = user.getDepartmentId();
        userObj["status"] = user.getStatus();
        userObj["created_at"] = user.getCreateTime().toString(Qt::ISODate);
        userObj["updated_at"] = user.getUpdateTime().toString(Qt::ISODate);

        // 获取用户角色列表
        QJsonArray rolesArray;
        QList<Role> userRoles = getUserRoles(user.getId());
        for (const Role &role : userRoles) {
            QJsonObject roleObj;
            roleObj["id"] = role.getId();
            roleObj["name"] = role.getName();
            roleObj["description"] = role.getDescription();
            rolesArray.append(roleObj);
        }
        userObj["roles"] = rolesArray;

        jsonArray.append(userObj);
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}