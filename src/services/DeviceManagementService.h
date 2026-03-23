#ifndef DEVICEMANAGEMENTSERVICE_H
#define DEVICEMANAGEMENTSERVICE_H

#include <QString>
#include <QList>
#include <QVariantMap>
#include "../models/device_model.h"  // 设备模型
#include "../models/user_model.h"    // 用户模型

class DeviceManagementService
{
public:
    // 单例获取方法
    static DeviceManagementService& getInstance();

    // 添加设备
    bool addDevice(const Device &device);

    // 更新设备信息
    bool updateDevice(const Device &device);

    // 删除设备
    bool deleteDevice(int deviceId);

    // 获取设备详情
    Device getDeviceById(int deviceId) const;

    // 获取所有设备
    QList<Device> getAllDevices() const;

    // 根据类型获取设备
    QList<Device> getDevicesByType(const QString &deviceType) const;

    // 搜索设备
    QList<Device> searchDevices(const QString &keyword) const;

    // 分配设备给用户
    bool assignDeviceToUser(int deviceId, int userId);

    // 取消设备分配
    bool unassignDevice(int deviceId);

    // 检查设备状态
    QString getDeviceStatus(int deviceId) const;

    // 获取用户分配的设备
    QList<Device> getDevicesByUser(int userId) const;

    // 更新设备状态
    bool updateDeviceStatus(int deviceId, const QString &status);

    // 检查设备使用权限
    bool checkDevicePermission(int userId, int deviceId) const;

private:
    DeviceManagementService();  // 私有构造函数，确保单例
    ~DeviceManagementService();

    // 记录设备操作日志
    bool logDeviceOperation(int deviceId, int userId, const QString &operation) const;
};

#endif // DEVICEMANAGEMENTSERVICE_H