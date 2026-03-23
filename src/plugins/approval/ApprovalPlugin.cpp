#include "ApprovalPlugin.h"
#include <QDebug>

ApprovalPlugin::ApprovalPlugin() : m_initialized(false) {
}

ApprovalPlugin::~ApprovalPlugin() {
    if (m_initialized) {
        cleanup();
    }
}

bool ApprovalPlugin::initialize() {
    qDebug() << "ApprovalPlugin initializing...";
    // 初始化审批插件逻辑
    m_initialized = true;
    return m_initialized;
}

void ApprovalPlugin::cleanup() {
    qDebug() << "ApprovalPlugin cleaning up...";
    // 清理资源
    m_initialized = false;
}

QString ApprovalPlugin::getName() const {
    return "ApprovalPlugin";
}

QString ApprovalPlugin::getVersion() const {
    return "1.0.0";
}

QString ApprovalPlugin::getDescription() const {
    return "审批插件，负责任务审批流程";
}

QVariant ApprovalPlugin::execute(const QString &function, const QVariantMap &params) {
    if (!m_initialized) {
        return QVariant("Plugin not initialized");
    }

    // 根据功能字符串执行不同操作
    if (function == "submitApproval") {
        // 提交审批请求
        QString taskId = params.value("taskId", "").toString();
        QString approver = params.value("approver", "").toString();
        QVariantMap result;
        result["taskId"] = taskId;
        result["approver"] = approver;
        result["status"] = "submitted";
        return result;
    } else if (function == "approveTask") {
        // 审批通过任务
        QString taskId = params.value("taskId", "").toString();
        QVariantMap result;
        result["taskId"] = taskId;
        result["status"] = "approved";
        return result;
    } else if (function == "rejectTask") {
        // 拒绝任务
        QString taskId = params.value("taskId", "").toString();
        QVariantMap result;
        result["taskId"] = taskId;
        result["status"] = "rejected";
        return result;
    } else if (function == "getPendingApprovals") {
        // 获取待审批任务
        return QVariant(QVariantList());
    }

    return QVariant(QString("Unknown function: %1").arg(function));
}

// 导出插件创建函数
EXPORT_PLUGIN IPlugin* createPlugin() {
    return new ApprovalPlugin();
}