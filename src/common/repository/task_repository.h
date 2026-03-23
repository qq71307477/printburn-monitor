#ifndef TASK_REPOSITORY_H
#define TASK_REPOSITORY_H

#include "models/task_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <vector>
#include <memory>

class TaskRepository {
private:
    DatabaseManager* db_manager_;

public:
    explicit TaskRepository(DatabaseManager* db_manager);

    bool create_table();
    std::unique_ptr<Task> find_by_id(int id);
    std::vector<std::unique_ptr<Task>> find_all();
    bool create(Task& task);
    bool update(const Task& task);
    bool remove(int id);
    std::vector<std::unique_ptr<Task>> find_by_assigned_user(int user_id);
    std::vector<std::unique_ptr<Task>> find_by_created_by_user(int user_id);
    std::vector<std::unique_ptr<Task>> find_by_device(int device_id);
    std::vector<std::unique_ptr<Task>> find_by_status(const std::string& status);
    std::vector<std::unique_ptr<Task>> find_by_priority(const std::string& priority);
};

#endif // TASK_REPOSITORY_H