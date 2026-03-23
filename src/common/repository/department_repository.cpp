#include "department_repository.h"

DepartmentRepository::DepartmentRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool DepartmentRepository::create_table() {
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS departments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            manager_id INTEGER,
            parent_department_id INTEGER,
            is_active BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
        CREATE INDEX IF NOT EXISTS idx_departments_name ON departments(name);
    )";
    return db_manager_->execute_query(sql);
}

std::unique_ptr<Department> DepartmentRepository::find_by_id(int id) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, description, manager_id, parent_department_id, is_active FROM departments WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        auto dept = std::make_unique<Department>();
        dept->id = query.value(0).toInt();
        dept->name = query.value(1).toString().toStdString();
        dept->description = query.value(2).toString().toStdString();
        dept->manager_id = query.value(3).toInt();
        dept->parent_department_id = query.value(4).toInt();
        dept->is_active = query.value(5).toBool();
        return dept;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Department>> DepartmentRepository::find_all() {
    std::vector<std::unique_ptr<Department>> departments;
    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, name, description, manager_id, parent_department_id, is_active FROM departments")) {
        while (query.next()) {
            auto dept = std::make_unique<Department>();
            dept->id = query.value(0).toInt();
            dept->name = query.value(1).toString().toStdString();
            dept->description = query.value(2).toString().toStdString();
            dept->manager_id = query.value(3).toInt();
            dept->parent_department_id = query.value(4).toInt();
            dept->is_active = query.value(5).toBool();
            departments.push_back(std::move(dept));
        }
    }
    return departments;
}

bool DepartmentRepository::create(Department& department) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO departments (name, description, manager_id, parent_department_id, is_active) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(QString::fromStdString(department.name));
    query.addBindValue(QString::fromStdString(department.description));
    query.addBindValue(department.manager_id);
    query.addBindValue(department.parent_department_id);
    query.addBindValue(department.is_active);

    if (query.exec()) {
        department.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool DepartmentRepository::update(const Department& department) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE departments SET name = ?, description = ?, manager_id = ?, parent_department_id = ?, is_active = ? WHERE id = ?");
    query.addBindValue(QString::fromStdString(department.name));
    query.addBindValue(QString::fromStdString(department.description));
    query.addBindValue(department.manager_id);
    query.addBindValue(department.parent_department_id);
    query.addBindValue(department.is_active);
    query.addBindValue(department.id);
    return query.exec();
}

bool DepartmentRepository::remove(int id) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM departments WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}