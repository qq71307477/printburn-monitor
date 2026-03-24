#ifndef USERMANAGEMENTSERVICE_H
#define USERMANAGEMENTSERVICE_H

#include <QString>
#include <QList>
#include <QVariantMap>
#include "../models/user_model.h"     // 用户模型
#include "../models/role_model.h"     // 角色模型
#include "../models/department_model.h" // 部门模型

class UserManagementService
{
public:
    // 单例获取方法
    static UserManagementService& getInstance();

    // 用户管理功能
    bool createUser(const User &user, int creatorId);
    bool updateUser(const User &user, int updaterId);
    bool deleteUser(int userId, int deleterId);
    User getUserById(int userId) const;
    QList<User> getAllUsers() const;
    QList<User> getUsersByDepartment(int departmentId) const;
    QList<User> searchUsers(const QString &keyword) const;

    // 用户角色管理
    bool assignRoleToUser(int userId, int roleId, int operatorId);
    bool removeRoleFromUser(int userId, int roleId, int operatorId);
    QList<Role> getUserRoles(int userId) const;

    // 批量操作
    bool bulkImportUsers(const QList<User> &users, int operatorId);
    bool bulkExportUsers(const QList<int> &userIds, const QString &exportPath) const;
    bool bulkResetPasswords(const QList<int> &userIds, const QString &newPassword, int operatorId);

    // 用户激活/停用
    bool activateUser(int userId, int operatorId);
    bool deactivateUser(int userId, int operatorId);

    // 重置密码
    bool resetUserPassword(int userId, const QString &newPassword, int operatorId);


private:
    UserManagementService();  // 私有构造函数，确保单例
    ~UserManagementService();

    // 检查操作者权限
    bool checkOperatorPermission(int operatorId, const QString &operation) const;

    // 记录用户管理操作日志
    bool logUserOperation(int userId, int operatorId, const QString &operation, const QString &details = "") const;

    // 更新用户状态（内部方法）
    bool updateUserStatus(int userId, const QString &newStatus, int operatorId);

    // 用户导出辅助方法
    bool exportUsersToCsv(const QList<User> &users, const QString &filePath) const;
    bool exportUsersToJson(const QList<User> &users, const QString &filePath) const;
};

#endif // USERMANAGEMENTSERVICE_H