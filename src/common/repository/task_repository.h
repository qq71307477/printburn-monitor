#ifndef TASK_REPOSITORY_H
#define TASK_REPOSITORY_H

#include "task_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QCache>
#include <vector>
#include <memory>

class TaskRepository;

// 用户名缓存接口（仅声明，实现在 .cpp 中避免重定义）
class UserNameCache {
public:
    static QString getUserName(int userId);
    static void cacheUserName(int userId, const QString& username);
    static void clear();
    static void preloadUserNames();
private:
    UserNameCache() = delete;
};

class TaskRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    TaskRepository();
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

    // Qt-style methods
    Task findById(int id);
    QList<Task> findByUserId(int userId, const QString& taskType = QString(), const QString& status = QString(), int limit = 0, int offset = 0);
    QList<Task> findByType(const QString& taskType, const QString& status = QString(), int limit = 0, int offset = 0);
    QList<Task> findPendingApprovalTasks(const QString& approverRole);

    // 分页查询方法（服务端分页）
    struct PagedResult {
        QList<Task> tasks;
        int totalCount;
        int totalPages;
    };
    PagedResult findByUserIdPaged(int userId, const QString& taskType, const QString& status,
                                   int page, int pageSize, const QString& sortBy = "created_at", bool sortDesc = true);
    PagedResult findPendingApprovalTasksPaged(const QString& approverRole, const QString& searchText,
                                               const QString& taskType, const QString& status,
                                               int page, int pageSize, const QString& sortBy = "created_at", bool sortDesc = true);

    // Serial number support
    bool updateSerialNumber(int taskId, const QString& serialNumber);

    // Count methods for statistics
    int countByUserId(int userId, const QString& taskType = QString(), const QString& status = QString());
    int countPendingApproval(const QString& approverRole = QString());
};

#endif // TASK_REPOSITORY_H