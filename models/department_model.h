#ifndef DEPARTMENT_MODEL_H
#define DEPARTMENT_MODEL_H

#include <string>
#include <ctime>

struct Department {
    int id;
    std::string name;
    std::string description;
    int manager_id;  // ID of the department manager
    int parent_department_id;  // For hierarchical structure
    bool is_active;
    std::time_t created_at;
    std::time_t updated_at;

    Department() : id(0), manager_id(0), parent_department_id(0), is_active(true) {}
};

#endif // DEPARTMENT_MODEL_H