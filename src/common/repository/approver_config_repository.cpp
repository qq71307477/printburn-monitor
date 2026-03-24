#include "approver_config_repository.h"
#include <QVariant>

// Static member initialization
DatabaseManager* ApproverConfigRepository::default_db_manager_ = nullptr;

ApproverConfigRepository::ApproverConfigRepository() : db_manager_(default_db_manager_) {}

ApproverConfigRepository::ApproverConfigRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool ApproverConfigRepository::create_table() {
    if (!db_manager_) return false;

    QString sql = R"(
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
    )";
    return db_manager_->execute_query(sql);
}

// Qt-style methods for services
ApproverConfig ApproverConfigRepository::findById(int id) {
    ApproverConfig config;
    if (!db_manager_) return config;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                  "is_active, created_at, updated_at FROM approver_configs WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        config.id = query.value(0).toInt();
        config.task_type = query.value(1).toInt();
        config.approver_role_id = query.value(2).toInt();
        config.min_security_level_id = query.value(3).toInt();
        config.max_security_level_id = query.value(4).toInt();
        config.is_active = query.value(5).toBool();
        config.created_at = query.value(6).toDateTime();
        config.updated_at = query.value(7).toDateTime();
    }
    return config;
}

QList<ApproverConfig> ApproverConfigRepository::findAll() {
    QList<ApproverConfig> configs;
    if (!db_manager_) return configs;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                   "is_active, created_at, updated_at FROM approver_configs ORDER BY id")) {
        while (query.next()) {
            ApproverConfig config;
            config.id = query.value(0).toInt();
            config.task_type = query.value(1).toInt();
            config.approver_role_id = query.value(2).toInt();
            config.min_security_level_id = query.value(3).toInt();
            config.max_security_level_id = query.value(4).toInt();
            config.is_active = query.value(5).toBool();
            config.created_at = query.value(6).toDateTime();
            config.updated_at = query.value(7).toDateTime();
            configs.append(config);
        }
    }
    return configs;
}

bool ApproverConfigRepository::create(ApproverConfig& config) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO approver_configs (task_type, approver_role_id, min_security_level_id, "
                  "max_security_level_id, is_active) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(config.task_type);
    query.addBindValue(config.approver_role_id);
    query.addBindValue(config.min_security_level_id);
    query.addBindValue(config.max_security_level_id);
    query.addBindValue(config.is_active);

    if (query.exec()) {
        config.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool ApproverConfigRepository::update(const ApproverConfig& config) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE approver_configs SET task_type = ?, approver_role_id = ?, "
                  "min_security_level_id = ?, max_security_level_id = ?, is_active = ?, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(config.task_type);
    query.addBindValue(config.approver_role_id);
    query.addBindValue(config.min_security_level_id);
    query.addBindValue(config.max_security_level_id);
    query.addBindValue(config.is_active);
    query.addBindValue(config.id);

    return query.exec();
}

bool ApproverConfigRepository::deleteById(int id) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM approver_configs WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

QList<ApproverConfig> ApproverConfigRepository::findByTaskType(int taskType) {
    QList<ApproverConfig> configs;
    if (!db_manager_) return configs;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                  "is_active, created_at, updated_at FROM approver_configs WHERE task_type = ? ORDER BY id");
    query.addBindValue(taskType);

    if (query.exec()) {
        while (query.next()) {
            ApproverConfig config;
            config.id = query.value(0).toInt();
            config.task_type = query.value(1).toInt();
            config.approver_role_id = query.value(2).toInt();
            config.min_security_level_id = query.value(3).toInt();
            config.max_security_level_id = query.value(4).toInt();
            config.is_active = query.value(5).toBool();
            config.created_at = query.value(6).toDateTime();
            config.updated_at = query.value(7).toDateTime();
            configs.append(config);
        }
    }
    return configs;
}

QList<ApproverConfig> ApproverConfigRepository::findBySecurityLevel(int taskType, int securityLevelId) {
    QList<ApproverConfig> configs;
    if (!db_manager_) return configs;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                  "is_active, created_at, updated_at FROM approver_configs "
                  "WHERE task_type = ? AND min_security_level_id <= ? AND max_security_level_id >= ? AND is_active = 1 "
                  "ORDER BY id");
    query.addBindValue(taskType);
    query.addBindValue(securityLevelId);
    query.addBindValue(securityLevelId);

    if (query.exec()) {
        while (query.next()) {
            ApproverConfig config;
            config.id = query.value(0).toInt();
            config.task_type = query.value(1).toInt();
            config.approver_role_id = query.value(2).toInt();
            config.min_security_level_id = query.value(3).toInt();
            config.max_security_level_id = query.value(4).toInt();
            config.is_active = query.value(5).toBool();
            config.created_at = query.value(6).toDateTime();
            config.updated_at = query.value(7).toDateTime();
            configs.append(config);
        }
    }
    return configs;
}

QList<ApproverConfig> ApproverConfigRepository::findByApproverRoleId(int approverRoleId) {
    QList<ApproverConfig> configs;
    if (!db_manager_) return configs;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                  "is_active, created_at, updated_at FROM approver_configs WHERE approver_role_id = ? ORDER BY id");
    query.addBindValue(approverRoleId);

    if (query.exec()) {
        while (query.next()) {
            ApproverConfig config;
            config.id = query.value(0).toInt();
            config.task_type = query.value(1).toInt();
            config.approver_role_id = query.value(2).toInt();
            config.min_security_level_id = query.value(3).toInt();
            config.max_security_level_id = query.value(4).toInt();
            config.is_active = query.value(5).toBool();
            config.created_at = query.value(6).toDateTime();
            config.updated_at = query.value(7).toDateTime();
            configs.append(config);
        }
    }
    return configs;
}

QList<ApproverConfig> ApproverConfigRepository::findActive() {
    QList<ApproverConfig> configs;
    if (!db_manager_) return configs;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                   "is_active, created_at, updated_at FROM approver_configs WHERE is_active = 1 ORDER BY id")) {
        while (query.next()) {
            ApproverConfig config;
            config.id = query.value(0).toInt();
            config.task_type = query.value(1).toInt();
            config.approver_role_id = query.value(2).toInt();
            config.min_security_level_id = query.value(3).toInt();
            config.max_security_level_id = query.value(4).toInt();
            config.is_active = query.value(5).toBool();
            config.created_at = query.value(6).toDateTime();
            config.updated_at = query.value(7).toDateTime();
            configs.append(config);
        }
    }
    return configs;
}

bool ApproverConfigRepository::hasOverlap(const ApproverConfig& config, int excludeId) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT COUNT(*) FROM approver_configs "
                  "WHERE task_type = ? AND approver_role_id = ? "
                  "AND min_security_level_id <= ? AND max_security_level_id >= ? "
                  "AND id != ?");
    query.addBindValue(config.task_type);
    query.addBindValue(config.approver_role_id);
    query.addBindValue(config.max_security_level_id);
    query.addBindValue(config.min_security_level_id);
    query.addBindValue(excludeId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// Legacy snake_case methods for backward compatibility
std::unique_ptr<ApproverConfig> ApproverConfigRepository::find_by_id(int id) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                  "is_active, created_at, updated_at FROM approver_configs WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        auto config = std::make_unique<ApproverConfig>();
        config->id = query.value(0).toInt();
        config->task_type = query.value(1).toInt();
        config->approver_role_id = query.value(2).toInt();
        config->min_security_level_id = query.value(3).toInt();
        config->max_security_level_id = query.value(4).toInt();
        config->is_active = query.value(5).toBool();
        config->created_at = query.value(6).toDateTime();
        config->updated_at = query.value(7).toDateTime();
        return config;
    }
    return nullptr;
}

std::vector<std::unique_ptr<ApproverConfig>> ApproverConfigRepository::find_all() {
    std::vector<std::unique_ptr<ApproverConfig>> configs;
    if (!db_manager_) return configs;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, task_type, approver_role_id, min_security_level_id, max_security_level_id, "
                   "is_active, created_at, updated_at FROM approver_configs ORDER BY id")) {
        while (query.next()) {
            auto config = std::make_unique<ApproverConfig>();
            config->id = query.value(0).toInt();
            config->task_type = query.value(1).toInt();
            config->approver_role_id = query.value(2).toInt();
            config->min_security_level_id = query.value(3).toInt();
            config->max_security_level_id = query.value(4).toInt();
            config->is_active = query.value(5).toBool();
            config->created_at = query.value(6).toDateTime();
            config->updated_at = query.value(7).toDateTime();
            configs.push_back(std::move(config));
        }
    }
    return configs;
}
