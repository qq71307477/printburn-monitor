#include "device_repository.h"

DeviceRepository::DeviceRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool DeviceRepository::create_table() {
    const char* sql = R"(
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
    const char* sql = "SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored, last_seen, created_at, updated_at FROM devices WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::unique_ptr<Device> device = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        device = std::make_unique<Device>();
        device->id = sqlite3_column_int(stmt, 0);
        device->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        device->device_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        device->device_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        device->ip_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        device->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        device->assigned_user_id = sqlite3_column_int(stmt, 6);
        device->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        device->specifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        device->is_monitored = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* last_seen_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
    }

    sqlite3_finalize(stmt);
    return device;
}

std::vector<std::unique_ptr<Device>> DeviceRepository::find_all() {
    const char* sql = "SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored, last_seen, created_at, updated_at FROM devices";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    std::vector<std::unique_ptr<Device>> devices;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto device = std::make_unique<Device>();
        device->id = sqlite3_column_int(stmt, 0);
        device->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        device->device_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        device->device_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        device->ip_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        device->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        device->assigned_user_id = sqlite3_column_int(stmt, 6);
        device->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        device->specifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        device->is_monitored = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* last_seen_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));

        devices.push_back(std::move(device));
    }

    sqlite3_finalize(stmt);
    return devices;
}

std::unique_ptr<Device> DeviceRepository::find_by_device_id(const std::string& device_id) {
    const char* sql = "SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored, last_seen, created_at, updated_at FROM devices WHERE device_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, device_id.c_str(), -1, SQLITE_STATIC);

    std::unique_ptr<Device> device = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        device = std::make_unique<Device>();
        device->id = sqlite3_column_int(stmt, 0);
        device->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        device->device_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        device->device_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        device->ip_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        device->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        device->assigned_user_id = sqlite3_column_int(stmt, 6);
        device->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        device->specifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        device->is_monitored = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* last_seen_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
    }

    sqlite3_finalize(stmt);
    return device;
}

std::unique_ptr<Device> DeviceRepository::find_by_ip_address(const std::string& ip_address) {
    const char* sql = "SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored, last_seen, created_at, updated_at FROM devices WHERE ip_address = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, ip_address.c_str(), -1, SQLITE_STATIC);

    std::unique_ptr<Device> device = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        device = std::make_unique<Device>();
        device->id = sqlite3_column_int(stmt, 0);
        device->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        device->device_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        device->device_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        device->ip_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        device->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        device->assigned_user_id = sqlite3_column_int(stmt, 6);
        device->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        device->specifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        device->is_monitored = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* last_seen_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
    }

    sqlite3_finalize(stmt);
    return device;
}

bool DeviceRepository::create(Device& device) {
    const char* sql = "INSERT INTO devices (name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, device.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, device.device_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, device.device_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, device.ip_address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, device.location.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, device.assigned_user_id);
    sqlite3_bind_text(stmt, 7, device.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, device.specifications.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, device.is_monitored ? 1 : 0);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        device.id = sqlite3_last_insert_rowid(db_manager_->get_connection());
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

bool DeviceRepository::update(const Device& device) {
    const char* sql = "UPDATE devices SET name = ?, device_type = ?, ip_address = ?, location = ?, assigned_user_id = ?, "
                      "status = ?, specifications = ?, is_monitored = ?, last_seen = CURRENT_TIMESTAMP, "
                      "updated_at = CURRENT_TIMESTAMP WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, device.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, device.device_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, device.ip_address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, device.location.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, device.assigned_user_id);
    sqlite3_bind_text(stmt, 6, device.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, device.specifications.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, device.is_monitored ? 1 : 0);
    sqlite3_bind_int(stmt, 9, device.id);

    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE);

    sqlite3_finalize(stmt);
    return result;
}

bool DeviceRepository::remove(int id) {
    const char* sql = "DELETE FROM devices WHERE id = ?";

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

std::vector<std::unique_ptr<Device>> DeviceRepository::find_by_assigned_user(int user_id) {
    const char* sql = "SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored, last_seen, created_at, updated_at FROM devices WHERE assigned_user_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_int(stmt, 1, user_id);

    std::vector<std::unique_ptr<Device>> devices;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto device = std::make_unique<Device>();
        device->id = sqlite3_column_int(stmt, 0);
        device->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        device->device_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        device->device_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        device->ip_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        device->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        device->assigned_user_id = sqlite3_column_int(stmt, 6);
        device->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        device->specifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        device->is_monitored = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* last_seen_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));

        devices.push_back(std::move(device));
    }

    sqlite3_finalize(stmt);
    return devices;
}

std::vector<std::unique_ptr<Device>> DeviceRepository::find_by_status(const std::string& status) {
    const char* sql = "SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored, last_seen, created_at, updated_at FROM devices WHERE status = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);

    std::vector<std::unique_ptr<Device>> devices;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto device = std::make_unique<Device>();
        device->id = sqlite3_column_int(stmt, 0);
        device->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        device->device_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        device->device_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        device->ip_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        device->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        device->assigned_user_id = sqlite3_column_int(stmt, 6);
        device->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        device->specifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        device->is_monitored = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* last_seen_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));

        devices.push_back(std::move(device));
    }

    sqlite3_finalize(stmt);
    return devices;
}

std::vector<std::unique_ptr<Device>> DeviceRepository::find_by_device_type(const std::string& device_type) {
    const char* sql = "SELECT id, name, device_id, device_type, ip_address, location, assigned_user_id, "
                      "status, specifications, is_monitored, last_seen, created_at, updated_at FROM devices WHERE device_type = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_manager_->get_connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_text(stmt, 1, device_type.c_str(), -1, SQLITE_STATIC);

    std::vector<std::unique_ptr<Device>> devices;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto device = std::make_unique<Device>();
        device->id = sqlite3_column_int(stmt, 0);
        device->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        device->device_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        device->device_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        device->ip_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        device->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        device->assigned_user_id = sqlite3_column_int(stmt, 6);
        device->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        device->specifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        device->is_monitored = sqlite3_column_int(stmt, 9) != 0;

        // Handle datetime columns
        const char* last_seen_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));

        devices.push_back(std::move(device));
    }

    sqlite3_finalize(stmt);
    return devices;
}