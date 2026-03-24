#ifndef SECURITY_LEVEL_REPOSITORY_H
#define SECURITY_LEVEL_REPOSITORY_H

#include "security_level_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDebug>

class SecurityLevelRepository
{
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    SecurityLevelRepository();
    explicit SecurityLevelRepository(DatabaseManager* db_manager);
    ~SecurityLevelRepository() = default;

    bool createTable();

    // CRUD operations
    bool create(SecurityLevel& level);
    bool update(const SecurityLevel& level);
    bool remove(int id);
    SecurityLevel findById(int id);
    SecurityLevel findByLevelCode(const QString& levelCode);
    QList<SecurityLevel> findAll();
    QList<SecurityLevel> findActive();

    static void setDefaultDatabaseManager(DatabaseManager* db_manager) {
        default_db_manager_ = db_manager;
    }
};

#endif // SECURITY_LEVEL_REPOSITORY_H
