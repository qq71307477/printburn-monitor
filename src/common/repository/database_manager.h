#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <memory>
#include <vector>

class DatabaseManager {
private:
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db_;

public:
    explicit DatabaseManager(const std::string& db_path);

    bool initialize();
    sqlite3* get_connection() const { return db_.get(); }

    // Execute non-query statements (INSERT, UPDATE, DELETE)
    bool execute_query(const std::string& sql);

    // Begin transaction
    bool begin_transaction();

    // Commit transaction
    bool commit_transaction();

    // Rollback transaction
    bool rollback_transaction();
};

#endif // DATABASE_MANAGER_H