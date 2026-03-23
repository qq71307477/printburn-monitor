#include "device_repository.h"

DeviceRepository::DeviceRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool DeviceRepository::create_table() {
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS devices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            device_id TEXT NOT NULL UNIQUE,
            device_type TEXT,
            ip_address TEXT,
            location TEXT,
            assigned_user_id INTEGER,
            status TEXT DEFAULT 'available',
            specifications TEXT,
            is_monitored BOOLEAN DEFAULT 1,
            last_seen DATETIME,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (assigned_user_id) REFERENCES users(id)
        );

        CREATE INDEX IF NOT EXISTS idx_devices_device_id ON devices(device_id);
        CREATE INDEX IF NOT EXISTS idx_devices_assigned_user ON devices(assigned_user_id);
        CREATE INDEX IF NOT EXISTS idx_devices_status ON devices(status);
    )";
    return db_manager_->execute_query(sql);
}

std::unique_ptr<Device> DeviceRepository::find_by_id(int id) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                  "status, specifications, is_monitored FROM devices WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        auto device = std::make_unique<Device>();
        device->id = query.value(0).toInt();
        device->name = query.value(1).toString().toStdString();
        device->device_id = query.value(2).toString().toStdString();
        device->device_type = query.value(3).toString().toStdString();
        device->ip_address = query.value(4).toString().toStdString();
        device->location = query.value(5).toString().toStdString();
        device->assigned_user_id = query.value(6).toInt();
        device->status = query.value(7).toString().toStdString();
        device->specifications = query.value(8).toString().toStdString();
        device->is_monitored = query.value(9).toBool();
        return device;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Device>> DeviceRepository::find_all() {
    std::vector<std::unique_ptr<Device>> devices;
    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                   "status, specifications, is_monitored FROM devices")) {
        while (query.next()) {
            auto device = std::make_unique<Device>();
            device->id = query.value(0).toInt();
            device->name = query.value(1).toString().toStdString();
            device->device_id = query.value(2).toString().toStdString();
            device->device_type = query.value(3).toString().toStdString();
            device->ip_address = query.value(4).toString().toStdString();
            device->location = query.value(5).toString().toStdString();
            device->assigned_user_id = query.value(6).toInt();
            device->status = query.value(7).toString().toStdString();
            device->specifications = query.value(8).toString().toStdString();
            device->is_monitored = query.value(9).toBool();
            devices.push_back(std::move(device));
        }
    }
    return devices;
}

std::unique_ptr<Device> DeviceRepository::find_by_device_id(const std::string& device_id) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                  "status, specifications, is_monitored FROM devices WHERE device_id = ?");
    query.addBindValue(QString::fromStdString(device_id));

    if (query.exec() && query.next()) {
        auto device = std::make_unique<Device>();
        device->id = query.value(0).toInt();
        device->name = query.value(1).toString().toStdString();
        device->device_id = query.value(2).toString().toStdString();
        device->device_type = query.value(3).toString().toStdString();
        device->ip_address = query.value(4).toString().toStdString();
        device->location = query.value(5).toString().toStdString();
        device->assigned_user_id = query.value(6).toInt();
        device->status = query.value(7).toString().toStdString();
        device->specifications = query.value(8).toString().toStdString();
        device->is_monitored = query.value(9).toBool();
        return device;
    }
    return nullptr;
}

std::unique_ptr<Device> DeviceRepository::find_by_ip_address(const std::string& ip_address) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                  "status, specifications, is_monitored FROM devices WHERE ip_address = ?");
    query.addBindValue(QString::fromStdString(ip_address));

    if (query.exec() && query.next()) {
        auto device = std::make_unique<Device>();
        device->id = query.value(0).toInt();
        device->name = query.value(1).toString().toStdString();
        device->device_id = query.value(2).toString().toStdString();
        device->device_type = query.value(3).toString().toStdString();
        device->ip_address = query.value(4).toString().toStdString();
        device->location = query.value(5).toString().toStdString();
        device->assigned_user_id = query.value(6).toInt();
        device->status = query.value(7).toString().toStdString();
        device->specifications = query.value(8).toString().toStdString();
        device->is_monitored = query.value(9).toBool();
        return device;
    }
    return nullptr;
}

bool DeviceRepository::create(Device& device) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO devices (name, device_id, device_type, ip_address, location, assigned_user_id, "
                  "status, specifications, is_monitored) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(QString::fromStdString(device.name));
    query.addBindValue(QString::fromStdString(device.device_id));
    query.addBindValue(QString::fromStdString(device.device_type));
    query.addBindValue(QString::fromStdString(device.ip_address));
    query.addBindValue(QString::fromStdString(device.location));
    query.addBindValue(device.assigned_user_id);
    query.addBindValue(QString::fromStdString(device.status));
    query.addBindValue(QString::fromStdString(device.specifications));
    query.addBindValue(device.is_monitored);

    if (query.exec()) {
        device.id = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

bool DeviceRepository::update(const Device& device) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE devices SET name = ?, device_type = ?, ip_address = ?, location = ?, assigned_user_id = ?, "
                  "status = ?, specifications = ?, is_monitored = ?, last_seen = CURRENT_TIMESTAMP, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(QString::fromStdString(device.name));
    query.addBindValue(QString::fromStdString(device.device_type));
    query.addBindValue(QString::fromStdString(device.ip_address));
    query.addBindValue(QString::fromStdString(device.location));
    query.addBindValue(device.assigned_user_id);
    query.addBindValue(QString::fromStdString(device.status));
    query.addBindValue(QString::fromStdString(device.specifications));
    query.addBindValue(device.is_monitored);
    query.addBindValue(device.id);

    return query.exec();
}

bool DeviceRepository::remove(int id) {
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM devices WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

std::vector<std::unique_ptr<Device>> DeviceRepository::find_by_assigned_user(int user_id) {
    std::vector<std::unique_ptr<Device>> devices;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                  "status, specifications, is_monitored FROM devices WHERE assigned_user_id = ?");
    query.addBindValue(user_id);

    if (query.exec()) {
        while (query.next()) {
            auto device = std::make_unique<Device>();
            device->id = query.value(0).toInt();
            device->name = query.value(1).toString().toStdString();
            device->device_id = query.value(2).toString().toStdString();
            device->device_type = query.value(3).toString().toStdString();
            device->ip_address = query.value(4).toString().toStdString();
            device->location = query.value(5).toString().toStdString();
            device->assigned_user_id = query.value(6).toInt();
            device->status = query.value(7).toString().toStdString();
            device->specifications = query.value(8).toString().toStdString();
            device->is_monitored = query.value(9).toBool();
            devices.push_back(std::move(device));
        }
    }
    return devices;
}

std::vector<std::unique_ptr<Device>> DeviceRepository::find_by_status(const std::string& status) {
    std::vector<std::unique_ptr<Device>> devices;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                  "status, specifications, is_monitored FROM devices WHERE status = ?");
    query.addBindValue(QString::fromStdString(status));

    if (query.exec()) {
        while (query.next()) {
            auto device = std::make_unique<Device>();
            device->id = query.value(0).toInt();
            device->name = query.value(1).toString().toStdString();
            device->device_id = query.value(2).toString().toStdString();
            device->device_type = query.value(3).toString().toStdString();
            device->ip_address = query.value(4).toString().toStdString();
            device->location = query.value(5).toString().toStdString();
            device->assigned_user_id = query.value(6).toInt();
            device->status = query.value(7).toString().toStdString();
            device->specifications = query.value(8).toString().toStdString();
            device->is_monitored = query.value(9).toBool();
            devices.push_back(std::move(device));
        }
    }
    return devices;
}

std::vector<std::unique_ptr<Device>> DeviceRepository::find_by_device_type(const std::string& device_type) {
    std::vector<std::unique_ptr<Device>> devices;
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                  "status, specifications, is_monitored FROM devices WHERE device_type = ?");
    query.addBindValue(QString::fromStdString(device_type));

    if (query.exec()) {
        while (query.next()) {
            auto device = std::make_unique<Device>();
            device->id = query.value(0).toInt();
            device->name = query.value(1).toString().toStdString();
            device->device_id = query.value(2).toString().toStdString();
            device->device_type = query.value(3).toString().toStdString();
            device->ip_address = query.value(4).toString().toStdString();
            device->location = query.value(5).toString().toStdString();
            device->assigned_user_id = query.value(6).toInt();
            device->status = query.value(7).toString().toStdString();
            device->specifications = query.value(8).toString().toStdString();
            device->is_monitored = query.value(9).toBool();
            devices.push_back(std::move(device));
        }
    }
    return devices;
}