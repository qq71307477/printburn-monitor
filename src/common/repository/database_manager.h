#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <memory>

class DatabaseManager {
private:
    QSqlDatabase db_;
    bool populate_default_data();

public:
    explicit DatabaseManager(const QString& db_path);
    ~DatabaseManager();

    bool initialize();
    QSqlDatabase get_connection() const { return db_; }

    // Execute non-query statements (INSERT, UPDATE, DELETE)
    bool execute_query(const QString& sql);

    // Begin transaction
    bool begin_transaction();

    // Commit transaction
    bool commit_transaction();

    // Rollback transaction
    bool rollback_transaction();
};

#endif // DATABASE_MANAGER_H