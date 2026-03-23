#include "DeviceManagementService.h"
#include "src/common/repository/device_repository.h"
#include "src/common/repository/user_repository.h"
#include <QDateTime>

// 静态实例
static DeviceManagementService* instance = nullptr;

DeviceManagementService& DeviceManagementService::getInstance()
{
    if (!instance) {
        instance = new DeviceManagementService();
    }
    return *instance;
}

DeviceManagementService::DeviceManagementService()
{
}

DeviceManagementService::~DeviceManagementService()
{
}

bool DeviceManagementService::addDevice(const Device &device)
{
    DeviceRepository deviceRepo;

    // 检查设备编号是否已存在
    Device existingDevice = deviceRepo.findBySerialNumber(device.getSerialNumber());
    if (existingDevice.getId() > 0) {
        return false; // 设备编号已存在
    }

    // 创建副本以传递给 create 方法
    Device newDevice = device;
    return deviceRepo.create(newDevice);
}

bool DeviceManagementService::updateDevice(const Device &device)
{
    DeviceRepository deviceRepo;
    return deviceRepo.update(device);
}

bool DeviceManagementService::deleteDevice(int deviceId)
{
    DeviceRepository deviceRepo;
    return deviceRepo.deleteById(deviceId);
}

Device DeviceManagementService::getDeviceById(int deviceId) const
{
    DeviceRepository deviceRepo;
    return deviceRepo.findById(deviceId);
}

QList<Device> DeviceManagementService::getAllDevices() const
{
    DeviceRepository deviceRepo;
    return deviceRepo.findAll();
}

QList<Device> DeviceManagementService::getDevicesByType(const QString &deviceType) const
{
    DeviceRepository deviceRepo;
    return deviceRepo.findByType(deviceType);
}

QList<Device> DeviceManagementService::searchDevices(const QString &keyword) const
{
    DeviceRepository deviceRepo;
    return deviceRepo.search(keyword);
}

bool DeviceManagementService::assignDeviceToUser(int deviceId, int userId)
{
    DeviceRepository deviceRepo;
    Device device = deviceRepo.findById(deviceId);

    if (device.getId() <= 0) {
        return false; // 设备不存在
    }

    // 检查用户是否存在
    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() <= 0) {
        return false; // 用户不存在
    }

    device.setAssignedUserId(userId);
    device.setAssignmentDate(QDateTime::currentDateTime());
    device.setLastUpdated(QDateTime::currentDateTime());

    bool result = deviceRepo.update(device);

    if (result) {
        logDeviceOperation(deviceId, userId, "ASSIGN");
    }

    return result;
}

bool DeviceManagementService::unassignDevice(int deviceId)
{
    DeviceRepository deviceRepo;
    Device device = deviceRepo.findById(deviceId);

    if (device.getId() <= 0) {
        return false; // 设备不存在
    }

    device.setAssignedUserId(0);
    device.setAssignmentDate(QDateTime()); // 清空分配日期
    device.setLastUpdated(QDateTime::currentDateTime());

    bool result = deviceRepo.update(device);

    if (result) {
        logDeviceOperation(deviceId, device.getAssignedUserId(), "UNASSIGN");
    }

    return result;
}

QString DeviceManagementService::getDeviceStatus(int deviceId) const
{
    Device device = getDeviceById(deviceId);
    return device.getStatus();
}

QList<Device> DeviceManagementService::getDevicesByUser(int userId) const
{
    DeviceRepository deviceRepo;
    return deviceRepo.findByUserId(userId);
}

bool DeviceManagementService::updateDeviceStatus(int deviceId, const QString &status)
{
    DeviceRepository deviceRepo;
    Device device = deviceRepo.findById(deviceId);

    if (device.getId() <= 0) {
        return false; // 设备不存在
    }

    device.setStatus(status);
    device.setLastUpdated(QDateTime::currentDateTime());

    bool result = deviceRepo.update(device);

    if (result) {
        logDeviceOperation(deviceId, device.getAssignedUserId(), "STATUS_UPDATE_" + status.toUpper());
    }

    return result;
}

bool DeviceManagementService::checkDevicePermission(int userId, int deviceId) const
{
    Device device = getDeviceById(deviceId);

    // 如果设备未分配，则任何人都可以使用（如公共设备）
    if (device.getAssignedUserId() <= 0) {
        return true;
    }

    // 如果设备分配给了指定用户，则只有该用户可以使用
    return device.getAssignedUserId() == userId;
}

bool DeviceManagementService::logDeviceOperation(int deviceId, int userId, const QString &operation) const
{
    // TODO: 实际实现中应记录设备操作日志到数据库
    // 这里应该调用日志服务或安全插件来记录操作
    return true;
}