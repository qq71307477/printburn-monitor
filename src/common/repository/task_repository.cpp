#include "task_repository.h"

TaskRepository::TaskRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool TaskRepository::create_table() {
    const char* sql = R"(
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

        CREATE INDEX IF NOT EXISTS idx_tasks_assigned_user ON tasks(assigned_user_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_created_by ON tasks(created_by_user_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_device_id ON tasks(device_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status);
        CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);
    )";

    return db_manager_->execute_query(sql);
}

std::unique_ptr<Task> TaskRepository::find_by_id(int id) {
    const char* sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date, created_at, updated_at, completed_at FROM tasks WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::unique_ptr<Task> task = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        task = std::make_unique<Task>();
        task->id = sqlite3_column_int(stmt, 0);
        task->title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task->assigned_user_id = sqlite3_column_int(stmt, 3);
        task->created_by_user_id = sqlite3_column_int(stmt, 4);
        task->device_id = sqlite3_column_int(stmt, 5);
        task->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task->priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        // Handle datetime columns
        const char* due_date_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* completed_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    }

    sqlite3_finalize(stmt);
    return task;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_all() {
    const char* sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date, created_at, updated_at, completed_at FROM tasks";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    std::vector<std::unique_ptr<Task>> tasks;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto task = std::make_unique<Task>();
        task->id = sqlite3_column_int(stmt, 0);
        task->title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task->assigned_user_id = sqlite3_column_int(stmt, 3);
        task->created_by_user_id = sqlite3_column_int(stmt, 4);
        task->device_id = sqlite3_column_int(stmt, 5);
        task->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task->priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        // Handle datetime columns
        const char* due_date_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* completed_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        tasks.push_back(std::move(task));
    }

    sqlite3_finalize(stmt);
    return tasks;
}

bool TaskRepository::create(Task& task) {
    const char* sql = "INSERT INTO tasks (title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, task.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, task.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, task.assigned_user_id);
    sqlite3_bind_int(stmt, 4, task.created_by_user_id);
    sqlite3_bind_int(stmt, 5, task.device_id);
    sqlite3_bind_text(stmt, 6, task.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, task.priority.c_str(), -1, SQLITE_STATIC);

    if (task.due_date != 0) {
        // Convert time_t to string and bind
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&task.due_date));
        sqlite3_bind_text(stmt, 8, buffer, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 8);
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        task.id = sqlite3_last_insert_rowid(db_manager_->get_connection());
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

bool TaskRepository::update(const Task& task) {
    const char* sql = "UPDATE tasks SET title = ?, description = ?, assigned_user_id = ?, created_by_user_id = ?, "
                      "device_id = ?, status = ?, priority = ?, due_date = ?, completed_at = ?, "
                      "updated_at = CURRENT_TIMESTAMP WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, task.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, task.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, task.assigned_user_id);
    sqlite3_bind_int(stmt, 4, task.created_by_user_id);
    sqlite3_bind_int(stmt, 5, task.device_id);
    sqlite3_bind_text(stmt, 6, task.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, task.priority.c_str(), -1, SQLITE_STATIC);

    if (task.due_date != 0) {
        // Convert time_t to string and bind
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&task.due_date));
        sqlite3_bind_text(stmt, 8, buffer, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 8);
    }

    if (task.completed_at != 0) {
        // Convert time_t to string and bind
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&task.completed_at));
        sqlite3_bind_text(stmt, 9, buffer, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 9);
    }

    sqlite3_bind_int(stmt, 10, task.id);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

bool TaskRepository::remove(int id) {
    const char* sql = "DELETE FROM tasks WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE && sqlite3_changes(db_manager_->get_connection()) > 0);

    sqlite3_finalize(stmt);
    return result;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_assigned_user(int user_id) {
    const char* sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date, created_at, updated_at, completed_at FROM tasks WHERE assigned_user_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_int(stmt, 1, user_id);

    std::vector<std::unique_ptr<Task>> tasks;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto task = std::make_unique<Task>();
        task->id = sqlite3_column_int(stmt, 0);
        task->title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task->assigned_user_id = sqlite3_column_int(stmt, 3);
        task->created_by_user_id = sqlite3_column_int(stmt, 4);
        task->device_id = sqlite3_column_int(stmt, 5);
        task->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task->priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        // Handle datetime columns
        const char* due_date_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* completed_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        tasks.push_back(std::move(task));
    }

    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_created_by_user(int user_id) {
    const char* sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date, created_at, updated_at, completed_at FROM tasks WHERE created_by_user_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_int(stmt, 1, user_id);

    std::vector<std::unique_ptr<Task>> tasks;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto task = std::make_unique<Task>();
        task->id = sqlite3_column_int(stmt, 0);
        task->title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task->assigned_user_id = sqlite3_column_int(stmt, 3);
        task->created_by_user_id = sqlite3_column_int(stmt, 4);
        task->device_id = sqlite3_column_int(stmt, 5);
        task->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task->priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        // Handle datetime columns
        const char* due_date_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* completed_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        tasks.push_back(std::move(task));
    }

    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_device(int device_id) {
    const char* sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date, created_at, updated_at, completed_at FROM tasks WHERE device_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_int(stmt, 1, device_id);

    std::vector<std::unique_ptr<Task>> tasks;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto task = std::make_unique<Task>();
        task->id = sqlite3_column_int(stmt, 0);
        task->title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task->assigned_user_id = sqlite3_column_int(stmt, 3);
        task->created_by_user_id = sqlite3_column_int(stmt, 4);
        task->device_id = sqlite3_column_int(stmt, 5);
        task->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task->priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        // Handle datetime columns
        const char* due_date_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* completed_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        tasks.push_back(std::move(task));
    }

    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_status(const std::string& status) {
    const char* sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date, created_at, updated_at, completed_at FROM tasks WHERE status = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);

    std::vector<std::unique_ptr<Task>> tasks;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto task = std::make_unique<Task>();
        task->id = sqlite3_column_int(stmt, 0);
        task->title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task->assigned_user_id = sqlite3_column_int(stmt, 3);
        task->created_by_user_id = sqlite3_column_int(stmt, 4);
        task->device_id = sqlite3_column_int(stmt, 5);
        task->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task->priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        // Handle datetime columns
        const char* due_date_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* completed_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        tasks.push_back(std::move(task));
    }

    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_priority(const std::string& priority) {
    const char* sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                      "status, priority, due_date, created_at, updated_at, completed_at FROM tasks WHERE priority = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_text(stmt, 1, priority.c_str(), -1, SQLITE_STATIC);

    std::vector<std::unique_ptr<Task>> tasks;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto task = std::make_unique<Task>();
        task->id = sqlite3_column_int(stmt, 0);
        task->title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task->description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task->assigned_user_id = sqlite3_column_int(stmt, 3);
        task->created_by_user_id = sqlite3_column_int(stmt, 4);
        task->device_id = sqlite3_column_int(stmt, 5);
        task->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task->priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        // Handle datetime columns
        const char* due_date_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* completed_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        tasks.push_back(std::move(task));
    }

    sqlite3_finalize(stmt);
    return tasks;
}