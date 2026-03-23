#include "AuthService.h"
#include "src/common/repository/user_repository.h"
#include "src/common/repository/role_repository.h"
#include "models/user_model.h"
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStringList>

// 静态实例
static AuthService* instance = nullptr;

AuthService& AuthService::getInstance()
{
    if (!instance) {
        instance = new AuthService();
    }
    return *instance;
}

AuthService::AuthService() : m_isLoggedIn(false)
{
}

AuthService::~AuthService()
{
}

bool AuthService::authenticate(const QString &username, const QString &password)
{
    // 使用Repository访问数据库验证用户
    UserRepository userRepository;

    // 加密密码进行比较
    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    User user = userRepository.findByUsername(username);
    if (user.getId() > 0 && user.getPassword() == hashedPassword) { // 用户存在且密码匹配
        m_currentUser = user;
        m_isLoggedIn = true;

        // 记录登录日志
        // TODO: 实际实现中应该调用安全插件记录审计日志

        return true;
    }

    return false;
}

bool AuthService::logout()
{
    if (m_isLoggedIn) {
        // 记录登出日志
        // TODO: 实际实现中应该调用安全插件记录审计日志

        m_currentUser = User();
        m_isLoggedIn = false;
        return true;
    }
    return false;
}

bool AuthService::changePassword(const QString &username, const QString &oldPassword, const QString &newPassword)
{
    if (!authenticate(username, oldPassword)) {
        return false;  // 旧密码验证失败
    }

    // 检查新密码强度
    if (newPassword.length() < 8) {
        return false;  // 密码长度不足
    }

    // 检查密码复杂度
    QRegularExpression complexRegex("(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)");
    if (!complexRegex.match(newPassword).hasMatch()) {
        return false;  // 密码复杂度不够
    }

    UserRepository userRepository;
    QString hashedNewPassword = QString(QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex());

    User user = userRepository.findByUsername(username);
    user.setPassword(hashedNewPassword);

    return userRepository.update(user);
}

User AuthService::getCurrentUser() const
{
    return m_currentUser;
}

bool AuthService::hasPermission(const QString &permission) const
{
    if (!m_isLoggedIn) {
        return false;
    }

    if (permission.isEmpty()) {
        return false;
    }

    // 获取当前用户的角色列表
    RoleRepository roleRepo;
    QList<Role> roles = roleRepo.findByUserId(m_currentUser.getId());

    if (roles.isEmpty()) {
        return false;
    }

    // 遍历用户的所有角色，检查是否包含所需权限
    for (const Role &role : roles) {
        // 检查角色是否激活
        if (!role.isActive()) {
            continue;
        }

        QString permissionsStr = role.getPermissions();
        if (permissionsStr.isEmpty()) {
            continue;
        }

        // 权限字符串格式为逗号分隔的权限列表，如 "CREATE_USER,UPDATE_USER,DELETE_USER"
        QStringList permissionList = permissionsStr.split(',', Qt::SkipEmptyParts);
        for (QString &perm : permissionList) {
            perm = perm.trimmed();
            if (perm == permission || perm == "*") {
                return true; // 找到匹配的权限或通配符权限
            }
        }
    }

    return false; // 未找到匹配的权限
}