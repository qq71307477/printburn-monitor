#include "TaskPlugin.h"
#include <QDebug>

TaskPlugin::TaskPlugin() : m_initialized(false) {
}

TaskPlugin::~TaskPlugin() {
    if (m_initialized) {
        cleanup();
    }
}

bool TaskPlugin::initialize() {
    qDebug() << "TaskPlugin initializing...";
    // 初始化任务插件逻辑
    m_initialized = true;
    return m_initialized;
}

void TaskPlugin::cleanup() {
    qDebug() << "TaskPlugin cleaning up...";
    // 清理资源
    m_initialized = false;
}

QString TaskPlugin::getName() const {
    return "TaskPlugin";
}

QString TaskPlugin::getVersion() const {
    return "1.0.0";
}

QString TaskPlugin::getDescription() const {
    return "任务插件，负责打印和刻录任务管理";
}

QVariant TaskPlugin::execute(const QString &function, const QVariantMap &params) {
    if (!m_initialized) {
        return QVariant("Plugin not initialized");
    }

    // 根据功能字符串执行不同操作
    if (function == "createPrintTask") {
        // 创建打印任务
        return QVariant(true);
    } else if (function == "createBurnTask") {
        // 创建刻录任务
        return QVariant(true);
    } else if (function == "getTasks") {
        // 获取任务列表
        return QVariant(QVariantList());
    } else if (function == "updateTaskStatus") {
        // 更新任务状态
        QString taskId = params.value("taskId", "").toString();
        QString status = params.value("status", "").toString();
        QVariantMap result;
        result["taskId"] = taskId;
        result["status"] = status;
        return result;
    }

    return QVariant(QString("Unknown function: %1").arg(function));
}

// 导出插件创建函数
EXPORT_PLUGIN IPlugin* createPlugin() {
    return new TaskPlugin();
}