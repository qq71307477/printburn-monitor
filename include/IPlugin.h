#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QVariant>

// 插件接口基类
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // 插件初始化
    virtual bool initialize() = 0;

    // 插件清理
    virtual void cleanup() = 0;

    // 获取插件名称
    virtual QString getName() const = 0;

    // 获取插件版本
    virtual QString getVersion() const = 0;

    // 获取插件描述
    virtual QString getDescription() const = 0;

    // 执行插件功能
    virtual QVariant execute(const QString &function, const QVariantMap &params) = 0;
};

// 定义插件工厂函数类型
typedef IPlugin* (*CreatePluginFunc)();

// 导出插件创建函数的宏
#ifdef Q_OS_WIN
#define EXPORT_PLUGIN extern "C" __declspec(dllexport)
#else
#define EXPORT_PLUGIN extern "C"
#endif

#endif // IPLUGIN_H