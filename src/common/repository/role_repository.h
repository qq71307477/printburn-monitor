#ifndef ROLE_REPOSITORY_H
#define ROLE_REPOSITORY_H

#include "role_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <vector>
#include <memory>

class RoleRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    RoleRepository();
    explicit RoleRepository(DatabaseManager* db_manager);
    ~RoleRepository() = default;

    bool create_table();

    // Qt-style methods for services
    Role findById(int id);
    Role findByName(const QString& name);
    QList<Role> findByUserId(int userId);
    QList<Role> findAll();
    QList<Role> search(const QString& keyword);
    bool deleteById(int id);

    // Legacy snake_case methods for backward compatibility
    std::unique_ptr<Role> find_by_id(int id);
    std::vector<std::unique_ptr<Role>> find_all();
    std::unique_ptr<Role> find_by_name(const std::string& name);
    bool create(Role& role);
    bool update(const Role& role);
    bool remove(int id);
};

#endif // ROLE_REPOSITORY_H