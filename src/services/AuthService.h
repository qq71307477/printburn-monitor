#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <QString>
#include <QVariantMap>
#include "../models/user_model.h"  // 使用正确的用户模型路径

class AuthService
{
public:
    // 单例获取方法
    static AuthService& getInstance();

    // 登录验证
    bool authenticate(const QString &username, const QString &password);

    // 登出
    bool logout();

    // 密码修改
    bool changePassword(const QString &username, const QString &oldPassword, const QString &newPassword);

    // 获取当前登录用户信息
    User getCurrentUser() const;

    // 检查用户权限
    bool hasPermission(const QString &permission) const;

    // 获取当前用户角色名称列表
    QStringList getCurrentUserRoleNames() const;

    // 检查当前用户是否拥有指定角色
    bool hasRole(const QString &roleName) const;

    // 静态方法，验证指定用户是否拥有特定权限
    static bool validateUserPermission(int userId, const QString &permission);

    // 密码哈希辅助函数
    static QString generateSalt(int length = 16);
    static QString hashPassword(const QString& password, const QString& salt, int iterations = 10000);
    static QString createPasswordHash(const QString& password);
    static bool verifyPassword(const QString& password, const QString& storedHash);


private:
    AuthService();  // 私有构造函数，确保单例
    ~AuthService();

    User m_currentUser;
    bool m_isLoggedIn;
};

#endif // AUTHSERVICE_H