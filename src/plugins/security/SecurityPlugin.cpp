#include "SecurityPlugin.h"
#include "SensitiveWordsService.h"
#include <QDebug>
#include <QDateTime>

SecurityPlugin::SecurityPlugin() : m_initialized(false) {
}

SecurityPlugin::~SecurityPlugin() {
    if (m_initialized) {
        cleanup();
    }
}

bool SecurityPlugin::initialize() {
    qDebug() << "SecurityPlugin initializing...";
    // 初始化安全插件逻辑
    m_initialized = true;
    return m_initialized;
}

void SecurityPlugin::cleanup() {
    qDebug() << "SecurityPlugin cleaning up...";
    // 清理资源
    m_initialized = false;
}

QString SecurityPlugin::getName() const {
    return "SecurityPlugin";
}

QString SecurityPlugin::getVersion() const {
    return "1.0.0";
}

QString SecurityPlugin::getDescription() const {
    return "安全插件，负责敏感词检查和安全策略";
}

QVariant SecurityPlugin::execute(const QString &function, const QVariantMap &params) {
    if (!m_initialized) {
        return QVariant("Plugin not initialized");
    }

    // 根据功能字符串执行不同操作
    if (function == "checkSensitiveWords") {
        // 检查敏感词
        QString content = params.value("content", "").toString();
        QStringList sensitiveWords = SensitiveWordsService::getInstance().getAllSensitiveWords();

        QVariantMap result;
        result["content"] = content;
        result["hasSensitiveWords"] = false;

        for (const QString &word : sensitiveWords) {
            if (content.contains(word)) {
                result["hasSensitiveWords"] = true;
                result["sensitiveWords"].setValue<QStringList>({word});
                break;
            }
        }

        return result;
    } else if (function == "applySecurityPolicy") {
        // 应用安全策略
        QString userId = params.value("userId", "").toString();
        QString action = params.value("action", "").toString();

        QVariantMap result;
        result["userId"] = userId;
        result["action"] = action;
        result["allowed"] = true; // 默认允许，实际实现中会根据策略判断

        return result;
    } else if (function == "generateAuditLog") {
        // 生成审计日志
        QString userId = params.value("userId", "").toString();
        QString action = params.value("action", "").toString();
        QString resource = params.value("resource", "").toString();

        QVariantMap logEntry;
        logEntry["userId"] = userId;
        logEntry["action"] = action;
        logEntry["resource"] = resource;
        logEntry["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        // 实际实现中会将日志保存到数据库或文件
        qDebug() << "Audit Log:" << logEntry;

        return logEntry;
    }

    return QVariant(QString("Unknown function: %1").arg(function));
}

// 导出插件创建函数
EXPORT_PLUGIN IPlugin* createPlugin() {
    return new SecurityPlugin();
}