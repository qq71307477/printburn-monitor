#include "AuthService.h"
#include "src/common/repository/user_repository.h"
#include "src/common/repository/role_repository.h"
#include "models/user_model.h"
#include "common/PluginManager.h"
#include "qt_compat.h"
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStringList>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>
#include <mutex>

// 静态实例
static std::once_flag onceFlag;
static AuthService* instance = nullptr;

AuthService& AuthService::getInstance()
{
    std::call_once(onceFlag, []() {
        instance = new AuthService();
    });
    return *instance;
}

AuthService::AuthService() : m_isLoggedIn(false)
{
}

AuthService::~AuthService()
{
}

// 生成随机 salt
QString AuthService::generateSalt(int length)
{
    QByteArray salt;
    salt.resize(length);

    for (int i = 0; i < length; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }

    return QString(salt.toBase64());
}

// 使用 PBKDF2 风格的多次迭代 SHA256 哈希
QString AuthService::hashPassword(const QString& password, const QString& salt, int iterations)
{
    QByteArray saltBytes = QByteArray::fromBase64(salt.toUtf8());
    QByteArray hash = password.toUtf8() + saltBytes;

    // 多次迭代哈希
    for (int i = 0; i < iterations; ++i) {
        hash = QCryptographicHash::hash(hash, QCryptographicHash::Sha256);
    }

    return QString(hash.toHex());
}

// 创建完整的密码存储格式：$pbkdf2$iterations$salt$hash
QString AuthService::createPasswordHash(const QString& password)
{
    QString salt = generateSalt(16);
    QString hash = hashPassword(password, salt, 10000);
    return QString("$pbkdf2$10000$%1$%2").arg(salt, hash);
}

// 验证密码
bool AuthService::verifyPassword(const QString& password, const QString& storedHash)
{
    // 检查是否是新格式
    if (storedHash.startsWith("$pbkdf2$")) {
        QStringList parts = storedHash.split('$', SPLIT_SKIP_EMPTY);
        if (parts.size() != 4) {
            qWarning() << "Invalid password hash format";
            return false;
        }

        // parts[0] = "pbkdf2", parts[1] = iterations, parts[2] = salt, parts[3] = hash
        bool ok = false;
        int iterations = parts[1].toInt(&ok);
        if (!ok || iterations <= 0) {
            qWarning() << "Invalid iterations in password hash";
            return false;
        }

        QString salt = parts[2];
        QString expectedHash = parts[3];
        QString computedHash = hashPassword(password, salt, iterations);

        return computedHash == expectedHash;
    }

    // 兼容旧格式：纯 SHA256 哈希（用于迁移）
    QString legacyHash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    return legacyHash == storedHash;
}

bool AuthService::authenticate(const QString &username, const QString &password)
{
    // 使用Repository访问数据库验证用户
    UserRepository userRepository;

    User user = userRepository.findByUsername(username);
    if (user.getId() > 0 && verifyPassword(password, user.getPassword())) { // 用户存在且密码匹配
        m_currentUser = user;
        m_isLoggedIn = true;

        // 记录登录日志
        QVariantMap logParams;
        logParams["userId"] = user.getId();
        logParams["action"] = "LOGIN";
        logParams["resource"] = user.getUsername();
        PluginManager::getInstance().executePlugin("SecurityPlugin", "generateAuditLog", logParams);

        return true;
    }

    return false;
}

bool AuthService::logout()
{
    if (m_isLoggedIn) {
        // 记录登出日志
        QVariantMap logParams;
        logParams["userId"] = m_currentUser.getId();
        logParams["action"] = "LOGOUT";
        logParams["resource"] = m_currentUser.getUsername();
        PluginManager::getInstance().executePlugin("SecurityPlugin", "generateAuditLog", logParams);

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
    QString hashedNewPassword = createPasswordHash(newPassword);

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
        QStringList permissionList = permissionsStr.split(',', SPLIT_SKIP_EMPTY);
        for (QString &perm : permissionList) {
            perm = perm.trimmed();
            if (perm == permission || perm == "*") {
                return true; // 找到匹配的权限或通配符权限
            }
        }
    }

    return false; // 未找到匹配的权限
}

QStringList AuthService::getCurrentUserRoleNames() const
{
    QStringList roleNames;

    if (!m_isLoggedIn) {
        return roleNames;
    }

    RoleRepository roleRepo;
    QList<Role> roles = roleRepo.findByUserId(m_currentUser.getId());

    for (const Role &role : roles) {
        if (role.isActive()) {
            roleNames.append(role.getName());
        }
    }

    return roleNames;
}

bool AuthService::hasRole(const QString &roleName) const
{
    if (!m_isLoggedIn) {
        return false;
    }

    QStringList roleNames = getCurrentUserRoleNames();
    return roleNames.contains(roleName);
}

bool AuthService::validateUserPermission(int userId, const QString &permission)
{
    if (permission.isEmpty()) {
        return false;
    }

    // 检查用户的角色是否包含指定权限
    RoleRepository roleRepo;
    QList<Role> userRoles = roleRepo.findByUserId(userId);

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

        // 权限字符串格式为逗号分隔的权限列表
        QStringList permissionList = permissionsStr.split(',', SPLIT_SKIP_EMPTY);
        for (QString &perm : permissionList) {
            perm = perm.trimmed();
            if (perm == permission || perm == "*") {
                return true; // 找到匹配的权限或通配符权限
            }
        }
    }

    return false; // 未找到匹配的权限
}