#ifndef ROLE_REPOSITORY_H
#define ROLE_REPOSITORY_H

#include "models/role_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <vector>
#include <memory>

class RoleRepository {
private:
    DatabaseManager* db_manager_;

public:
    explicit RoleRepository(DatabaseManager* db_manager);

    bool create_table();
    std::unique_ptr<Role> find_by_id(int id);
    std::vector<std::unique_ptr<Role>> find_all();
    std::unique_ptr<Role> find_by_name(const std::string& name);
    bool create(Role& role);
    bool update(const Role& role);
    bool remove(int id);
};

#endif // ROLE_REPOSITORY_H