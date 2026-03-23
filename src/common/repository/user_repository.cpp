#include "user_repository.h"
#include <sstream>
#include <iomanip>
#include <ctime>

UserRepository::UserRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool UserRepository::create_table() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            email TEXT NOT NULL UNIQUE,
            role_id INTEGER NOT NULL,
            department_id INTEGER,
            first_name TEXT,
            last_name TEXT,
            phone TEXT,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (role_id) REFERENCES roles(id),
            FOREIGN KEY (department_id) REFERENCES departments(id)
        );

        CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
        CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
        CREATE INDEX IF NOT EXISTS idx_users_role_id ON users(role_id);
        CREATE INDEX IF NOT EXISTS idx_users_department_id ON users(department_id);
    )";

    return db_manager_->execute_query(sql);
}

std::unique_ptr<User> UserRepository::find_by_id(int id) {
    const char* sql = "SELECT id, username, password_hash, email, role_id, department_id, "
                      "first_name, last_name, phone, is_active, created_at, updated_at FROM users WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->role_id = sqlite3_column_int(stmt, 4);
        user->department_id = sqlite3_column_int(stmt, 5);
        user->first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        user->phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        user->is_active = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    }

    sqlite3_finalize(stmt);
    return user;
}

std::vector<std::unique_ptr<User>> UserRepository::find_all() {
    const char* sql = "SELECT id, username, password_hash, email, role_id, department_id, "
                      "first_name, last_name, phone, is_active, created_at, updated_at FROM users";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    std::vector<std::unique_ptr<User>> users;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->role_id = sqlite3_column_int(stmt, 4);
        user->department_id = sqlite3_column_int(stmt, 5);
        user->first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        user->phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        user->is_active = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        users.push_back(std::move(user));
    }

    sqlite3_finalize(stmt);
    return users;
}

std::unique_ptr<User> UserRepository::find_by_username(const std::string& username) {
    const char* sql = "SELECT id, username, password_hash, email, role_id, department_id, "
                      "first_name, last_name, phone, is_active, created_at, updated_at FROM users WHERE username = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->role_id = sqlite3_column_int(stmt, 4);
        user->department_id = sqlite3_column_int(stmt, 5);
        user->first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        user->phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        user->is_active = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    }

    sqlite3_finalize(stmt);
    return user;
}

std::unique_ptr<User> UserRepository::find_by_email(const std::string& email) {
    const char* sql = "SELECT id, username, password_hash, email, role_id, department_id, "
                      "first_name, last_name, phone, is_active, created_at, updated_at FROM users WHERE email = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);

    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->role_id = sqlite3_column_int(stmt, 4);
        user->department_id = sqlite3_column_int(stmt, 5);
        user->first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        user->phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        user->is_active = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    }

    sqlite3_finalize(stmt);
    return user;
}

bool UserRepository::create(User& user) {
    const char* sql = "INSERT INTO users (username, password_hash, email, role_id, department_id, "
                      "first_name, last_name, phone, is_active) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user.email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, user.role_id);
    sqlite3_bind_int(stmt, 5, user.department_id);
    sqlite3_bind_text(stmt, 6, user.first_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, user.last_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, user.phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, user.is_active ? 1 : 0);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        user.id = sqlite3_last_insert_rowid(db_manager_->get_connection());
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

bool UserRepository::update(const User& user) {
    const char* sql = "UPDATE users SET username = ?, password_hash = ?, email = ?, role_id = ?, "
                      "department_id = ?, first_name = ?, last_name = ?, phone = ?, is_active = ?, "
                      "updated_at = CURRENT_TIMESTAMP WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user.email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, user.role_id);
    sqlite3_bind_int(stmt, 5, user.department_id);
    sqlite3_bind_text(stmt, 6, user.first_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, user.last_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, user.phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, user.is_active ? 1 : 0);
    sqlite3_bind_int(stmt, 10, user.id);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

bool UserRepository::remove(int id) {
    const char* sql = "DELETE FROM users WHERE id = ?";

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

std::vector<std::unique_ptr<User>> UserRepository::find_by_department(int department_id) {
    const char* sql = "SELECT id, username, password_hash, email, role_id, department_id, "
                      "first_name, last_name, phone, is_active, created_at, updated_at FROM users WHERE department_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_int(stmt, 1, department_id);

    std::vector<std::unique_ptr<User>> users;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->role_id = sqlite3_column_int(stmt, 4);
        user->department_id = sqlite3_column_int(stmt, 5);
        user->first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        user->phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        user->is_active = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        users.push_back(std::move(user));
    }

    sqlite3_finalize(stmt);
    return users;
}

std::vector<std::unique_ptr<User>> UserRepository::find_by_role(int role_id) {
    const char* sql = "SELECT id, username, password_hash, email, role_id, department_id, "
                      "first_name, last_name, phone, is_active, created_at, updated_at FROM users WHERE role_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_int(stmt, 1, role_id);

    std::vector<std::unique_ptr<User>> users;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->role_id = sqlite3_column_int(stmt, 4);
        user->department_id = sqlite3_column_int(stmt, 5);
        user->first_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->last_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        user->phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        user->is_active = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        users.push_back(std::move(user));
    }

    sqlite3_finalize(stmt);
    return users;
}