#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <string>
#include <ctime>

struct User {
    int id;
    std::string username;
    std::string password_hash;
    std::string email;
    int role_id;
    int department_id;
    std::string first_name;
    std::string last_name;
    std::string phone;
    bool is_active;
    std::time_t created_at;
    std::time_t updated_at;

    User() : id(0), role_id(0), department_id(0), is_active(true) {}
};

#endif // USER_MODEL_H