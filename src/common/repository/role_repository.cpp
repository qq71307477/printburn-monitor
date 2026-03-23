#include "role_repository.h"

RoleRepository::RoleRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool RoleRepository::create_table() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS roles (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT,
            permissions TEXT,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_roles_name ON roles(name);
    )";

    return db_manager_->execute_query(sql);
}

std::unique_ptr<Role> RoleRepository::find_by_id(int id) {
    const char* sql = "SELECT id, name, description, permissions, is_active, created_at, updated_at FROM roles WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::unique_ptr<Role> role = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        role = std::make_unique<Role>();
        role->id = sqlite3_column_int(stmt, 0);
        role->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        role->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        role->permissions = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        role->is_active = sqlite3_column_int(stmt, 4) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    }

    sqlite3_finalize(stmt);
    return role;
}

std::vector<std::unique_ptr<Role>> RoleRepository::find_all() {
    const char* sql = "SELECT id, name, description, permissions, is_active, created_at, updated_at FROM roles";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    std::vector<std::unique_ptr<Role>> roles;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto role = std::make_unique<Role>();
        role->id = sqlite3_column_int(stmt, 0);
        role->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        role->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        role->permissions = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        role->is_active = sqlite3_column_int(stmt, 4) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        roles.push_back(std::move(role));
    }

    sqlite3_finalize(stmt);
    return roles;
}

std::unique_ptr<Role> RoleRepository::find_by_name(const std::string& name) {
    const char* sql = "SELECT id, name, description, permissions, is_active, created_at, updated_at FROM roles WHERE name = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    std::unique_ptr<Role> role = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        role = std::make_unique<Role>();
        role->id = sqlite3_column_int(stmt, 0);
        role->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        role->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        role->permissions = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        role->is_active = sqlite3_column_int(stmt, 4) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    }

    sqlite3_finalize(stmt);
    return role;
}

bool RoleRepository::create(Role& role) {
    const char* sql = "INSERT INTO roles (name, description, permissions, is_active) VALUES (?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, role.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, role.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role.permissions.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, role.is_active ? 1 : 0);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        role.id = sqlite3_last_insert_rowid(db_manager_->get_connection());
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

bool RoleRepository::update(const Role& role) {
    const char* sql = "UPDATE roles SET name = ?, description = ?, permissions = ?, is_active = ?, "
                      "updated_at = CURRENT_TIMESTAMP WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, role.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, role.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role.permissions.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, role.is_active ? 1 : 0);
    sqlite3_bind_int(stmt, 5, role.id);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

bool RoleRepository::remove(int id) {
    const char* sql = "DELETE FROM roles WHERE id = ?";

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