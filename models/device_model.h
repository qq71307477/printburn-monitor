#ifndef DEVICE_MODEL_H
#define DEVICE_MODEL_H

#include <string>
#include <ctime>

struct Device {
    int id;
    std::string name;
    std::string device_id;  // Unique identifier for the device
    std::string device_type;  // printer, scanner, etc.
    std::string ip_address;
    std::string location;
    int assigned_user_id;  // Currently assigned user
    std::string status;  // "available", "in_use", "maintenance", "offline"
    std::string specifications;  // Technical specs
    bool is_monitored;  // Whether the device is under security monitoring
    std::time_t last_seen;
    std::time_t created_at;
    std::time_t updated_at;

    Device() : id(0), assigned_user_id(0), is_monitored(false) {}
};

#endif // DEVICE_MODEL_H