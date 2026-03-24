#include "serial_number_config_repository.h"

// Static member initialization
DatabaseManager* SerialNumberConfigRepository::default_db_manager_ = nullptr;

SerialNumberConfigRepository::SerialNumberConfigRepository() : db_manager_(default_db_manager_) {}

SerialNumberConfigRepository::SerialNumberConfigRepository(DatabaseManager* db_manager)
    : db_manager_(db_manager) {}

bool SerialNumberConfigRepository::create_table() {
    if (!db_manager_) return false;

    QString sql = R"(
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
    )";

    return db_manager_->execute_query(sql);
}

bool SerialNumberConfigRepository::create(SerialNumberConfig& config) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO serial_number_configs "
                  "(config_type, prefix, date_format, sequence_length, current_sequence, "
                  "reset_period, last_reset_date, is_active) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(config.getConfigType());
    query.addBindValue(config.getPrefix());
    query.addBindValue(config.getDateFormat());
    query.addBindValue(config.getSequenceLength());
    query.addBindValue(config.getCurrentSequence());
    query.addBindValue(config.getResetPeriod());
    query.addBindValue(config.getLastResetDate());
    query.addBindValue(config.getIsActive());

    if (query.exec()) {
        config.setId(query.lastInsertId().toInt());
        return true;
    }
    qDebug() << "Failed to create serial config:" << query.lastError().text();
    return false;
}

bool SerialNumberConfigRepository::update(const SerialNumberConfig& config) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE serial_number_configs SET "
                  "config_type = ?, prefix = ?, date_format = ?, sequence_length = ?, "
                  "current_sequence = ?, reset_period = ?, last_reset_date = ?, "
                  "is_active = ?, updated_at = CURRENT_TIMESTAMP "
                  "WHERE id = ?");
    query.addBindValue(config.getConfigType());
    query.addBindValue(config.getPrefix());
    query.addBindValue(config.getDateFormat());
    query.addBindValue(config.getSequenceLength());
    query.addBindValue(config.getCurrentSequence());
    query.addBindValue(config.getResetPeriod());
    query.addBindValue(config.getLastResetDate());
    query.addBindValue(config.getIsActive());
    query.addBindValue(config.getId());

    if (!query.exec()) {
        qDebug() << "Failed to update serial config:" << query.lastError().text();
        return false;
    }
    return true;
}

bool SerialNumberConfigRepository::remove(int id) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM serial_number_configs WHERE id = ?");
    query.addBindValue(id);

    return query.exec();
}

SerialNumberConfig SerialNumberConfigRepository::findById(int id) {
    SerialNumberConfig config;
    if (!db_manager_) return config;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, config_type, prefix, date_format, sequence_length, "
                  "current_sequence, reset_period, last_reset_date, is_active, "
                  "created_at, updated_at FROM serial_number_configs WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        config.setId(query.value(0).toInt());
        config.setConfigType(query.value(1).toInt());
        config.setPrefix(query.value(2).toString());
        config.setDateFormat(query.value(3).toString());
        config.setSequenceLength(query.value(4).toInt());
        config.setCurrentSequence(query.value(5).toInt());
        config.setResetPeriod(query.value(6).toInt());
        config.setLastResetDate(query.value(7).toDate());
        config.setIsActive(query.value(8).toBool());
        config.setCreatedAt(query.value(9).toDateTime());
        config.setUpdatedAt(query.value(10).toDateTime());
    }
    return config;
}

SerialNumberConfig SerialNumberConfigRepository::findByType(int configType) {
    SerialNumberConfig config;
    if (!db_manager_) return config;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, config_type, prefix, date_format, sequence_length, "
                  "current_sequence, reset_period, last_reset_date, is_active, "
                  "created_at, updated_at FROM serial_number_configs WHERE config_type = ?");
    query.addBindValue(configType);

    if (query.exec() && query.next()) {
        config.setId(query.value(0).toInt());
        config.setConfigType(query.value(1).toInt());
        config.setPrefix(query.value(2).toString());
        config.setDateFormat(query.value(3).toString());
        config.setSequenceLength(query.value(4).toInt());
        config.setCurrentSequence(query.value(5).toInt());
        config.setResetPeriod(query.value(6).toInt());
        config.setLastResetDate(query.value(7).toDate());
        config.setIsActive(query.value(8).toBool());
        config.setCreatedAt(query.value(9).toDateTime());
        config.setUpdatedAt(query.value(10).toDateTime());
    }
    return config;
}

QList<SerialNumberConfig> SerialNumberConfigRepository::findAll() {
    QList<SerialNumberConfig> configs;
    if (!db_manager_) return configs;

    QSqlQuery query(db_manager_->get_connection());
    if (query.exec("SELECT id, config_type, prefix, date_format, sequence_length, "
                   "current_sequence, reset_period, last_reset_date, is_active, "
                   "created_at, updated_at FROM serial_number_configs ORDER BY config_type")) {
        while (query.next()) {
            SerialNumberConfig config;
            config.setId(query.value(0).toInt());
            config.setConfigType(query.value(1).toInt());
            config.setPrefix(query.value(2).toString());
            config.setDateFormat(query.value(3).toString());
            config.setSequenceLength(query.value(4).toInt());
            config.setCurrentSequence(query.value(5).toInt());
            config.setResetPeriod(query.value(6).toInt());
            config.setLastResetDate(query.value(7).toDate());
            config.setIsActive(query.value(8).toBool());
            config.setCreatedAt(query.value(9).toDateTime());
            config.setUpdatedAt(query.value(10).toDateTime());
            configs.append(config);
        }
    }
    return configs;
}

bool SerialNumberConfigRepository::existsByType(int configType) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT COUNT(*) FROM serial_number_configs WHERE config_type = ?");
    query.addBindValue(configType);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

SerialNumberConfig SerialNumberConfigRepository::getOrCreateDefault(int configType) {
    SerialNumberConfig config = findByType(configType);

    // If not found, create default config
    if (config.getId() == 0) {
        config.setConfigType(configType);
        config.setPrefix(configType == SerialNumberConfigType::PRINT ? "PRT-" : "BRN-");
        config.setDateFormat("yyyyMMdd");
        config.setSequenceLength(4);
        config.setCurrentSequence(0);
        config.setResetPeriod(ResetPeriod::DAILY);
        config.setLastResetDate(QDate::currentDate());
        config.setIsActive(true);

        create(config);
    }

    return config;
}

bool SerialNumberConfigRepository::updateCurrentSequence(int id, int newSequence) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE serial_number_configs SET current_sequence = ?, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(newSequence);
    query.addBindValue(id);

    return query.exec();
}

bool SerialNumberConfigRepository::updateLastResetDate(int id, const QDate& date) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE serial_number_configs SET last_reset_date = ?, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(date);
    query.addBindValue(id);

    return query.exec();
}
