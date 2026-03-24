#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "user_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <vector>
#include <memory>
#include <optional>

class UserRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    UserRepository();
    explicit UserRepository(DatabaseManager* db_manager);
    ~UserRepository() = default;

    bool create_table();

    // Qt-style methods for services
    User findById(int id);
    User findByUsername(const QString& username);
    QList<User> findAll();
    bool deleteById(int id);
    bool create(User& user);
    bool update(const User& user);
    QList<User> search(const QString& keyword);
    QList<User> findByDepartmentId(int department_id);
    QList<User> findByRoleId(int roleId); // New signature

    // User-Role relationship management
    bool addUserRole(int userId, int roleId);
    bool removeUserRole(int userId, int roleId);
    bool hasRole(int userId, int roleId);

    // Legacy snake_case methods for backward compatibility
    std::unique_ptr<User> find_by_id(int id);
    std::vector<std::unique_ptr<User>> find_all();
    std::unique_ptr<User> find_by_username(const std::string& username);
    std::unique_ptr<User> find_by_email(const std::string& email);
    bool remove(int id);
    std::vector<std::unique_ptr<User>> find_by_department(int department_id);
    // Removed: std::vector<std::unique_ptr<User>> find_by_role(int role_id);
};

#endif // USER_REPOSITORY_H
