#include "department_repository.h"

DepartmentRepository::DepartmentRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool DepartmentRepository::create_table() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS departments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            manager_id INTEGER,
            parent_department_id INTEGER,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (manager_id) REFERENCES users(id),
            FOREIGN KEY (parent_department_id) REFERENCES departments(id)
        );

        CREATE INDEX IF NOT EXISTS idx_departments_name ON departments(name);
    )";

    return db_manager_->execute_query(sql);
}

std::unique_ptr<Department> DepartmentRepository::find_by_id(int id) {
    const char* sql = "SELECT id, name, description, manager_id, parent_department_id, is_active, created_at, updated_at FROM departments WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::unique_ptr<Department> department = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        department = std::make_unique<Department>();
        department->id = sqlite3_column_int(stmt, 0);
        department->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        department->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        department->manager_id = sqlite3_column_int(stmt, 3);
        department->parent_department_id = sqlite3_column_int(stmt, 4);
        department->is_active = sqlite3_column_int(stmt, 5) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    }

    sqlite3_finalize(stmt);
    return department;
}

std::vector<std::unique_ptr<Department>> DepartmentRepository::find_all() {
    const char* sql = "SELECT id, name, description, manager_id, parent_department_id, is_active, created_at, updated_at FROM departments";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    std::vector<std::unique_ptr<Department>> departments;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto department = std::make_unique<Department>();
        department->id = sqlite3_column_int(stmt, 0);
        department->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        department->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        department->manager_id = sqlite3_column_int(stmt, 3);
        department->parent_department_id = sqlite3_column_int(stmt, 4);
        department->is_active = sqlite3_column_int(stmt, 5) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        departments.push_back(std::move(department));
    }

    sqlite3_finalize(stmt);
    return departments;
}

std::unique_ptr<Department> DepartmentRepository::find_by_name(const std::string& name) {
    const char* sql = "SELECT id, name, description, manager_id, parent_department_id, is_active, created_at, updated_at FROM departments WHERE name = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    std::unique_ptr<Department> department = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        department = std::make_unique<Department>();
        department->id = sqlite3_column_int(stmt, 0);
        department->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        department->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        department->manager_id = sqlite3_column_int(stmt, 3);
        department->parent_department_id = sqlite3_column_int(stmt, 4);
        department->is_active = sqlite3_column_int(stmt, 5) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    }

    sqlite3_finalize(stmt);
    return department;
}

bool DepartmentRepository::create(Department& department) {
    const char* sql = "INSERT INTO departments (name, description, manager_id, parent_department_id, is_active) VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, department.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, department.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, department.manager_id);
    sqlite3_bind_int(stmt, 4, department.parent_department_id);
    sqlite3_bind_int(stmt, 5, department.is_active ? 1 : 0);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        department.id = sqlite3_last_insert_rowid(db_manager_->get_connection());
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

bool DepartmentRepository::update(const Department& department) {
    const char* sql = "UPDATE departments SET name = ?, description = ?, manager_id = ?, parent_department_id = ?, is_active = ?, "
                      "updated_at = CURRENT_TIMESTAMP WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, department.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, department.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, department.manager_id);
    sqlite3_bind_int(stmt, 4, department.parent_department_id);
    sqlite3_bind_int(stmt, 5, department.is_active ? 1 : 0);
    sqlite3_bind_int(stmt, 6, department.id);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

bool DepartmentRepository::remove(int id) {
    const char* sql = "DELETE FROM departments WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE && sqlite3_changes(db_manager_->get_connection()) > 0);

    sqlite3_finalize(stmt);
    return result;
}