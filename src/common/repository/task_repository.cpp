#include "task_repository.h"
#include <QDateTime>

// Static member initialization
DatabaseManager* TaskRepository::default_db_manager_ = nullptr;

TaskRepository::TaskRepository() : db_manager_(default_db_manager_) {}

TaskRepository::TaskRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool TaskRepository::create_table() {
    QString sql = R"(
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
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        auto task = std::make_unique<Task>();
        task->id = query.value(0).toInt();
        task->title = query.value(1).toString().toStdString();
        task->description = query.value(2).toString().toStdString();
        task->assigned_user_id = query.value(3).toInt();
        task->created_by_user_id = query.value(4).toInt();
        task->device_id = query.value(5).toInt();
        task->status = query.value(6).toString().toStdString();
        task->priority = query.value(7).toString().toStdString();

        QString due_date_str = query.value(8).toString();
        if (!due_date_str.isEmpty()) {
            QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
            if (!due_date.isValid()) {
                due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
            }
            task->due_date = due_date.toSecsSinceEpoch();
        }

        QString completed_at_str = query.value(9).toString();
        if (!completed_at_str.isEmpty()) {
            QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
            if (!completed_at.isValid()) {
                completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
            }
            task->completed_at = completed_at.toSecsSinceEpoch();
        }
        return task;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_all() {
    std::vector<std::unique_ptr<Task>> tasks;
    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                   "status, priority, due_date, completed_at FROM tasks")) {
        while (query.next()) {
            auto task = std::make_unique<Task>();
            task->id = query.value(0).toInt();
            task->title = query.value(1).toString().toStdString();
            task->description = query.value(2).toString().toStdString();
            task->assigned_user_id = query.value(3).toInt();
            task->created_by_user_id = query.value(4).toInt();
            task->device_id = query.value(5).toInt();
            task->status = query.value(6).toString().toStdString();
            task->priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task->due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task->completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.push_back(std::move(task));
        }
    }
    return tasks;
}

bool TaskRepository::create(Task& task) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO tasks (title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(QString::fromStdString(task.title));
    query.addBindValue(QString::fromStdString(task.description));
    query.addBindValue(task.assigned_user_id);
    query.addBindValue(task.created_by_user_id);
    query.addBindValue(task.device_id);
    query.addBindValue(QString::fromStdString(task.status));
    query.addBindValue(QString::fromStdString(task.priority));

    if (task.due_date != 0) {
        QDateTime due_date;
        due_date.setSecsSinceEpoch(task.due_date);
        query.addBindValue(due_date.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        query.addBindValue(QVariant());
    }

    if (query.exec()) {
        task.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool TaskRepository::update(const Task& task) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE tasks SET title = ?, description = ?, assigned_user_id = ?, created_by_user_id = ?, "
                  "device_id = ?, status = ?, priority = ?, due_date = ?, completed_at = ?, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(QString::fromStdString(task.title));
    query.addBindValue(QString::fromStdString(task.description));
    query.addBindValue(task.assigned_user_id);
    query.addBindValue(task.created_by_user_id);
    query.addBindValue(task.device_id);
    query.addBindValue(QString::fromStdString(task.status));
    query.addBindValue(QString::fromStdString(task.priority));

    if (task.due_date != 0) {
        QDateTime due_date;
        due_date.setSecsSinceEpoch(task.due_date);
        query.addBindValue(due_date.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        query.addBindValue(QVariant());
    }

    if (task.completed_at != 0) {
        QDateTime completed_at;
        completed_at.setSecsSinceEpoch(task.completed_at);
        query.addBindValue(completed_at.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        query.addBindValue(QVariant());
    }

    query.addBindValue(task.id);

    return query.exec();
}

bool TaskRepository::remove(int id) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM tasks WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_assigned_user(int user_id) {
    std::vector<std::unique_ptr<Task>> tasks;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE assigned_user_id = ?");
    query.addBindValue(user_id);

    if (query.exec()) {
        while (query.next()) {
            auto task = std::make_unique<Task>();
            task->id = query.value(0).toInt();
            task->title = query.value(1).toString().toStdString();
            task->description = query.value(2).toString().toStdString();
            task->assigned_user_id = query.value(3).toInt();
            task->created_by_user_id = query.value(4).toInt();
            task->device_id = query.value(5).toInt();
            task->status = query.value(6).toString().toStdString();
            task->priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task->due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task->completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.push_back(std::move(task));
        }
    }
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_created_by_user(int user_id) {
    std::vector<std::unique_ptr<Task>> tasks;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE created_by_user_id = ?");
    query.addBindValue(user_id);

    if (query.exec()) {
        while (query.next()) {
            auto task = std::make_unique<Task>();
            task->id = query.value(0).toInt();
            task->title = query.value(1).toString().toStdString();
            task->description = query.value(2).toString().toStdString();
            task->assigned_user_id = query.value(3).toInt();
            task->created_by_user_id = query.value(4).toInt();
            task->device_id = query.value(5).toInt();
            task->status = query.value(6).toString().toStdString();
            task->priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task->due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task->completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.push_back(std::move(task));
        }
    }
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_device(int device_id) {
    std::vector<std::unique_ptr<Task>> tasks;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE device_id = ?");
    query.addBindValue(device_id);

    if (query.exec()) {
        while (query.next()) {
            auto task = std::make_unique<Task>();
            task->id = query.value(0).toInt();
            task->title = query.value(1).toString().toStdString();
            task->description = query.value(2).toString().toStdString();
            task->assigned_user_id = query.value(3).toInt();
            task->created_by_user_id = query.value(4).toInt();
            task->device_id = query.value(5).toInt();
            task->status = query.value(6).toString().toStdString();
            task->priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task->due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task->completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.push_back(std::move(task));
        }
    }
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_status(const std::string& status) {
    std::vector<std::unique_ptr<Task>> tasks;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE status = ?");
    query.addBindValue(QString::fromStdString(status));

    if (query.exec()) {
        while (query.next()) {
            auto task = std::make_unique<Task>();
            task->id = query.value(0).toInt();
            task->title = query.value(1).toString().toStdString();
            task->description = query.value(2).toString().toStdString();
            task->assigned_user_id = query.value(3).toInt();
            task->created_by_user_id = query.value(4).toInt();
            task->device_id = query.value(5).toInt();
            task->status = query.value(6).toString().toStdString();
            task->priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task->due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task->completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.push_back(std::move(task));
        }
    }
    return tasks;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_priority(const std::string& priority) {
    std::vector<std::unique_ptr<Task>> tasks;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE priority = ?");
    query.addBindValue(QString::fromStdString(priority));

    if (query.exec()) {
        while (query.next()) {
            auto task = std::make_unique<Task>();
            task->id = query.value(0).toInt();
            task->title = query.value(1).toString().toStdString();
            task->description = query.value(2).toString().toStdString();
            task->assigned_user_id = query.value(3).toInt();
            task->created_by_user_id = query.value(4).toInt();
            task->device_id = query.value(5).toInt();
            task->status = query.value(6).toString().toStdString();
            task->priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task->due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task->completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.push_back(std::move(task));
        }
    }
    return tasks;
}

Task TaskRepository::findById(int id) {
    Task task;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        task.id = query.value(0).toInt();
        task.title = query.value(1).toString().toStdString();
        task.description = query.value(2).toString().toStdString();
        task.assigned_user_id = query.value(3).toInt();
        task.created_by_user_id = query.value(4).toInt();
        task.device_id = query.value(5).toInt();
        task.status = query.value(6).toString().toStdString();
        task.priority = query.value(7).toString().toStdString();

        QString due_date_str = query.value(8).toString();
        if (!due_date_str.isEmpty()) {
            QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
            if (!due_date.isValid()) {
                due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
            }
            task.due_date = due_date.toSecsSinceEpoch();
        }

        QString completed_at_str = query.value(9).toString();
        if (!completed_at_str.isEmpty()) {
            QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
            if (!completed_at.isValid()) {
                completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
            }
            task.completed_at = completed_at.toSecsSinceEpoch();
        }
    }
    return task;
}

QList<Task> TaskRepository::findByUserId(int userId, const QString& taskType, const QString& status, int limit, int offset) {
    QList<Task> tasks;
    QSqlQuery query(db_manager_->get_connection());

    QString sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE (assigned_user_id = ? OR created_by_user_id = ?)";

    if (!status.isEmpty()) {
        sql += " AND status = ?";
    }
    if (!taskType.isEmpty()) {
        sql += " AND priority = ?";
    }
    if (limit > 0) {
        sql += QString(" LIMIT %1").arg(limit);
        if (offset > 0) {
            sql += QString(" OFFSET %1").arg(offset);
        }
    }

    query.prepare(sql);
    query.addBindValue(userId);
    query.addBindValue(userId);

    if (!status.isEmpty()) {
        query.addBindValue(status);
    }
    if (!taskType.isEmpty()) {
        query.addBindValue(taskType);
    }

    if (query.exec()) {
        while (query.next()) {
            Task task;
            task.id = query.value(0).toInt();
            task.title = query.value(1).toString().toStdString();
            task.description = query.value(2).toString().toStdString();
            task.assigned_user_id = query.value(3).toInt();
            task.created_by_user_id = query.value(4).toInt();
            task.device_id = query.value(5).toInt();
            task.status = query.value(6).toString().toStdString();
            task.priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task.due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task.completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.append(task);
        }
    }
    return tasks;
}

QList<Task> TaskRepository::findPendingApprovalTasks(const QString& approverRole) {
    QList<Task> tasks;
    QSqlQuery query(db_manager_->get_connection());

    // Find tasks that are pending approval based on approver role
    // This assumes there's a workflow or approval mechanism
    QString sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at FROM tasks WHERE status = 'pending_approval'";

    query.prepare(sql);

    if (query.exec()) {
        while (query.next()) {
            Task task;
            task.id = query.value(0).toInt();
            task.title = query.value(1).toString().toStdString();
            task.description = query.value(2).toString().toStdString();
            task.assigned_user_id = query.value(3).toInt();
            task.created_by_user_id = query.value(4).toInt();
            task.device_id = query.value(5).toInt();
            task.status = query.value(6).toString().toStdString();
            task.priority = query.value(7).toString().toStdString();

            QString due_date_str = query.value(8).toString();
            if (!due_date_str.isEmpty()) {
                QDateTime due_date = QDateTime::fromString(due_date_str, "yyyy-MM-dd hh:mm:ss");
                if (!due_date.isValid()) {
                    due_date = QDateTime::fromString(due_date_str, Qt::ISODate);
                }
                task.due_date = due_date.toSecsSinceEpoch();
            }

            QString completed_at_str = query.value(9).toString();
            if (!completed_at_str.isEmpty()) {
                QDateTime completed_at = QDateTime::fromString(completed_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!completed_at.isValid()) {
                    completed_at = QDateTime::fromString(completed_at_str, Qt::ISODate);
                }
                task.completed_at = completed_at.toSecsSinceEpoch();
            }
            tasks.append(task);
        }
    }
    return tasks;
}