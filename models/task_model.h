#ifndef TASK_MODEL_H
#define TASK_MODEL_H

#include <string>
#include <ctime>

struct Task {
    int id;
    std::string title;
    std::string description;
    int assigned_user_id;
    int created_by_user_id;
    int device_id;
    std::string status;  // "pending", "in_progress", "completed", "cancelled"
    std::string priority;  // "low", "medium", "high", "critical"
    std::time_t due_date;
    std::time_t created_at;
    std::time_t updated_at;
    std::time_t completed_at;

    Task() : id(0), assigned_user_id(0), created_by_user_id(0), device_id(0) {}
};

#endif // TASK_MODEL_H