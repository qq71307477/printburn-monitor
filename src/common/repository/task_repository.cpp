#include "task_repository.h"
#include <QVariant>
#include <QDateTime>

// Static member initialization
DatabaseManager* TaskRepository::default_db_manager_ = nullptr;

TaskRepository::TaskRepository() : db_manager_(default_db_manager_) {}

TaskRepository::TaskRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool TaskRepository::create_table() {
    if (!db_manager_) return false;

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
            user_id INTEGER,
            type TEXT,
            file_path TEXT,
            copies INTEGER DEFAULT 0,
            approver_id INTEGER,
            approval_status TEXT,
            media_type TEXT,
            session_mode TEXT,
            approval_time DATETIME,
            approval_reason TEXT,
            serial_number TEXT,
            FOREIGN KEY (assigned_user_id) REFERENCES users(id),
            FOREIGN KEY (created_by_user_id) REFERENCES users(id),
            FOREIGN KEY (device_id) REFERENCES devices(id),
            FOREIGN KEY (user_id) REFERENCES users(id),
            FOREIGN KEY (approver_id) REFERENCES users(id)
        );

        CREATE INDEX IF NOT EXISTS idx_tasks_assigned_user ON tasks(assigned_user_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_created_by ON tasks(created_by_user_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_device_id ON tasks(device_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status);
        CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);
        CREATE INDEX IF NOT EXISTS idx_tasks_user_id ON tasks(user_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_approval_status ON tasks(approval_status);
        CREATE INDEX IF NOT EXISTS idx_tasks_serial_number ON tasks(serial_number);
    )";
    return db_manager_->execute_query(sql);
}

std::unique_ptr<Task> TaskRepository::find_by_id(int id) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at, user_id, type, file_path, copies, "
                  "approver_id, approval_status, media_type, session_mode, approval_time, approval_reason, serial_number FROM tasks WHERE id = ?");
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

        // New fields
        task->setUserId(query.value(10).toInt());
        task->setType(query.value(11).toString());
        task->setFilePath(query.value(12).toString());
        task->setCopies(query.value(13).toInt());
        task->setApproverId(query.value(14).toInt());
        task->setApprovalStatus(query.value(15).toString());
        task->setMediaType(query.value(16).toString());
        task->setSessionMode(query.value(17).toString());

        QString approval_time_str = query.value(18).toString();
        if (!approval_time_str.isEmpty()) {
            QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
            if (!approval_time.isValid()) {
                approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
            }
            task->setApprovalTime(approval_time);
        }
        task->setApprovalReason(query.value(19).toString());
        task->setSerialNumber(query.value(20).toString());

        return task;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_all() {
    std::vector<std::unique_ptr<Task>> tasks;
    if (!db_manager_) return tasks;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                   "status, priority, due_date, completed_at, user_id, type, file_path, copies, "
                   "approver_id, approval_status, media_type, session_mode, approval_time, approval_reason, serial_number FROM tasks")) {
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

            // New fields
            task->setUserId(query.value(10).toInt());
            task->setType(query.value(11).toString());
            task->setFilePath(query.value(12).toString());
            task->setCopies(query.value(13).toInt());
            task->setApproverId(query.value(14).toInt());
            task->setApprovalStatus(query.value(15).toString());
            task->setMediaType(query.value(16).toString());
            task->setSessionMode(query.value(17).toString());

            QString approval_time_str = query.value(18).toString();
            if (!approval_time_str.isEmpty()) {
                QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
                if (!approval_time.isValid()) {
                    approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
                }
                task->setApprovalTime(approval_time);
            }
            task->setApprovalReason(query.value(19).toString());
            task->setSerialNumber(query.value(20).toString());

            tasks.push_back(std::move(task));
        }
    }
    return tasks;
}

bool TaskRepository::create(Task& task) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO tasks (title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, user_id, type, file_path, copies, approver_id, "
                  "approval_status, media_type, session_mode, approval_time, approval_reason) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
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

    // New fields
    query.addBindValue(task.getUserId());
    query.addBindValue(task.getType());
    query.addBindValue(task.getFilePath());
    query.addBindValue(task.getCopies());
    query.addBindValue(task.getApproverId());
    query.addBindValue(task.getApprovalStatus());
    query.addBindValue(task.getMediaType());
    query.addBindValue(task.getSessionMode());

    QDateTime approvalTime = task.getApprovalTime();
    if (approvalTime.isValid()) {
        query.addBindValue(approvalTime.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        query.addBindValue(QVariant());
    }
    query.addBindValue(task.getApprovalReason());

    if (query.exec()) {
        task.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool TaskRepository::update(const Task& task) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE tasks SET title = ?, description = ?, assigned_user_id = ?, created_by_user_id = ?, "
                  "device_id = ?, status = ?, priority = ?, due_date = ?, completed_at = ?, "
                  "user_id = ?, type = ?, file_path = ?, copies = ?, approver_id = ?, "
                  "approval_status = ?, media_type = ?, session_mode = ?, approval_time = ?, approval_reason = ?, "
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

    // New fields
    query.addBindValue(task.getUserId());
    query.addBindValue(task.getType());
    query.addBindValue(task.getFilePath());
    query.addBindValue(task.getCopies());
    query.addBindValue(task.getApproverId());
    query.addBindValue(task.getApprovalStatus());
    query.addBindValue(task.getMediaType());
    query.addBindValue(task.getSessionMode());

    QDateTime approvalTime = task.getApprovalTime();
    if (approvalTime.isValid()) {
        query.addBindValue(approvalTime.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        query.addBindValue(QVariant());
    }
    query.addBindValue(task.getApprovalReason());

    query.addBindValue(task.id);

    return query.exec();
}

bool TaskRepository::remove(int id) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM tasks WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

std::vector<std::unique_ptr<Task>> TaskRepository::find_by_assigned_user(int user_id) {
    std::vector<std::unique_ptr<Task>> tasks;
    if (!db_manager_) return tasks;

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
    if (!db_manager_) return tasks;

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
    if (!db_manager_) return tasks;

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
    if (!db_manager_) return tasks;

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
    if (!db_manager_) return tasks;

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
    if (!db_manager_) return task;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at, created_at, updated_at, user_id, type, file_path, copies, "
                  "approver_id, approval_status, media_type, session_mode, approval_time, approval_reason, serial_number FROM tasks WHERE id = ?");
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

        // created_at and updated_at
        QString created_at_str = query.value(10).toString();
        if (!created_at_str.isEmpty()) {
            QDateTime created_at = QDateTime::fromString(created_at_str, "yyyy-MM-dd hh:mm:ss");
            if (!created_at.isValid()) {
                created_at = QDateTime::fromString(created_at_str, Qt::ISODate);
            }
            task.created_at = created_at.toSecsSinceEpoch();
        }

        QString updated_at_str = query.value(11).toString();
        if (!updated_at_str.isEmpty()) {
            QDateTime updated_at = QDateTime::fromString(updated_at_str, "yyyy-MM-dd hh:mm:ss");
            if (!updated_at.isValid()) {
                updated_at = QDateTime::fromString(updated_at_str, Qt::ISODate);
            }
            task.updated_at = updated_at.toSecsSinceEpoch();
        }

        // New fields
        task.setUserId(query.value(12).toInt());
        task.setType(query.value(13).toString());
        task.setFilePath(query.value(14).toString());
        task.setCopies(query.value(15).toInt());
        task.setApproverId(query.value(16).toInt());
        task.setApprovalStatus(query.value(17).toString());
        task.setMediaType(query.value(18).toString());
        task.setSessionMode(query.value(19).toString());

        QString approval_time_str = query.value(20).toString();
        if (!approval_time_str.isEmpty()) {
            QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
            if (!approval_time.isValid()) {
                approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
            }
            task.setApprovalTime(approval_time);
        }
        task.setApprovalReason(query.value(21).toString());
        task.setSerialNumber(query.value(22).toString());
    }
    return task;
}

QList<Task> TaskRepository::findByUserId(int userId, const QString& taskType, const QString& status, int limit, int offset) {
    QList<Task> tasks;
    if (!db_manager_) return tasks;

    QSqlQuery query(db_manager_->get_connection());

    QString sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at, user_id, type, file_path, copies, "
                  "approver_id, approval_status, media_type, session_mode, approval_time, approval_reason "
                  "FROM tasks WHERE (assigned_user_id = ? OR created_by_user_id = ?)";

    if (!status.isEmpty()) {
        sql += " AND status = ?";
    }
    if (!taskType.isEmpty()) {
        sql += " AND type = ?";
    }
    if (limit > 0) {
        sql += " LIMIT ?";
        if (offset > 0) {
            sql += " OFFSET ?";
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
    if (limit > 0) {
        query.addBindValue(limit);
        if (offset > 0) {
            query.addBindValue(offset);
        }
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

            // New fields
            task.setUserId(query.value(10).toInt());
            task.setType(query.value(11).toString());
            task.setFilePath(query.value(12).toString());
            task.setCopies(query.value(13).toInt());
            task.setApproverId(query.value(14).toInt());
            task.setApprovalStatus(query.value(15).toString());
            task.setMediaType(query.value(16).toString());
            task.setSessionMode(query.value(17).toString());

            QString approval_time_str = query.value(18).toString();
            if (!approval_time_str.isEmpty()) {
                QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
                if (!approval_time.isValid()) {
                    approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
                }
                task.setApprovalTime(approval_time);
            }
            task.setApprovalReason(query.value(19).toString());

            tasks.append(task);
        }
    }
    return tasks;
}

QList<Task> TaskRepository::findByType(const QString& taskType, const QString& status, int limit, int offset) {
    QList<Task> tasks;
    if (!db_manager_) return tasks;

    QSqlQuery query(db_manager_->get_connection());

    QString sql = "SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                  "status, priority, due_date, completed_at, created_at, updated_at, user_id, type, file_path, copies, "
                  "approver_id, approval_status, media_type, session_mode, approval_time, approval_reason "
                  "FROM tasks WHERE type = ?";

    if (!status.isEmpty()) {
        sql += " AND status = ?";
    }
    sql += " ORDER BY created_at DESC";
    if (limit > 0) {
        sql += " LIMIT ?";
        if (offset > 0) {
            sql += " OFFSET ?";
        }
    }

    query.prepare(sql);
    query.addBindValue(taskType);

    if (!status.isEmpty()) {
        query.addBindValue(status);
    }
    if (limit > 0) {
        query.addBindValue(limit);
        if (offset > 0) {
            query.addBindValue(offset);
        }
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

            // created_at and updated_at
            QString created_at_str = query.value(10).toString();
            if (!created_at_str.isEmpty()) {
                QDateTime created_at = QDateTime::fromString(created_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!created_at.isValid()) {
                    created_at = QDateTime::fromString(created_at_str, Qt::ISODate);
                }
                task.created_at = created_at.toSecsSinceEpoch();
            }

            QString updated_at_str = query.value(11).toString();
            if (!updated_at_str.isEmpty()) {
                QDateTime updated_at = QDateTime::fromString(updated_at_str, "yyyy-MM-dd hh:mm:ss");
                if (!updated_at.isValid()) {
                    updated_at = QDateTime::fromString(updated_at_str, Qt::ISODate);
                }
                task.updated_at = updated_at.toSecsSinceEpoch();
            }

            // New fields
            task.setUserId(query.value(12).toInt());
            task.setType(query.value(13).toString());
            task.setFilePath(query.value(14).toString());
            task.setCopies(query.value(15).toInt());
            task.setApproverId(query.value(16).toInt());
            task.setApprovalStatus(query.value(17).toString());
            task.setMediaType(query.value(18).toString());
            task.setSessionMode(query.value(19).toString());

            QString approval_time_str = query.value(20).toString();
            if (!approval_time_str.isEmpty()) {
                QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
                if (!approval_time.isValid()) {
                    approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
                }
                task.setApprovalTime(approval_time);
            }
            task.setApprovalReason(query.value(21).toString());

            tasks.append(task);
        }
    }
    return tasks;
}

QList<Task> TaskRepository::findPendingApprovalTasks(const QString& approverRole) {
    QList<Task> tasks;
    if (!db_manager_) return tasks;

    QSqlQuery query(db_manager_->get_connection());

    // Find tasks that are pending approval for a specific approver role
    // JOIN with users, user_roles, and roles to filter by approver's role
    QString sql = "SELECT t.id, t.title, t.description, t.assigned_user_id, t.created_by_user_id, t.device_id, "
                  "t.status, t.priority, t.due_date, t.completed_at, t.user_id, t.type, t.file_path, t.copies, "
                  "t.approver_id, t.approval_status, t.media_type, t.session_mode, t.approval_time, t.approval_reason "
                  "FROM tasks t "
                  "LEFT JOIN users u ON t.approver_id = u.id "
                  "LEFT JOIN user_roles ur ON u.id = ur.user_id "
                  "LEFT JOIN roles r ON ur.role_id = r.id "
                  "WHERE t.status = 'pending_approval'";

    if (!approverRole.isEmpty()) {
        sql += " AND r.name = ?";
    }

    query.prepare(sql);

    if (!approverRole.isEmpty()) {
        query.addBindValue(approverRole);
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

            // New fields
            task.setUserId(query.value(10).toInt());
            task.setType(query.value(11).toString());
            task.setFilePath(query.value(12).toString());
            task.setCopies(query.value(13).toInt());
            task.setApproverId(query.value(14).toInt());
            task.setApprovalStatus(query.value(15).toString());
            task.setMediaType(query.value(16).toString());
            task.setSessionMode(query.value(17).toString());

            QString approval_time_str = query.value(18).toString();
            if (!approval_time_str.isEmpty()) {
                QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
                if (!approval_time.isValid()) {
                    approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
                }
                task.setApprovalTime(approval_time);
            }
            task.setApprovalReason(query.value(19).toString());

            tasks.append(task);
        }
    }
    return tasks;
}

bool TaskRepository::updateSerialNumber(int taskId, const QString& serialNumber) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE tasks SET serial_number = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(serialNumber);
    query.addBindValue(taskId);

    return query.exec();
}

// 用户名缓存实现
QCache<int, QString> UserNameCache::m_cache(MAX_CACHE_SIZE);

QString UserNameCache::getUserName(int userId) {
    if (m_cache.contains(userId)) {
        return *m_cache.object(userId);
    }
    return QString();
}

void UserNameCache::cacheUserName(int userId, const QString& username) {
    m_cache.insert(userId, new QString(username));
}

void UserNameCache::clear() {
    m_cache.clear();
}

// 分页查询方法实现
TaskRepository::PagedResult TaskRepository::findByUserIdPaged(int userId, const QString& taskType, const QString& status,
                                                               int page, int pageSize, const QString& sortBy, bool sortDesc) {
    PagedResult result;
    result.totalCount = 0;
    result.totalPages = 0;

    if (!db_manager_) return result;

    // 先计算总数
    QString countSql = "SELECT COUNT(*) FROM tasks WHERE (assigned_user_id = ? OR created_by_user_id = ?)";
    if (!taskType.isEmpty()) countSql += " AND type = ?";
    if (!status.isEmpty()) countSql += " AND status = ?";

    QSqlQuery countQuery(db_manager_->get_connection());
    countQuery.prepare(countSql);
    countQuery.addBindValue(userId);
    countQuery.addBindValue(userId);
    if (!taskType.isEmpty()) countQuery.addBindValue(taskType);
    if (!status.isEmpty()) countQuery.addBindValue(status);

    if (countQuery.exec() && countQuery.next()) {
        result.totalCount = countQuery.value(0).toInt();
        result.totalPages = (result.totalCount + pageSize - 1) / pageSize;
    }

    // 分页查询数据
    QString orderDirection = sortDesc ? "DESC" : "ASC";
    QString sql = QString("SELECT id, title, description, assigned_user_id, created_by_user_id, device_id, "
                          "status, priority, due_date, completed_at, user_id, type, file_path, copies, "
                          "approver_id, approval_status, media_type, session_mode, approval_time, approval_reason "
                          "FROM tasks WHERE (assigned_user_id = ? OR created_by_user_id = ?)");
    if (!taskType.isEmpty()) sql += " AND type = ?";
    if (!status.isEmpty()) sql += " AND status = ?";
    sql += QString(" ORDER BY %1 %2 LIMIT ? OFFSET ?").arg(sortBy, orderDirection);

    QSqlQuery query(db_manager_->get_connection());
    query.prepare(sql);
    query.addBindValue(userId);
    query.addBindValue(userId);
    if (!taskType.isEmpty()) query.addBindValue(taskType);
    if (!status.isEmpty()) query.addBindValue(status);
    query.addBindValue(pageSize);
    query.addBindValue((page - 1) * pageSize);

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

            task.setUserId(query.value(10).toInt());
            task.setType(query.value(11).toString());
            task.setFilePath(query.value(12).toString());
            task.setCopies(query.value(13).toInt());
            task.setApproverId(query.value(14).toInt());
            task.setApprovalStatus(query.value(15).toString());
            task.setMediaType(query.value(16).toString());
            task.setSessionMode(query.value(17).toString());

            QString approval_time_str = query.value(18).toString();
            if (!approval_time_str.isEmpty()) {
                QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
                if (!approval_time.isValid()) {
                    approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
                }
                task.setApprovalTime(approval_time);
            }
            task.setApprovalReason(query.value(19).toString());

            result.tasks.append(task);
        }
    }

    return result;
}

TaskRepository::PagedResult TaskRepository::findPendingApprovalTasksPaged(const QString& approverRole, const QString& searchText,
                                                                           const QString& taskType, const QString& status,
                                                                           int page, int pageSize, const QString& sortBy, bool sortDesc) {
    PagedResult result;
    result.totalCount = 0;
    result.totalPages = 0;

    if (!db_manager_) return result;

    // 构建查询条件
    QString whereClause = "WHERE t.status = 'pending_approval'";
    if (!approverRole.isEmpty()) {
        whereClause += " AND r.name = ?";
    }
    if (!searchText.isEmpty()) {
        whereClause += " AND (t.title LIKE ? OR t.description LIKE ?)";
    }
    if (!taskType.isEmpty()) {
        whereClause += " AND t.type = ?";
    }
    if (!status.isEmpty()) {
        whereClause += " AND t.approval_status = ?";
    }

    // 先计算总数
    QString countSql = QString("SELECT COUNT(DISTINCT t.id) FROM tasks t "
                               "LEFT JOIN users u ON t.approver_id = u.id "
                               "LEFT JOIN user_roles ur ON u.id = ur.user_id "
                               "LEFT JOIN roles r ON ur.role_id = r.id %1").arg(whereClause);

    QSqlQuery countQuery(db_manager_->get_connection());
    countQuery.prepare(countSql);

    int bindIndex = 0;
    if (!approverRole.isEmpty()) {
        countQuery.addBindValue(approverRole);
    }
    if (!searchText.isEmpty()) {
        QString pattern = "%" + searchText + "%";
        countQuery.addBindValue(pattern);
        countQuery.addBindValue(pattern);
    }
    if (!taskType.isEmpty()) {
        countQuery.addBindValue(taskType);
    }
    if (!status.isEmpty()) {
        countQuery.addBindValue(status);
    }

    if (countQuery.exec() && countQuery.next()) {
        result.totalCount = countQuery.value(0).toInt();
        result.totalPages = (result.totalCount + pageSize - 1) / pageSize;
    }

    // 分页查询数据
    QString orderDirection = sortDesc ? "DESC" : "ASC";
    QString sql = QString("SELECT DISTINCT t.id, t.title, t.description, t.assigned_user_id, t.created_by_user_id, t.device_id, "
                          "t.status, t.priority, t.due_date, t.completed_at, t.user_id, t.type, t.file_path, t.copies, "
                          "t.approver_id, t.approval_status, t.media_type, t.session_mode, t.approval_time, t.approval_reason "
                          "FROM tasks t "
                          "LEFT JOIN users u ON t.approver_id = u.id "
                          "LEFT JOIN user_roles ur ON u.id = ur.user_id "
                          "LEFT JOIN roles r ON ur.role_id = r.id "
                          "%1 ORDER BY t.%2 %3 LIMIT ? OFFSET ?").arg(whereClause, sortBy, orderDirection);

    QSqlQuery query(db_manager_->get_connection());
    query.prepare(sql);

    if (!approverRole.isEmpty()) {
        query.addBindValue(approverRole);
    }
    if (!searchText.isEmpty()) {
        QString pattern = "%" + searchText + "%";
        query.addBindValue(pattern);
        query.addBindValue(pattern);
    }
    if (!taskType.isEmpty()) {
        query.addBindValue(taskType);
    }
    if (!status.isEmpty()) {
        query.addBindValue(status);
    }
    query.addBindValue(pageSize);
    query.addBindValue((page - 1) * pageSize);

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

            task.setUserId(query.value(10).toInt());
            task.setType(query.value(11).toString());
            task.setFilePath(query.value(12).toString());
            task.setCopies(query.value(13).toInt());
            task.setApproverId(query.value(14).toInt());
            task.setApprovalStatus(query.value(15).toString());
            task.setMediaType(query.value(16).toString());
            task.setSessionMode(query.value(17).toString());

            QString approval_time_str = query.value(18).toString();
            if (!approval_time_str.isEmpty()) {
                QDateTime approval_time = QDateTime::fromString(approval_time_str, "yyyy-MM-dd hh:mm:ss");
                if (!approval_time.isValid()) {
                    approval_time = QDateTime::fromString(approval_time_str, Qt::ISODate);
                }
                task.setApprovalTime(approval_time);
            }
            task.setApprovalReason(query.value(19).toString());

            result.tasks.append(task);
        }
    }

    return result;
}

int TaskRepository::countByUserId(int userId, const QString& taskType, const QString& status) {
    if (!db_manager_) return 0;

    QString sql = "SELECT COUNT(*) FROM tasks WHERE (assigned_user_id = ? OR created_by_user_id = ?)";
    if (!taskType.isEmpty()) sql += " AND type = ?";
    if (!status.isEmpty()) sql += " AND status = ?";

    QSqlQuery query(db_manager_->get_connection());
    query.prepare(sql);
    query.addBindValue(userId);
    query.addBindValue(userId);
    if (!taskType.isEmpty()) query.addBindValue(taskType);
    if (!status.isEmpty()) query.addBindValue(status);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int TaskRepository::countPendingApproval(const QString& approverRole) {
    if (!db_manager_) return 0;

    QString sql = "SELECT COUNT(DISTINCT t.id) FROM tasks t "
                  "LEFT JOIN users u ON t.approver_id = u.id "
                  "LEFT JOIN user_roles ur ON u.id = ur.user_id "
                  "LEFT JOIN roles r ON ur.role_id = r.id "
                  "WHERE t.status = 'pending_approval'";

    if (!approverRole.isEmpty()) {
        sql += " AND r.name = ?";
    }

    QSqlQuery query(db_manager_->get_connection());
    query.prepare(sql);
    if (!approverRole.isEmpty()) {
        query.addBindValue(approverRole);
    }

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}
