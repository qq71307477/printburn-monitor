#include "PluginManager.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent) {
}

PluginManager::~PluginManager() {
    cleanupAllPlugins();

    // 清理所有插件实例和库
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        PluginData &data = it.value();
        if (data.instance) {
            delete data.instance;
            data.instance = nullptr;
        }
        if (data.library) {
            data.library->unload();
            delete data.library;
            data.library = nullptr;
        }
    }
    m_plugins.clear();
}

bool PluginManager::loadPlugin(const QString &pluginPath) {
    QFileInfo fileInfo(pluginPath);
    QString pluginName = fileInfo.baseName();

    // 检查插件是否已加载
    if (m_plugins.contains(pluginName)) {
        qDebug() << "Plugin already loaded:" << pluginName;
        return false;
    }

    QLibrary *library = new QLibrary(pluginPath);
    if (!library->load()) {
        qDebug() << "Failed to load plugin:" << pluginPath << library->errorString();
        delete library;
        return false;
    }

    // 获取创建插件的函数
    CreatePluginFunc createPlugin = (CreatePluginFunc)library->resolve("createPlugin");
    if (!createPlugin) {
        qDebug() << "Cannot resolve createPlugin function in:" << pluginPath << library->errorString();
        library->unload();
        delete library;
        return false;
    }

    // 创建插件实例
    IPlugin *pluginInstance = createPlugin();
    if (!pluginInstance) {
        qDebug() << "Failed to create plugin instance from:" << pluginPath;
        library->unload();
        delete library;
        return false;
    }

    // 初始化插件
    if (!pluginInstance->initialize()) {
        qDebug() << "Failed to initialize plugin:" << pluginName;
        delete pluginInstance;
        library->unload();
        delete library;
        return false;
    }

    // 存储插件信息
    PluginData data;
    data.instance = pluginInstance;
    data.library = library;
    data.path = pluginPath;
    data.initialized = true;

    m_plugins.insert(pluginName, data);

    qDebug() << "Successfully loaded plugin:" << pluginName;
    return true;
}

bool PluginManager::unloadPlugin(const QString &pluginName) {
    auto it = m_plugins.find(pluginName);
    if (it == m_plugins.end()) {
        qDebug() << "Plugin not found:" << pluginName;
        return false;
    }

    PluginData &data = it.value();

    // 清理插件
    if (data.instance) {
        data.instance->cleanup();
        delete data.instance;
        data.instance = nullptr;
    }

    // 卸载库
    if (data.library) {
        data.library->unload();
        delete data.library;
        data.library = nullptr;
    }

    m_plugins.erase(it);
    qDebug() << "Successfully unloaded plugin:" << pluginName;
    return true;
}

QVariant PluginManager::executePlugin(const QString &pluginName, const QString &function, const QVariantMap &params) {
    auto it = m_plugins.find(pluginName);
    if (it == m_plugins.end()) {
        return QVariant(QString("Plugin not found: %1").arg(pluginName));
    }

    PluginData &data = it.value();
    if (!data.instance) {
        return QVariant(QString("Plugin instance is null: %1").arg(pluginName));
    }

    return data.instance->execute(function, params);
}

QStringList PluginManager::getLoadedPlugins() const {
    return m_plugins.keys();
}

bool PluginManager::initializeAllPlugins() {
    bool allSuccess = true;
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        PluginData &data = it.value();
        if (data.instance && !data.initialized) {
            if (!data.instance->initialize()) {
                qDebug() << "Failed to initialize plugin:" << it.key();
                allSuccess = false;
            } else {
                data.initialized = true;
            }
        }
    }
    return allSuccess;
}

void PluginManager::cleanupAllPlugins() {
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        PluginData &data = it.value();
        if (data.instance) {
            data.instance->cleanup();
        }
    }
}