#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QLibrary>
#include "IPlugin.h"

class PluginManager : public QObject {
    Q_OBJECT

public:
    static PluginManager& getInstance();

    // Deleted copy constructor and assignment operator
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    // 加载插件
    bool loadPlugin(const QString &pluginPath);

    // 卸载插件
    bool unloadPlugin(const QString &pluginName);

    // 执行插件功能
    QVariant executePlugin(const QString &pluginName, const QString &function, const QVariantMap &params);

    // 获取已加载的插件列表
    QStringList getLoadedPlugins() const;

    // 初始化所有插件
    bool initializeAllPlugins();

    // 清理所有插件
    void cleanupAllPlugins();

private:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    struct PluginData {
        IPlugin *instance = nullptr;
        QLibrary *library = nullptr;
        QString path;
        bool initialized = false;
    };

    QHash<QString, PluginData> m_plugins;
};

#endif // PLUGINMANAGER_H