#include "ClientPlugin.h"
#include <QDebug>

ClientPlugin::ClientPlugin() : m_initialized(false) {
}

ClientPlugin::~ClientPlugin() {
    if (m_initialized) {
        cleanup();
    }
}

bool ClientPlugin::initialize() {
    qDebug() << "ClientPlugin initializing...";
    // 初始化客户端插件逻辑
    m_initialized = true;
    return m_initialized;
}

void ClientPlugin::cleanup() {
    qDebug() << "ClientPlugin cleaning up...";
    // 清理资源
    m_initialized = false;
}

QString ClientPlugin::getName() const {
    return "ClientPlugin";
}

QString ClientPlugin::getVersion() const {
    return "1.0.0";
}

QString ClientPlugin::getDescription() const {
    return "客户端插件，负责界面和用户交互";
}

QVariant ClientPlugin::execute(const QString &function, const QVariantMap &params) {
    if (!m_initialized) {
        return QVariant("Plugin not initialized");
    }

    // 根据功能字符串执行不同操作
    if (function == "showLoginDialog") {
        // 显示登录对话框的逻辑
        return QVariant(true);
    } else if (function == "showMainInterface") {
        // 显示主界面的逻辑
        return QVariant(true);
    } else if (function == "getUserInfo") {
        // 获取用户信息
        QVariantMap userInfo;
        userInfo["name"] = params.value("name", "Unknown");
        return userInfo;
    }

    return QVariant(QString("Unknown function: %1").arg(function));
}

// 导出插件创建函数
EXPORT_PLUGIN IPlugin* createPlugin() {
    return new ClientPlugin();
}