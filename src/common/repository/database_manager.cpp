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

#include <QCryptographicHash>
#include <QVariant>

bool DatabaseManager::initialize() {
    // 启用 SQLite 性能优化
    QString pragma_sql = R"(
        PRAGMA foreign_keys = ON;
        PRAGMA journal_mode = WAL;
        PRAGMA synchronous = NORMAL;
        PRAGMA cache_size = -64000;
        PRAGMA temp_store = MEMORY;
    )";

    if (!execute_query(pragma_sql)) {
        return false;
    }

    QString init_sql = R"(
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
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE
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
            FOREIGN KEY (manager_id) REFERENCES users(id) ON DELETE SET NULL,
            FOREIGN KEY (parent_department_id) REFERENCES departments(id) ON DELETE SET NULL
        );

        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            email TEXT UNIQUE,
            department_id INTEGER,
            first_name TEXT,
            last_name TEXT,
            phone TEXT,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (department_id) REFERENCES departments(id) ON DELETE SET NULL
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
            FOREIGN KEY (assigned_user_id) REFERENCES users(id) ON DELETE SET NULL
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

        -- 基础索引
        CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
        CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
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

        CREATE TABLE IF NOT EXISTS security_levels (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            level_code TEXT NOT NULL UNIQUE,
            level_name TEXT NOT NULL,
            retention_days INTEGER DEFAULT 30,
            timeout_minutes INTEGER DEFAULT 30,
            description TEXT,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_security_levels_code ON security_levels(level_code);
        CREATE INDEX IF NOT EXISTS idx_security_levels_active ON security_levels(is_active);

        CREATE TABLE IF NOT EXISTS serial_number_configs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            config_type INTEGER NOT NULL UNIQUE,
            prefix TEXT NOT NULL DEFAULT '',
            date_format TEXT NOT NULL DEFAULT 'yyyyMMdd',
            sequence_length INTEGER NOT NULL DEFAULT 4,
            current_sequence INTEGER NOT NULL DEFAULT 0,
            reset_period INTEGER NOT NULL DEFAULT 0,
            last_reset_date DATE,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_serial_configs_type ON serial_number_configs(config_type);

        CREATE TABLE IF NOT EXISTS approver_configs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            task_type INTEGER NOT NULL,
            approver_role_id INTEGER NOT NULL,
            min_security_level_id INTEGER NOT NULL,
            max_security_level_id INTEGER NOT NULL,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (approver_role_id) REFERENCES roles(id)
        );

        CREATE INDEX IF NOT EXISTS idx_approver_configs_task_type ON approver_configs(task_type);
        CREATE INDEX IF NOT EXISTS idx_approver_configs_approver_role ON approver_configs(approver_role_id);
        CREATE INDEX IF NOT EXISTS idx_approver_configs_security_level ON approver_configs(min_security_level_id, max_security_level_id);
        CREATE INDEX IF NOT EXISTS idx_approver_configs_is_active ON approver_configs(is_active);

        CREATE TABLE IF NOT EXISTS barcode_configs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            code_type INTEGER NOT NULL UNIQUE,
            format_string TEXT DEFAULT '{task_id}-{date}-{seq}',
            include_task_id BOOLEAN DEFAULT 1,
            include_date BOOLEAN DEFAULT 1,
            include_security_level BOOLEAN DEFAULT 1,
            include_applicant BOOLEAN DEFAULT 1,
            include_serial_number BOOLEAN DEFAULT 1,
            width INTEGER DEFAULT 200,
            height INTEGER DEFAULT 100,
            margin INTEGER DEFAULT 10,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_barcode_configs_code_type ON barcode_configs(code_type);
        CREATE INDEX IF NOT EXISTS idx_barcode_configs_is_active ON barcode_configs(is_active);

        CREATE TABLE IF NOT EXISTS proxy_approvers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_user_id INTEGER NOT NULL,
            proxy_user_id INTEGER NOT NULL,
            min_security_level_id INTEGER DEFAULT 0,
            max_security_level_id INTEGER DEFAULT 9999,
            task_type INTEGER DEFAULT 2,
            start_date DATE,
            end_date DATE,
            enabled INTEGER DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (owner_user_id) REFERENCES users(id),
            FOREIGN KEY (proxy_user_id) REFERENCES users(id),
            FOREIGN KEY (min_security_level_id) REFERENCES security_levels(id),
            FOREIGN KEY (max_security_level_id) REFERENCES security_levels(id)
        );

        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_owner ON proxy_approvers(owner_user_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_proxy ON proxy_approvers(proxy_user_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_min_security ON proxy_approvers(min_security_level_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_max_security ON proxy_approvers(max_security_level_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_task_type ON proxy_approvers(task_type);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_enabled ON proxy_approvers(enabled);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_dates ON proxy_approvers(start_date, end_date);

        -- 性能优化：复合索引（用于多条件查询）
        CREATE INDEX IF NOT EXISTS idx_tasks_user_status ON tasks(user_id, status);
        CREATE INDEX IF NOT EXISTS idx_tasks_user_type ON tasks(user_id, type);
        CREATE INDEX IF NOT EXISTS idx_tasks_type_status ON tasks(type, status);
        CREATE INDEX IF NOT EXISTS idx_tasks_created_at ON tasks(created_at DESC);

        -- 审计日志表（用于导出和查询优化）
        CREATE TABLE IF NOT EXISTS audit_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            action TEXT NOT NULL,
            resource TEXT,
            details TEXT,
            ip_address TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id)
        );

        CREATE INDEX IF NOT EXISTS idx_audit_logs_user ON audit_logs(user_id);
        CREATE INDEX IF NOT EXISTS idx_audit_logs_action ON audit_logs(action);
        CREATE INDEX IF NOT EXISTS idx_audit_logs_timestamp ON audit_logs(timestamp DESC);

        -- 安全策略表
        CREATE TABLE IF NOT EXISTS security_policies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            max_login_attempts INTEGER DEFAULT 5,
            lock_duration_minutes INTEGER DEFAULT 10,
            password_complexity TEXT,
            password_min_length INTEGER DEFAULT 10,
            password_expiry_days INTEGER DEFAULT 7,
            session_timeout_minutes INTEGER DEFAULT 5,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        -- 刻录任务文件关联表
        CREATE TABLE IF NOT EXISTS burn_task_files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            task_id INTEGER NOT NULL,
            file_path TEXT NOT NULL,
            file_size TEXT,
            file_type TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (task_id) REFERENCES tasks(id)
        );

        CREATE INDEX IF NOT EXISTS idx_burn_task_files_task ON burn_task_files(task_id);

        -- 登录日志表
        CREATE TABLE IF NOT EXISTS login_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            username TEXT,
            dept_id INTEGER,
            user_roles TEXT,
            ip TEXT,
            os TEXT,
            action TEXT NOT NULL,
            success INTEGER DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_login_logs_user ON login_logs(user_id);
        CREATE INDEX IF NOT EXISTS idx_login_logs_username ON login_logs(username);
        CREATE INDEX IF NOT EXISTS idx_login_logs_created_at ON login_logs(created_at DESC);

        -- 操作日志表
        CREATE TABLE IF NOT EXISTS operation_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            module TEXT NOT NULL,
            user_id INTEGER,
            username TEXT,
            dept_id INTEGER,
            user_roles TEXT,
            ip TEXT,
            os TEXT,
            operation TEXT NOT NULL,
            success INTEGER DEFAULT 1,
            details TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_operation_logs_user ON operation_logs(user_id);
        CREATE INDEX IF NOT EXISTS idx_operation_logs_module ON operation_logs(module);
        CREATE INDEX IF NOT EXISTS idx_operation_logs_created_at ON operation_logs(created_at DESC);

        -- 设备授权表
        CREATE TABLE IF NOT EXISTS device_auth (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id INTEGER NOT NULL,
            user_id INTEGER,
            department_id INTEGER,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (device_id) REFERENCES devices(id),
            FOREIGN KEY (user_id) REFERENCES users(id),
            FOREIGN KEY (department_id) REFERENCES departments(id)
        );

        CREATE INDEX IF NOT EXISTS idx_device_auth_device ON device_auth(device_id);
        CREATE INDEX IF NOT EXISTS idx_device_auth_user ON device_auth(user_id);
        CREATE INDEX IF NOT EXISTS idx_device_auth_dept ON device_auth(department_id);

        -- 审批员用户关联表
        CREATE TABLE IF NOT EXISTS approver_users (
            config_id INTEGER NOT NULL,
            user_id INTEGER NOT NULL,
            PRIMARY KEY (config_id, user_id),
            FOREIGN KEY (config_id) REFERENCES approver_configs(id),
            FOREIGN KEY (user_id) REFERENCES users(id)
        );

        CREATE INDEX IF NOT EXISTS idx_approver_users_config ON approver_users(config_id);
        CREATE INDEX IF NOT EXISTS idx_approver_users_user ON approver_users(user_id);
    )";

    if (!execute_query(init_sql)) {
        return false;
    }

    // 检查是否需要插入默认数据
    QSqlQuery count_query("SELECT COUNT(*) FROM users", db_);
    if (count_query.exec() && count_query.next() && count_query.value(0).toInt() == 0) {
        qDebug() << "Database is empty, populating with default data.";
        return populate_default_data();
    }

    return true;
}

bool DatabaseManager::populate_default_data() {
    begin_transaction();

    // 默认密码 "123456" 的 SHA256 哈希值
    QString default_password_hash = QString(QCryptographicHash::hash(QByteArray("123456"), QCryptographicHash::Sha256).toHex());

    // 1. 创建角色
    QSqlQuery role_query(db_);
    role_query.prepare("INSERT INTO roles (name, description, permissions) VALUES (?, ?, ?)");

    // 系统管理员
    role_query.bindValue(0, "系统管理员");
    role_query.bindValue(1, "负责系统的日常维护和管理");
    role_query.bindValue(2, "manage_users,manage_roles,manage_departments,manage_devices,view_logs");
    if (!role_query.exec()) { rollback_transaction(); return false; }
    int admin_role_id = role_query.lastInsertId().toInt();

    // 安全保密员
    role_query.bindValue(0, "安全保密员");
    role_query.bindValue(1, "负责安全策略的配置和审批");
    role_query.bindValue(2, "manage_security_policies,approve_tasks,manage_approvers");
    if (!role_query.exec()) { rollback_transaction(); return false; }
    int sec_role_id = role_query.lastInsertId().toInt();

    // 安全审计员
    role_query.bindValue(0, "安全审计员");
    role_query.bindValue(1, "负责审计系统日志和操作记录");
    role_query.bindValue(2, "view_audit_logs,export_logs");
    if (!role_query.exec()) { rollback_transaction(); return false; }
    int audit_role_id = role_query.lastInsertId().toInt();

    // 普通用户
    role_query.bindValue(0, "普通用户");
    role_query.bindValue(1, "只能申请打印和刻录任务");
    role_query.bindValue(2, "apply_print_task,apply_burn_task,view_my_tasks");
    if (!role_query.exec()) { rollback_transaction(); return false; }
    int user_role_id = role_query.lastInsertId().toInt();


    // 2. 创建用户
    QSqlQuery user_query(db_);
    user_query.prepare("INSERT INTO users (username, password_hash, email) VALUES (?, ?, ?)");

    // admin
    user_query.bindValue(0, "admin");
    user_query.bindValue(1, default_password_hash);
    user_query.bindValue(2, "admin@example.com");
    if (!user_query.exec()) { rollback_transaction(); return false; }
    int admin_user_id = user_query.lastInsertId().toInt();

    // secadmin
    user_query.bindValue(0, "secadmin");
    user_query.bindValue(1, default_password_hash);
    user_query.bindValue(2, "secadmin@example.com");
    if (!user_query.exec()) { rollback_transaction(); return false; }
    int sec_user_id = user_query.lastInsertId().toInt();

    // auditor
    user_query.bindValue(0, "auditor");
    user_query.bindValue(1, default_password_hash);
    user_query.bindValue(2, "auditor@example.com");
    if (!user_query.exec()) { rollback_transaction(); return false; }
    int audit_user_id = user_query.lastInsertId().toInt();

    // testuser
    user_query.bindValue(0, "testuser");
    user_query.bindValue(1, default_password_hash);
    user_query.bindValue(2, "testuser@example.com");
    if (!user_query.exec()) { rollback_transaction(); return false; }
    int test_user_id = user_query.lastInsertId().toInt();

    // 3. 关联用户和角色
    QSqlQuery user_role_query(db_);
    user_role_query.prepare("INSERT INTO user_roles (user_id, role_id) VALUES (?, ?)");

    user_role_query.bindValue(0, admin_user_id);
    user_role_query.bindValue(1, admin_role_id);
    if (!user_role_query.exec()) { rollback_transaction(); return false; }

    user_role_query.bindValue(0, sec_user_id);
    user_role_query.bindValue(1, sec_role_id);
    if (!user_role_query.exec()) { rollback_transaction(); return false; }

    user_role_query.bindValue(0, audit_user_id);
    user_role_query.bindValue(1, audit_role_id);
    if (!user_role_query.exec()) { rollback_transaction(); return false; }

    user_role_query.bindValue(0, test_user_id);
    user_role_query.bindValue(1, user_role_id);
    if (!user_role_query.exec()) { rollback_transaction(); return false; }

    return commit_transaction();
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