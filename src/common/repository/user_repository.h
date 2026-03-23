#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "../models/user_model.h"
#include "database_manager.h"
#include <vector>
#include <memory>
#include <optional>

class UserRepository {
private:
    DatabaseManager* db_manager_;

public:
    explicit UserRepository(DatabaseManager* db_manager);

    bool create_table();
    std::unique_ptr<User> find_by_id(int id);
    std::vector<std::unique_ptr<User>> find_all();
    std::unique_ptr<User> find_by_username(const std::string& username);
    std::unique_ptr<User> find_by_email(const std::string& email);
    bool create(User& user);
    bool update(const User& user);
    bool remove(int id);
    std::vector<std::unique_ptr<User>> find_by_department(int department_id);
    std::vector<std::unique_ptr<User>> find_by_role(int role_id);
};

#endif // USER_REPOSITORY_H