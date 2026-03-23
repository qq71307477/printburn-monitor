#include "user_repository.h"

UserRepository::UserRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool UserRepository::create_table() {
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

std::unique_ptr<User> UserRepository::find_by_id(int id) {
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

bool UserRepository::create(User& user) {
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

bool UserRepository::remove(int id) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM users WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

std::vector<std::unique_ptr<User>> UserRepository::find_by_department(int department_id) {
    std::vector<std::unique_ptr<User>> users;
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