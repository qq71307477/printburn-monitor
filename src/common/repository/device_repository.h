#ifndef DEVICE_REPOSITORY_H
#define DEVICE_REPOSITORY_H

#include "device_model.h"
#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <vector>
#include <memory>

class DeviceRepository {
private:
    DatabaseManager* db_manager_;
    static DatabaseManager* default_db_manager_;

public:
    DeviceRepository();
    explicit DeviceRepository(DatabaseManager* db_manager);
    ~DeviceRepository() = default;

    bool create_table();

    // Qt-style methods for services
    Device findById(int id);
    Device findBySerialNumber(const QString& serialNumber);
    QList<Device> findAll();
    bool deleteById(int id);
    bool create(Device& device);
    bool update(const Device& device);
    QList<Device> search(const QString& keyword);
    QList<Device> findByType(const QString& deviceType);
    QList<Device> findByUserId(int userId);

    // Legacy snake_case methods for backward compatibility
    std::unique_ptr<Device> find_by_id(int id);
    std::vector<std::unique_ptr<Device>> find_all();
    std::unique_ptr<Device> find_by_device_id(const std::string& device_id);
    std::unique_ptr<Device> find_by_ip_address(const std::string& ip_address);
    bool remove(int id);
    std::vector<std::unique_ptr<Device>> find_by_assigned_user(int user_id);
    std::vector<std::unique_ptr<Device>> find_by_status(const std::string& status);
    std::vector<std::unique_ptr<Device>> find_by_device_type(const std::string& device_type);
};

#endif // DEVICE_REPOSITORY_H