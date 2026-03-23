#include "user_repository.h"
#include <QVariant>

// Static member initialization
DatabaseManager* UserRepository::default_db_manager_ = nullptr;

UserRepository::UserRepository() : db_manager_(default_db_manager_) {}

UserRepository::UserRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool UserRepository::create_table() {
    if (!db_manager_) return false;

    QString sql = R"(
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

// Qt-style methods for services
User UserRepository::findById(int id) {
    User user;
    if (!db_manager_) return user;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        user.id = query.value(0).toInt();
        user.username = query.value(1).toString().toStdString();
        user.password_hash = query.value(2).toString().toStdString();
        user.email = query.value(3).toString().toStdString();
        user.role_id = query.value(4).toInt();
        user.department_id = query.value(5).toInt();
        user.first_name = query.value(6).toString().toStdString();
        user.last_name = query.value(7).toString().toStdString();
        user.phone = query.value(8).toString().toStdString();
        user.is_active = query.value(9).toBool();
    }
    return user;
}

User UserRepository::findByUsername(const QString& username) {
    User user;
    if (!db_manager_) return user;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE username = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        user.id = query.value(0).toInt();
        user.username = query.value(1).toString().toStdString();
        user.password_hash = query.value(2).toString().toStdString();
        user.email = query.value(3).toString().toStdString();
        user.role_id = query.value(4).toInt();
        user.department_id = query.value(5).toInt();
        user.first_name = query.value(6).toString().toStdString();
        user.last_name = query.value(7).toString().toStdString();
        user.phone = query.value(8).toString().toStdString();
        user.is_active = query.value(9).toBool();
    }
    return user;
}

QList<User> UserRepository::findAll() {
    QList<User> users;
    if (!db_manager_) return users;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, username, password_hash, email, role_id, department_id, "
                   "first_name, last_name, phone, is_active FROM users")) {
        while (query.next()) {
            User user;
            user.id = query.value(0).toInt();
            user.username = query.value(1).toString().toStdString();
            user.password_hash = query.value(2).toString().toStdString();
            user.email = query.value(3).toString().toStdString();
            user.role_id = query.value(4).toInt();
            user.department_id = query.value(5).toInt();
            user.first_name = query.value(6).toString().toStdString();
            user.last_name = query.value(7).toString().toStdString();
            user.phone = query.value(8).toString().toStdString();
            user.is_active = query.value(9).toBool();
            users.append(user);
        }
    }
    return users;
}

bool UserRepository::deleteById(int id) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM users WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool UserRepository::create(User& user) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO users (username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(QString::fromStdString(user.username));
    query.addBindValue(QString::fromStdString(user.password_hash));
    query.addBindValue(QString::fromStdString(user.email));
    query.addBindValue(user.role_id);
    query.addBindValue(user.department_id);
    query.addBindValue(QString::fromStdString(user.first_name));
    query.addBindValue(QString::fromStdString(user.last_name));
    query.addBindValue(QString::fromStdString(user.phone));
    query.addBindValue(user.is_active);

    if (query.exec()) {
        user.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool UserRepository::update(const User& user) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE users SET username = ?, password_hash = ?, email = ?, role_id = ?, "
                  "department_id = ?, first_name = ?, last_name = ?, phone = ?, is_active = ?, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(QString::fromStdString(user.username));
    query.addBindValue(QString::fromStdString(user.password_hash));
    query.addBindValue(QString::fromStdString(user.email));
    query.addBindValue(user.role_id);
    query.addBindValue(user.department_id);
    query.addBindValue(QString::fromStdString(user.first_name));
    query.addBindValue(QString::fromStdString(user.last_name));
    query.addBindValue(QString::fromStdString(user.phone));
    query.addBindValue(user.is_active);
    query.addBindValue(user.id);

    return query.exec();
}

QList<User> UserRepository::search(const QString& keyword) {
    QList<User> users;
    if (!db_manager_) return users;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users "
                  "WHERE username LIKE ? OR first_name LIKE ? OR last_name LIKE ? OR email LIKE ?");
    QString pattern = "%" + keyword + "%";
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    query.addBindValue(pattern);

    if (query.exec()) {
        while (query.next()) {
            User user;
            user.id = query.value(0).toInt();
            user.username = query.value(1).toString().toStdString();
            user.password_hash = query.value(2).toString().toStdString();
            user.email = query.value(3).toString().toStdString();
            user.role_id = query.value(4).toInt();
            user.department_id = query.value(5).toInt();
            user.first_name = query.value(6).toString().toStdString();
            user.last_name = query.value(7).toString().toStdString();
            user.phone = query.value(8).toString().toStdString();
            user.is_active = query.value(9).toBool();
            users.append(user);
        }
    }
    return users;
}

QList<User> UserRepository::findByDepartmentId(int department_id) {
    QList<User> users;
    if (!db_manager_) return users;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE department_id = ?");
    query.addBindValue(department_id);

    if (query.exec()) {
        while (query.next()) {
            User user;
            user.id = query.value(0).toInt();
            user.username = query.value(1).toString().toStdString();
            user.password_hash = query.value(2).toString().toStdString();
            user.email = query.value(3).toString().toStdString();
            user.role_id = query.value(4).toInt();
            user.department_id = query.value(5).toInt();
            user.first_name = query.value(6).toString().toStdString();
            user.last_name = query.value(7).toString().toStdString();
            user.phone = query.value(8).toString().toStdString();
            user.is_active = query.value(9).toBool();
            users.append(user);
        }
    }
    return users;
}

QList<User> UserRepository::findByRoleId(int roleId) {
    QList<User> users;
    if (!db_manager_) return users;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE role_id = ?");
    query.addBindValue(roleId);

    if (query.exec()) {
        while (query.next()) {
            User user;
            user.id = query.value(0).toInt();
            user.username = query.value(1).toString().toStdString();
            user.password_hash = query.value(2).toString().toStdString();
            user.email = query.value(3).toString().toStdString();
            user.role_id = query.value(4).toInt();
            user.department_id = query.value(5).toInt();
            user.first_name = query.value(6).toString().toStdString();
            user.last_name = query.value(7).toString().toStdString();
            user.phone = query.value(8).toString().toStdString();
            user.is_active = query.value(9).toBool();
            users.append(user);
        }
    }
    return users;
}

bool UserRepository::addUserRole(int userId, int roleId) {
    if (!db_manager_) return false;

    // Check if the relationship already exists
    if (hasRole(userId, roleId)) {
        return true; // Already exists, consider it success
    }

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO user_roles (user_id, role_id) VALUES (?, ?)");
    query.addBindValue(userId);
    query.addBindValue(roleId);

    return query.exec();
}

bool UserRepository::removeUserRole(int userId, int roleId) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM user_roles WHERE user_id = ? AND role_id = ?");
    query.addBindValue(userId);
    query.addBindValue(roleId);

    return query.exec();
}

bool UserRepository::hasRole(int userId, int roleId) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT COUNT(*) FROM user_roles WHERE user_id = ? AND role_id = ?");
    query.addBindValue(userId);
    query.addBindValue(roleId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// Legacy snake_case methods for backward compatibility
std::unique_ptr<User> UserRepository::find_by_id(int id) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        auto user = std::make_unique<User>();
        user->id = query.value(0).toInt();
        user->username = query.value(1).toString().toStdString();
        user->password_hash = query.value(2).toString().toStdString();
        user->email = query.value(3).toString().toStdString();
        user->role_id = query.value(4).toInt();
        user->department_id = query.value(5).toInt();
        user->first_name = query.value(6).toString().toStdString();
        user->last_name = query.value(7).toString().toStdString();
        user->phone = query.value(8).toString().toStdString();
        user->is_active = query.value(9).toBool();
        return user;
    }
    return nullptr;
}

std::vector<std::unique_ptr<User>> UserRepository::find_all() {
    std::vector<std::unique_ptr<User>> users;
    if (!db_manager_) return users;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, username, password_hash, email, role_id, department_id, "
                   "first_name, last_name, phone, is_active FROM users")) {
        while (query.next()) {
            auto user = std::make_unique<User>();
            user->id = query.value(0).toInt();
            user->username = query.value(1).toString().toStdString();
            user->password_hash = query.value(2).toString().toStdString();
            user->email = query.value(3).toString().toStdString();
            user->role_id = query.value(4).toInt();
            user->department_id = query.value(5).toInt();
            user->first_name = query.value(6).toString().toStdString();
            user->last_name = query.value(7).toString().toStdString();
            user->phone = query.value(8).toString().toStdString();
            user->is_active = query.value(9).toBool();
            users.push_back(std::move(user));
        }
    }
    return users;
}

std::unique_ptr<User> UserRepository::find_by_username(const std::string& username) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE username = ?");
    query.addBindValue(QString::fromStdString(username));

    if (query.exec() && query.next()) {
        auto user = std::make_unique<User>();
        user->id = query.value(0).toInt();
        user->username = query.value(1).toString().toStdString();
        user->password_hash = query.value(2).toString().toStdString();
        user->email = query.value(3).toString().toStdString();
        user->role_id = query.value(4).toInt();
        user->department_id = query.value(5).toInt();
        user->first_name = query.value(6).toString().toStdString();
        user->last_name = query.value(7).toString().toStdString();
        user->phone = query.value(8).toString().toStdString();
        user->is_active = query.value(9).toBool();
        return user;
    }
    return nullptr;
}

std::unique_ptr<User> UserRepository::find_by_email(const std::string& email) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE email = ?");
    query.addBindValue(QString::fromStdString(email));

    if (query.exec() && query.next()) {
        auto user = std::make_unique<User>();
        user->id = query.value(0).toInt();
        user->username = query.value(1).toString().toStdString();
        user->password_hash = query.value(2).toString().toStdString();
        user->email = query.value(3).toString().toStdString();
        user->role_id = query.value(4).toInt();
        user->department_id = query.value(5).toInt();
        user->first_name = query.value(6).toString().toStdString();
        user->last_name = query.value(7).toString().toStdString();
        user->phone = query.value(8).toString().toStdString();
        user->is_active = query.value(9).toBool();
        return user;
    }
    return nullptr;
}

bool UserRepository::remove(int id) {
    return deleteById(id);
}

std::vector<std::unique_ptr<User>> UserRepository::find_by_department(int department_id) {
    std::vector<std::unique_ptr<User>> users;
    if (!db_manager_) return users;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE department_id = ?");
    query.addBindValue(department_id);

    if (query.exec()) {
        while (query.next()) {
            auto user = std::make_unique<User>();
            user->id = query.value(0).toInt();
            user->username = query.value(1).toString().toStdString();
            user->password_hash = query.value(2).toString().toStdString();
            user->email = query.value(3).toString().toStdString();
            user->role_id = query.value(4).toInt();
            user->department_id = query.value(5).toInt();
            user->first_name = query.value(6).toString().toStdString();
            user->last_name = query.value(7).toString().toStdString();
            user->phone = query.value(8).toString().toStdString();
            user->is_active = query.value(9).toBool();
            users.push_back(std::move(user));
        }
    }
    return users;
}

std::vector<std::unique_ptr<User>> UserRepository::find_by_role(int role_id) {
    std::vector<std::unique_ptr<User>> users;
    if (!db_manager_) return users;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, username, password_hash, email, role_id, department_id, "
                  "first_name, last_name, phone, is_active FROM users WHERE role_id = ?");
    query.addBindValue(role_id);

    if (query.exec()) {
        while (query.next()) {
            auto user = std::make_unique<User>();
            user->id = query.value(0).toInt();
            user->username = query.value(1).toString().toStdString();
            user->password_hash = query.value(2).toString().toStdString();
            user->email = query.value(3).toString().toStdString();
            user->role_id = query.value(4).toInt();
            user->department_id = query.value(5).toInt();
            user->first_name = query.value(6).toString().toStdString();
            user->last_name = query.value(7).toString().toStdString();
            user->phone = query.value(8).toString().toStdString();
            user->is_active = query.value(9).toBool();
            users.push_back(std::move(user));
        }
    }
    return users;
}