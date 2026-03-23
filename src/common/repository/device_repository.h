#ifndef DEVICE_REPOSITORY_H
#define DEVICE_REPOSITORY_H

#include "../models/device_model.h"
#include "database_manager.h"
#include <vector>
#include <memory>

class DeviceRepository {
private:
    DatabaseManager* db_manager_;

public:
    explicit DeviceRepository(DatabaseManager* db_manager);

    bool create_table();
    std::unique_ptr<Device> find_by_id(int id);
    std::vector<std::unique_ptr<Device>> find_all();
    std::unique_ptr<Device> find_by_device_id(const std::string& device_id);
    std::unique_ptr<Device> find_by_ip_address(const std::string& ip_address);
    bool create(Device& device);
    bool update(const Device& device);
    bool remove(int id);
    std::vector<std::unique_ptr<Device>> find_by_assigned_user(int user_id);
    std::vector<std::unique_ptr<Device>> find_by_status(const std::string& status);
    std::vector<std::unique_ptr<Device>> find_by_device_type(const std::string& device_type);
};

#endif // DEVICE_REPOSITORY_H