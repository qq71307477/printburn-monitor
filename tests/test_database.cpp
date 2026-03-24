#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include "src/common/repository/database_manager.h"
#include "src/common/repository/user_repository.h"
#include "src/common/repository/role_repository.h"
#include "src/common/repository/department_repository.h"
#include "src/common/repository/device_repository.h"
#include "src/common/repository/task_repository.h"

void test_database_initialization() {
    std::cout << "Testing database initialization...\n";

    try {
        DatabaseManager db("./test_database.db");
        bool initialized = db.initialize();
        assert(initialized == true);
        std::cout << "✓ Database initialization successful\n";
    } catch (const std::exception& e) {
        std::cerr << "✗ Database initialization failed: " << e.what() << std::endl;
        assert(false);
    }
}

void test_role_repository() {
    std::cout << "Testing Role Repository...\n";

    DatabaseManager db("./test_database.db");
    db.initialize();
    RoleRepository role_repo(&db);

    // Create table
    bool table_created = role_repo.create_table();
    assert(table_created == true);
    std::cout << "✓ Role table creation successful\n";

    // Create a role
    Role role;
    role.name = "Test Role";
    role.description = "A test role for unit testing";
    role.permissions = "{\"users\": \"read\", \"devices\": \"read\"}";

    bool created = role_repo.create(role);
    assert(created == true && role.id > 0);
    std::cout << "✓ Role creation successful\n";

    // Find the role by ID
    auto found_role = role_repo.find_by_id(role.id);
    assert(found_role != nullptr);
    assert(found_role->name == role.name);
    std::cout << "✓ Role retrieval by ID successful\n";

    // Find the role by name
    auto found_by_name = role_repo.find_by_name(role.name);
    assert(found_by_name != nullptr);
    assert(found_by_name->id == role.id);
    std::cout << "✓ Role retrieval by name successful\n";

    // Update the role
    role.description = "Updated test role description";
    bool updated = role_repo.update(role);
    assert(updated == true);

    // Verify update
    auto updated_role = role_repo.find_by_id(role.id);
    assert(updated_role != nullptr);
    assert(updated_role->description == "Updated test role description");
    std::cout << "✓ Role update successful\n";

    // Find all roles
    auto all_roles = role_repo.find_all();
    assert(!all_roles.empty());
    std::cout << "✓ Find all roles successful\n";

    std::cout << "✓ All Role Repository tests passed\n";
}

void test_department_repository() {
    std::cout << "Testing Department Repository...\n";

    DatabaseManager db("./test_database.db");
    db.initialize();
    DepartmentRepository dept_repo(&db);

    // Create table
    bool table_created = dept_repo.create_table();
    assert(table_created == true);
    std::cout << "✓ Department table creation successful\n";

    // Create a department
    Department dept;
    dept.name = "Test Department";
    dept.description = "A test department for unit testing";

    bool created = dept_repo.create(dept);
    assert(created == true && dept.id > 0);
    std::cout << "✓ Department creation successful\n";

    // Find the department by ID
    auto found_dept = dept_repo.find_by_id(dept.id);
    assert(found_dept != nullptr);
    assert(found_dept->name == dept.name);
    std::cout << "✓ Department retrieval by ID successful\n";

    // Update the department
    dept.description = "Updated test department description";
    bool updated = dept_repo.update(dept);
    assert(updated == true);

    // Verify update
    auto updated_dept = dept_repo.find_by_id(dept.id);
    assert(updated_dept != nullptr);
    assert(updated_dept->description == "Updated test department description");
    std::cout << "✓ Department update successful\n";

    // Find all departments
    auto all_depts = dept_repo.find_all();
    assert(!all_depts.empty());
    std::cout << "✓ Find all departments successful\n";

    std::cout << "✓ All Department Repository tests passed\n";
}

void test_user_repository() {
    std::cout << "Testing User Repository...\n";

    DatabaseManager db("./test_database.db");
    db.initialize();
    UserRepository user_repo(&db);

    // Create table
    bool table_created = user_repo.create_table();
    assert(table_created == true);
    std::cout << "✓ User table creation successful\n";

    // First, create a role and department to link with the user
    RoleRepository role_repo(&db);
    role_repo.create_table();
    Role role;
    role.name = "Test User Role";
    role_repo.create(role);

    DepartmentRepository dept_repo(&db);
    dept_repo.create_table();
    Department dept;
    dept.name = "Test User Dept";
    dept_repo.create(dept);

    // Create a user
    User user;
    user.username = "testuser";
    user.password_hash = "hashed_password";
    user.email = "testuser@example.com";
    user.role_id = role.id;
    user.department_id = dept.id;
    user.first_name = "Test";
    user.last_name = "User";

    bool created = user_repo.create(user);
    assert(created == true && user.id > 0);
    std::cout << "✓ User creation successful\n";

    // Find the user by ID
    auto found_user = user_repo.find_by_id(user.id);
    assert(found_user != nullptr);
    assert(found_user->username == user.username);
    std::cout << "✓ User retrieval by ID successful\n";

    // Find the user by username
    auto found_by_username = user_repo.find_by_username(user.username);
    assert(found_by_username != nullptr);
    assert(found_by_username->id == user.id);
    std::cout << "✓ User retrieval by username successful\n";

    // Find the user by email
    auto found_by_email = user_repo.find_by_email(user.email);
    assert(found_by_email != nullptr);
    assert(found_by_email->id == user.id);
    std::cout << "✓ User retrieval by email successful\n";

    // Update the user
    user.first_name = "Updated Test";
    bool updated = user_repo.update(user);
    assert(updated == true);

    // Verify update
    auto updated_user = user_repo.find_by_id(user.id);
    assert(updated_user != nullptr);
    assert(updated_user->first_name == "Updated Test");
    std::cout << "✓ User update successful\n";

    // Find all users
    auto all_users = user_repo.find_all();
    assert(!all_users.empty());
    std::cout << "✓ Find all users successful\n";

    // Find users by department
    auto users_by_dept = user_repo.find_by_department(dept.id);
    assert(!users_by_dept.empty());
    std::cout << "✓ Find users by department successful\n";

    std::cout << "✓ All User Repository tests passed\n";
}

void test_device_repository() {
    std::cout << "Testing Device Repository...\n";

    DatabaseManager db("./test_database.db");
    db.initialize();
    DeviceRepository device_repo(&db);

    // Create table
    bool table_created = device_repo.create_table();
    assert(table_created == true);
    std::cout << "✓ Device table creation successful\n";

    // Create a device
    Device device;
    device.name = "Test Printer";
    device.device_id = "DEV001";
    device.device_type = "printer";
    device.ip_address = "192.168.1.100";
    device.location = "Office Room 101";
    device.status = "available";

    bool created = device_repo.create(device);
    assert(created == true && device.id > 0);
    std::cout << "✓ Device creation successful\n";

    // Find the device by ID
    auto found_device = device_repo.find_by_id(device.id);
    assert(found_device != nullptr);
    assert(found_device->device_id == device.device_id);
    std::cout << "✓ Device retrieval by ID successful\n";

    // Find the device by device_id
    auto found_by_device_id = device_repo.find_by_device_id(device.device_id);
    assert(found_by_device_id != nullptr);
    assert(found_by_device_id->id == device.id);
    std::cout << "✓ Device retrieval by device_id successful\n";

    // Find the device by IP address
    auto found_by_ip = device_repo.find_by_ip_address(device.ip_address);
    assert(found_by_ip != nullptr);
    assert(found_by_ip->id == device.id);
    std::cout << "✓ Device retrieval by IP address successful\n";

    // Update the device
    device.location = "Updated Location";
    bool updated = device_repo.update(device);
    assert(updated == true);

    // Verify update
    auto updated_device = device_repo.find_by_id(device.id);
    assert(updated_device != nullptr);
    assert(updated_device->location == "Updated Location");
    std::cout << "✓ Device update successful\n";

    // Find all devices
    auto all_devices = device_repo.find_all();
    assert(!all_devices.empty());
    std::cout << "✓ Find all devices successful\n";

    // Find devices by status
    auto devices_by_status = device_repo.find_by_status("available");
    assert(!devices_by_status.empty());
    std::cout << "✓ Find devices by status successful\n";

    // Find devices by device type
    auto devices_by_type = device_repo.find_by_device_type("printer");
    assert(!devices_by_type.empty());
    std::cout << "✓ Find devices by device type successful\n";

    std::cout << "✓ All Device Repository tests passed\n";
}

void test_task_repository() {
    std::cout << "Testing Task Repository...\n";

    DatabaseManager db("./test_database.db");
    db.initialize();
    TaskRepository task_repo(&db);

    // Create table
    bool table_created = task_repo.create_table();
    assert(table_created == true);
    std::cout << "✓ Task table creation successful\n";

    // Create related entities first (user and device)
    UserRepository user_repo(&db);
    user_repo.create_table();
    User user;
    user.username = "task_test_user";
    user.password_hash = "pwd";
    user.email = "task_test@example.com";
    user_repo.create(user);

    DeviceRepository device_repo(&db);
    device_repo.create_table();
    Device device;
    device.name = "Task Test Device";
    device.device_id = "TASK_DEV_001";
    device_repo.create(device);

    // Create a task
    Task task;
    task.title = "Test Task";
    task.description = "A test task for unit testing";
    task.assigned_user_id = user.id;
    task.created_by_user_id = user.id;
    task.device_id = device.id;
    task.status = "pending";
    task.priority = "medium";

    bool created = task_repo.create(task);
    assert(created == true && task.id > 0);
    std::cout << "✓ Task creation successful\n";

    // Find the task by ID
    auto found_task = task_repo.find_by_id(task.id);
    assert(found_task != nullptr);
    assert(found_task->title == task.title);
    std::cout << "✓ Task retrieval by ID successful\n";

    // Update the task
    task.status = "in_progress";
    bool updated = task_repo.update(task);
    assert(updated == true);

    // Verify update
    auto updated_task = task_repo.find_by_id(task.id);
    assert(updated_task != nullptr);
    assert(updated_task->status == "in_progress");
    std::cout << "✓ Task update successful\n";

    // Find all tasks
    auto all_tasks = task_repo.find_all();
    assert(!all_tasks.empty());
    std::cout << "✓ Find all tasks successful\n";

    // Find tasks by assigned user
    auto tasks_by_user = task_repo.find_by_assigned_user(user.id);
    assert(!tasks_by_user.empty());
    std::cout << "✓ Find tasks by assigned user successful\n";

    // Find tasks by status
    auto tasks_by_status = task_repo.find_by_status("in_progress");
    assert(!tasks_by_status.empty());
    std::cout << "✓ Find tasks by status successful\n";

    // Find tasks by priority
    auto tasks_by_priority = task_repo.find_by_priority("medium");
    assert(!tasks_by_priority.empty());
    std::cout << "✓ Find tasks by priority successful\n";

    std::cout << "✓ All Task Repository tests passed\n";
}

int main() {
    std::cout << "Starting Database Unit Tests...\n\n";

    test_database_initialization();
    std::cout << "\n";

    test_role_repository();
    std::cout << "\n";

    test_department_repository();
    std::cout << "\n";

    test_user_repository();
    std::cout << "\n";

    test_device_repository();
    std::cout << "\n";

    test_task_repository();
    std::cout << "\n";

    std::cout << "All Database Unit Tests Passed Successfully!\n";
    std::cout << "Database design and implementation is working correctly.\n";

    return 0;
}