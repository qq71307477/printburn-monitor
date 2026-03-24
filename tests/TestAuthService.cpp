#include "TestAuthService.h"
#include <QCryptographicHash>

void TestAuthService::initTestCase()
{
    authService = &AuthService::getInstance();
    userRepository = new UserRepository();

    // 创建测试用户
    User testUser;
    testUser.setUsername("testuser");
    testUser.setPassword(QString(QCryptographicHash::hash("testpass", QCryptographicHash::Sha256).toHex()));
    testUser.setRoleId(1);  // 使用 setRoleId 而非 setRole
    testUser.setStatus("ACTIVE");
    userRepository->create(testUser);
}

void TestAuthService::cleanupTestCase()
{
    // 清理测试用户
    User testUser = userRepository->findByUsername("testuser");
    if (testUser.getId() > 0) {
        userRepository->deleteById(testUser.getId());
    }

    delete userRepository;
}

void TestAuthService::testAuthenticateValidUser()
{
    bool result = authService->authenticate("testuser", "testpass");
    QVERIFY(result);

    // 验证用户已登录
    User currentUser = authService->getCurrentUser();
    QCOMPARE(currentUser.getUsername(), QString("testuser"));
}

void TestAuthService::testAuthenticateInvalidUser()
{
    bool result = authService->authenticate("invaliduser", "invalidpass");
    QVERIFY(!result);

    // 验证没有用户登录
    User currentUser = authService->getCurrentUser();
    QCOMPARE(currentUser.getId(), 0);
}

void TestAuthService::testLogout()
{
    // 先登录
    authService->authenticate("testuser", "testpass");
    QVERIFY(authService->getCurrentUser().getId() > 0);

    // 登出
    bool result = authService->logout();
    QVERIFY(result);

    // 验证已登出
    User currentUser = authService->getCurrentUser();
    QCOMPARE(currentUser.getId(), 0);
}

void TestAuthService::testChangePassword()
{
    // 登录测试用户
    authService->authenticate("testuser", "testpass");

    bool result = authService->changePassword("testuser", "testpass", "newpass123");
    QVERIFY(result);

    // 登出并尝试使用新密码登录
    authService->logout();
    bool loginResult = authService->authenticate("testuser", "newpass123");
    QVERIFY(loginResult);

    // 改回原密码
    authService->changePassword("testuser", "newpass123", "testpass");
    authService->logout();
}

void TestAuthService::testHasPermission()
{
    bool result = authService->hasPermission("some_permission");
    QVERIFY(result); // 当前是简化实现，总是返回true
}

QTEST_MAIN(TestAuthService)
#include "TestAuthService.moc"