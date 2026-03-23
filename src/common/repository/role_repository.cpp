#include "role_repository.h"
#include <QVariant>

// Static member initialization
DatabaseManager* RoleRepository::default_db_manager_ = nullptr;

RoleRepository::RoleRepository() : db_manager_(default_db_manager_) {}

RoleRepository::RoleRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool RoleRepository::create_table() {
    QString sql = R"(
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

// Qt-style methods for services
Role RoleRepository::findById(int id) {
    Role role;
    if (!db_manager_) return role;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, description, permissions, is_active FROM roles WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        role.id = query.value(0).toInt();
        role.name = query.value(1).toString().toStdString();
        role.description = query.value(2).toString().toStdString();
        role.permissions = query.value(3).toString().toStdString();
        role.is_active = query.value(4).toBool();
    }
    return role;
}

QList<Role> RoleRepository::findByUserId(int userId) {
    QList<Role> roles;
    if (!db_manager_) return roles;

    // Query roles assigned to a user through user_roles junction table
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT r.id, r.name, r.description, r.permissions, r.is_active "
                  "FROM roles r "
                  "INNER JOIN user_roles ur ON r.id = ur.role_id "
                  "WHERE ur.user_id = ?");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            Role role;
            role.id = query.value(0).toInt();
            role.name = query.value(1).toString().toStdString();
            role.description = query.value(2).toString().toStdString();
            role.permissions = query.value(3).toString().toStdString();
            role.is_active = query.value(4).toBool();
            roles.append(role);
        }
    }
    return roles;
}

QList<Role> RoleRepository::findAll() {
    QList<Role> roles;
    if (!db_manager_) return roles;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, name, description, permissions, is_active FROM roles")) {
        while (query.next()) {
            Role role;
            role.id = query.value(0).toInt();
            role.name = query.value(1).toString().toStdString();
            role.description = query.value(2).toString().toStdString();
            role.permissions = query.value(3).toString().toStdString();
            role.is_active = query.value(4).toBool();
            roles.append(role);
        }
    }
    return roles;
}

Role RoleRepository::findByName(const QString& name) {
    Role role;
    if (!db_manager_) return role;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, description, permissions, is_active FROM roles WHERE name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        role.id = query.value(0).toInt();
        role.name = query.value(1).toString().toStdString();
        role.description = query.value(2).toString().toStdString();
        role.permissions = query.value(3).toString().toStdString();
        role.is_active = query.value(4).toBool();
    }
    return role;
}

QList<Role> RoleRepository::search(const QString& keyword) {
    QList<Role> roles;
    if (!db_manager_) return roles;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, description, permissions, is_active FROM roles "
                  "WHERE name LIKE ? OR description LIKE ?");
    QString pattern = "%" + keyword + "%";
    query.addBindValue(pattern);
    query.addBindValue(pattern);

    if (query.exec()) {
        while (query.next()) {
            Role role;
            role.id = query.value(0).toInt();
            role.name = query.value(1).toString().toStdString();
            role.description = query.value(2).toString().toStdString();
            role.permissions = query.value(3).toString().toStdString();
            role.is_active = query.value(4).toBool();
            roles.append(role);
        }
    }
    return roles;
}

bool RoleRepository::deleteById(int id) {
    return remove(id);
}

// Legacy snake_case methods for backward compatibility
std::unique_ptr<Role> RoleRepository::find_by_id(int id) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, description, permissions, is_active FROM roles WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        auto role = std::make_unique<Role>();
        role->id = query.value(0).toInt();
        role->name = query.value(1).toString().toStdString();
        role->description = query.value(2).toString().toStdString();
        role->permissions = query.value(3).toString().toStdString();
        role->is_active = query.value(4).toBool();
        return role;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Role>> RoleRepository::find_all() {
    std::vector<std::unique_ptr<Role>> roles;
    if (!db_manager_) return roles;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, name, description, permissions, is_active FROM roles")) {
        while (query.next()) {
            auto role = std::make_unique<Role>();
            role->id = query.value(0).toInt();
            role->name = query.value(1).toString().toStdString();
            role->description = query.value(2).toString().toStdString();
            role->permissions = query.value(3).toString().toStdString();
            role->is_active = query.value(4).toBool();
            roles.push_back(std::move(role));
        }
    }
    return roles;
}

std::unique_ptr<Role> RoleRepository::find_by_name(const std::string& name) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, description, permissions, is_active FROM roles WHERE name = ?");
    query.addBindValue(QString::fromStdString(name));

    if (query.exec() && query.next()) {
        auto role = std::make_unique<Role>();
        role->id = query.value(0).toInt();
        role->name = query.value(1).toString().toStdString();
        role->description = query.value(2).toString().toStdString();
        role->permissions = query.value(3).toString().toStdString();
        role->is_active = query.value(4).toBool();
        return role;
    }
    return nullptr;
}

bool RoleRepository::create(Role& role) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO roles (name, description, permissions, is_active) VALUES (?, ?, ?, ?)");
    query.addBindValue(QString::fromStdString(role.name));
    query.addBindValue(QString::fromStdString(role.description));
    query.addBindValue(QString::fromStdString(role.permissions));
    query.addBindValue(role.is_active);

    if (query.exec()) {
        role.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool RoleRepository::update(const Role& role) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE roles SET name = ?, description = ?, permissions = ?, is_active = ?, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(QString::fromStdString(role.name));
    query.addBindValue(QString::fromStdString(role.description));
    query.addBindValue(QString::fromStdString(role.permissions));
    query.addBindValue(role.is_active);
    query.addBindValue(role.id);

    return query.exec();
}

bool RoleRepository::remove(int id) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM roles WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}