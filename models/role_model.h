#ifndef ROLE_MODEL_H
#define ROLE_MODEL_H

#include <string>
#include <ctime>

struct Role {
    int id;
    std::string name;
    std::string description;
    std::string permissions;
    bool is_active;
    std::time_t created_at;
    std::time_t updated_at;

    Role() : id(0), is_active(true) {}
};

#endif // ROLE_MODEL_H