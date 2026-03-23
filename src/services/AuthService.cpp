#include "AuthService.h"
#include "../src/common/repository/UserRepository.h"
#include "../models/user_model.h"
#include <QCryptographicHash>
#include <QRegularExpression>

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

    // 在实际实现中，这会检查用户的权限
    // 可以通过用户的角色来确定权限

    // 临时实现 - 检查用户角色是否包含所需权限
    // 这里应该连接到权限验证系统
    return true; // 简化实现，实际应根据用户角色和权限进行验证
}