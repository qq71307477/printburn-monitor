#ifndef DEPARTMENT_REPOSITORY_H
#define DEPARTMENT_REPOSITORY_H

#include "department_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <vector>
#include <memory>

class DepartmentRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    DepartmentRepository();
    explicit DepartmentRepository(DatabaseManager* db_manager);

    bool create_table();
    std::unique_ptr<Department> find_by_id(int id);
    std::vector<std::unique_ptr<Department>> find_all();
    bool create(Department& department);
    bool update(const Department& department);
    bool remove(int id);
};

#endif // DEPARTMENT_REPOSITORY_H