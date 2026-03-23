#ifndef TEST_AUTHSERVICE_H
#define TEST_AUTHSERVICE_H

#include <QtTest>
#include <QObject>
#include "../src/services/AuthService.h"
#include "src/common/repository/UserRepository.h"
#include "models/user_model.h"

class TestAuthService : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testAuthenticateValidUser();
    void testAuthenticateInvalidUser();
    void testLogout();
    void testChangePassword();
    void testHasPermission();

private:
    AuthService *authService;
    UserRepository *userRepository;
};

#endif // TEST_AUTHSERVICE_H