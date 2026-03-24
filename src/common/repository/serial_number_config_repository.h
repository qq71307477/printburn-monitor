#ifndef SERIAL_NUMBER_CONFIG_REPOSITORY_H
#define SERIAL_NUMBER_CONFIG_REPOSITORY_H

#include "serial_number_config_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDebug>
#include <optional>

class SerialNumberConfigRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    SerialNumberConfigRepository();
    explicit SerialNumberConfigRepository(DatabaseManager* db_manager);
    ~SerialNumberConfigRepository() = default;

    static void setDefaultDbManager(DatabaseManager* db_manager) { default_db_manager_ = db_manager; }

    bool create_table();

    // CRUD Operations
    bool create(SerialNumberConfig& config);
    bool update(const SerialNumberConfig& config);
    bool remove(int id);

    // Query Operations
    SerialNumberConfig findById(int id);
    SerialNumberConfig findByType(int configType);
    QList<SerialNumberConfig> findAll();

    // Check if config exists for a type
    bool existsByType(int configType);

    // Get or create default config for a type
    SerialNumberConfig getOrCreateDefault(int configType);

    // Update current sequence
    bool updateCurrentSequence(int id, int newSequence);

    // Update last reset date
    bool updateLastResetDate(int id, const QDate& date);
};

#endif // SERIAL_NUMBER_CONFIG_REPOSITORY_H
