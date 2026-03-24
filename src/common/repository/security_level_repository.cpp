#include "security_level_repository.h"
#include <QVariant>

// Static member initialization
DatabaseManager* SecurityLevelRepository::default_db_manager_ = nullptr;

SecurityLevelRepository::SecurityLevelRepository() : db_manager_(default_db_manager_) {}

SecurityLevelRepository::SecurityLevelRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool SecurityLevelRepository::createTable()
{
    if (!db_manager_) return false;

    QString sql = R"(
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
    )";

    return db_manager_->execute_query(sql);
}

bool SecurityLevelRepository::create(SecurityLevel& level)
{
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO security_levels (level_code, level_name, retention_days, timeout_minutes, description, is_active) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(level.level_code);
    query.addBindValue(level.level_name);
    query.addBindValue(level.retention_days);
    query.addBindValue(level.timeout_minutes);
    query.addBindValue(level.description);
    query.addBindValue(level.is_active);

    if (query.exec()) {
        level.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool SecurityLevelRepository::update(const SecurityLevel& level)
{
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE security_levels SET level_code = ?, level_name = ?, retention_days = ?, "
                  "timeout_minutes = ?, description = ?, is_active = ?, updated_at = CURRENT_TIMESTAMP "
                  "WHERE id = ?");
    query.addBindValue(level.level_code);
    query.addBindValue(level.level_name);
    query.addBindValue(level.retention_days);
    query.addBindValue(level.timeout_minutes);
    query.addBindValue(level.description);
    query.addBindValue(level.is_active);
    query.addBindValue(level.id);

    return query.exec();
}

bool SecurityLevelRepository::remove(int id)
{
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM security_levels WHERE id = ?");
    query.addBindValue(id);

    return query.exec();
}

SecurityLevel SecurityLevelRepository::findById(int id)
{
    SecurityLevel level;
    if (!db_manager_) return level;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, level_code, level_name, retention_days, timeout_minutes, "
                  "description, is_active, created_at, updated_at FROM security_levels WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        level.id = query.value(0).toInt();
        level.level_code = query.value(1).toString();
        level.level_name = query.value(2).toString();
        level.retention_days = query.value(3).toInt();
        level.timeout_minutes = query.value(4).toInt();
        level.description = query.value(5).toString();
        level.is_active = query.value(6).toBool();
        level.created_at = query.value(7).toDateTime();
        level.updated_at = query.value(8).toDateTime();
    }
    return level;
}

SecurityLevel SecurityLevelRepository::findByLevelCode(const QString& levelCode)
{
    SecurityLevel level;
    if (!db_manager_) return level;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, level_code, level_name, retention_days, timeout_minutes, "
                  "description, is_active, created_at, updated_at FROM security_levels WHERE level_code = ?");
    query.addBindValue(levelCode);

    if (query.exec() && query.next()) {
        level.id = query.value(0).toInt();
        level.level_code = query.value(1).toString();
        level.level_name = query.value(2).toString();
        level.retention_days = query.value(3).toInt();
        level.timeout_minutes = query.value(4).toInt();
        level.description = query.value(5).toString();
        level.is_active = query.value(6).toBool();
        level.created_at = query.value(7).toDateTime();
        level.updated_at = query.value(8).toDateTime();
    }
    return level;
}

QList<SecurityLevel> SecurityLevelRepository::findAll()
{
    QList<SecurityLevel> levels;
    if (!db_manager_) return levels;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, level_code, level_name, retention_days, timeout_minutes, "
                   "description, is_active, created_at, updated_at FROM security_levels ORDER BY level_code")) {
        while (query.next()) {
            SecurityLevel level;
            level.id = query.value(0).toInt();
            level.level_code = query.value(1).toString();
            level.level_name = query.value(2).toString();
            level.retention_days = query.value(3).toInt();
            level.timeout_minutes = query.value(4).toInt();
            level.description = query.value(5).toString();
            level.is_active = query.value(6).toBool();
            level.created_at = query.value(7).toDateTime();
            level.updated_at = query.value(8).toDateTime();
            levels.append(level);
        }
    }
    return levels;
}

QList<SecurityLevel> SecurityLevelRepository::findActive()
{
    QList<SecurityLevel> levels;
    if (!db_manager_) return levels;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, level_code, level_name, retention_days, timeout_minutes, "
                  "description, is_active, created_at, updated_at FROM security_levels WHERE is_active = 1 ORDER BY level_code");

    if (query.exec()) {
        while (query.next()) {
            SecurityLevel level;
            level.id = query.value(0).toInt();
            level.level_code = query.value(1).toString();
            level.level_name = query.value(2).toString();
            level.retention_days = query.value(3).toInt();
            level.timeout_minutes = query.value(4).toInt();
            level.description = query.value(5).toString();
            level.is_active = query.value(6).toBool();
            level.created_at = query.value(7).toDateTime();
            level.updated_at = query.value(8).toDateTime();
            levels.append(level);
        }
    }
    return levels;
}
