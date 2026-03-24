#include <QTest>
#include "services/AuthService.h"
#include "services/UserManagementService.h"
#include "services/RoleManagementService.h"
#include "common/repository/database_manager.h"

class TestPermission : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testAdminPermissions();
    void testSecAdminPermissions();
    void testAuditorPermissions();
    void testUserPermissions();

    void testUserManagementPermissions();
    void testRoleManagementPermissions();

private:
    std::unique_ptr<DatabaseManager> m_dbManager;
};

void TestPermission::initTestCase()
{
    m_dbManager = std::make_unique<DatabaseManager>(":memory:");
    m_dbManager->initialize();
}

void TestPermission::cleanupTestCase()
{
    m_dbManager.reset();
}

void TestPermission::testAdminPermissions()
{
    User admin = UserManagementService::getInstance().getUserById(1); // admin's ID is 1
    QVERIFY(AuthService::validateUserPermission(admin.getId(), "manage_users"));
    QVERIFY(AuthService::validateUserPermission(admin.getId(), "manage_roles"));
    // ... more permission checks
}

void TestPermission::testSecAdminPermissions()
{
    User secadmin = UserManagementService::getInstance().getUserById(2); // secadmin's ID is 2
    QVERIFY(AuthService::validateUserPermission(secadmin.getId(), "approve_tasks"));
    QVERIFY(!AuthService::validateUserPermission(secadmin.getId(), "manage_users"));
}

void TestPermission::testAuditorPermissions()
{
    User auditor = UserManagementService::getInstance().getUserById(3); // auditor's ID is 3
    QVERIFY(AuthService::validateUserPermission(auditor.getId(), "view_audit_logs"));
    QVERIFY(!AuthService::validateUserPermission(auditor.getId(), "manage_roles"));
}

void TestPermission::testUserPermissions()
{
    User testuser = UserManagementService::getInstance().getUserById(4); // testuser's ID is 4
    QVERIFY(AuthService::validateUserPermission(testuser.getId(), "apply_print_task"));
    QVERIFY(!AuthService::validateUserPermission(testuser.getId(), "manage_users"));
}

void TestPermission::testUserManagementPermissions()
{
    User admin = UserManagementService::getInstance().getUserById(1);
    User testuser = UserManagementService::getInstance().getUserById(4);

    User newUser;
    newUser.setUsername("new_user");
    newUser.setPassword("password123");

    // Admin can create user
    QVERIFY(UserManagementService::getInstance().createUser(newUser, admin.getId()));

    // Testuser cannot create user
    QVERIFY(!UserManagementService::getInstance().createUser(newUser, testuser.getId()));
}

void TestPermission::testRoleManagementPermissions()
{
    User admin = UserManagementService::getInstance().getUserById(1);
    User testuser = UserManagementService::getInstance().getUserById(4);

    Role newRole;
    newRole.setName("New Role");

    // Admin can create role
    QVERIFY(RoleManagementService::getInstance().createRole(newRole, admin.getId()));

    // Testuser cannot create role
    QVERIFY(!RoleManagementService::getInstance().createRole(newRole, testuser.getId()));
}

QTEST_MAIN(TestPermission)
#include "TestPermission.moc"
