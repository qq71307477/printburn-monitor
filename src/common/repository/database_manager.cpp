#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

DatabaseManager::DatabaseManager(const QString& db_path) {
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(db_path);

    if (!db_.open()) {
        qCritical() << "Cannot open database:" << db_.lastError().text();
    }
}

DatabaseManager::~DatabaseManager() {
    if (db_.isOpen()) {
        db_.close();
    }
}

bool DatabaseManager::initialize() {
    QString init_sql = R"(
        PRAGMA foreign_keys = ON;

        CREATE TABLE IF NOT EXISTS roles (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT,
            permissions TEXT,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS user_roles (
            user_id INTEGER NOT NULL,
            role_id INTEGER NOT NULL,
            PRIMARY KEY (user_id, role_id),
            FOREIGN KEY (user_id) REFERENCES users(id),
            FOREIGN KEY (role_id) REFERENCES roles(id)
        );

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

        CREATE TABLE IF NOT EXISTS devices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            device_id TEXT NOT NULL UNIQUE,
            device_type TEXT,
            ip_address TEXT,
            location TEXT,
            assigned_user_id INTEGER,
            status TEXT DEFAULT 'available',
            specifications TEXT,
            is_monitored BOOLEAN DEFAULT 1,
            last_seen DATETIME,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (assigned_user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT,
            assigned_user_id INTEGER NOT NULL,
            created_by_user_id INTEGER NOT NULL,
            device_id INTEGER,
            status TEXT DEFAULT 'pending',
            priority TEXT DEFAULT 'medium',
            due_date DATETIME,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            completed_at DATETIME,
            FOREIGN KEY (assigned_user_id) REFERENCES users(id),
            FOREIGN KEY (created_by_user_id) REFERENCES users(id),
            FOREIGN KEY (device_id) REFERENCES devices(id)
        );

        CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
        CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
        CREATE INDEX IF NOT EXISTS idx_users_role_id ON users(role_id);
        CREATE INDEX IF NOT EXISTS idx_users_department_id ON users(department_id);
        CREATE INDEX IF NOT EXISTS idx_devices_device_id ON devices(device_id);
        CREATE INDEX IF NOT EXISTS idx_devices_assigned_user ON devices(assigned_user_id);
        CREATE INDEX IF NOT EXISTS idx_devices_status ON devices(status);
        CREATE INDEX IF NOT EXISTS idx_tasks_assigned_user ON tasks(assigned_user_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_created_by ON tasks(created_by_user_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_device_id ON tasks(device_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status);
        CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);
        CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date);
    )";

    return execute_query(init_sql);
}

bool DatabaseManager::execute_query(const QString& sql) {
    QSqlQuery query(db_);
    if (!query.exec(sql)) {
        qCritical() << "SQL error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::begin_transaction() {
    return db_.transaction();
}

bool DatabaseManager::commit_transaction() {
    return db_.commit();
}

bool DatabaseManager::rollback_transaction() {
    return db_.rollback();
}