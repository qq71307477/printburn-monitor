#include "TaskService.h"
#include "src/common/repository/task_repository.h"
#include "src/common/repository/user_repository.h"
#include "src/plugins/security/SecurityPlugin.h"
#include "common/PluginManager.h"
#include <QDateTime>
#include <QUuid>
#include <mutex>

// 静态实例
static std::once_flag onceFlag;
static TaskService* instance = nullptr;

TaskService& TaskService::getInstance()
{
    std::call_once(onceFlag, []() {
        instance = new TaskService();
    });
    return *instance;
}

TaskService::TaskService()
{
}

TaskService::~TaskService()
{
}

bool TaskService::createPrintTask(const User &user, const QString &documentTitle, const QString &purpose,
                                int copies, const QString &filePath, const QString &priority)
{
    // 检查敏感词
    if (checkSensitiveWords(documentTitle + " " + purpose)) {
        // 如果包含敏感词，需要特殊处理或拒绝
        return false;
    }

    TaskRepository taskRepo;

    Task task;
    task.setId(0); // 新任务ID由数据库自动生成
    task.setUserId(user.getId());
    task.setType("PRINT");
    task.setTitle(documentTitle);
    task.setDescription(purpose);
    task.setStatus("PENDING");
    task.setPriority(priority);
    task.setFilePath(filePath);
    task.setCopies(copies);
    task.setCreateTime(QDateTime::currentDateTime());
    task.setUpdateTime(QDateTime::currentDateTime());

    // 设置审批流程
    task.setApproverId(0); // 待分配审批人
    task.setApprovalStatus("PENDING");

    return taskRepo.create(task);
}

bool TaskService::createBurnTask(const User &user, const QString &documentTitle, const QString &purpose,
                                int copies, const QString &mediaType, const QStringList &filePaths,
                                const QString &sessionMode)
{
    // 检查敏感词
    if (checkSensitiveWords(documentTitle + " " + purpose)) {
        // 如果包含敏感词，需要特殊处理或拒绝
        return false;
    }

    TaskRepository taskRepo;

    Task task;
    task.setId(0); // 新任务ID由数据库自动生成
    task.setUserId(user.getId());
    task.setType("BURN");
    task.setTitle(documentTitle);
    task.setDescription(purpose);
    task.setStatus("PENDING");
    task.setPriority("NORMAL");
    task.setMediaType(mediaType);
    task.setSessionMode(sessionMode);
    task.setFilePath(filePaths.join(";")); // 多个文件路径用分号分隔
    task.setCopies(copies);
    task.setCreateTime(QDateTime::currentDateTime());
    task.setUpdateTime(QDateTime::currentDateTime());

    // 设置审批流程
    task.setApproverId(0); // 待分配审批人
    task.setApprovalStatus("PENDING");

    return taskRepo.create(task);
}

QList<Task> TaskService::getUserTasks(const QString &username, const QString &taskType,
                                    const QString &status, int limit, int offset)
{
    UserRepository userRepo;
    User user = userRepo.findByUsername(username);

    TaskRepository taskRepo;
    return taskRepo.findByUserId(user.getId(), taskType, status, limit, offset);
}

QList<Task> TaskService::getTasksByType(const QString &taskType, const QString &status,
                                         int limit, int offset)
{
    TaskRepository taskRepo;
    return taskRepo.findByType(taskType, status, limit, offset);
}

QList<Task> TaskService::getPendingApprovalTasks(const QString &approverRole)
{
    TaskRepository taskRepo;
    return taskRepo.findPendingApprovalTasks(approverRole);
}

bool TaskService::submitApproval(int taskId, const QString &approver, bool approved, const QString &reason)
{
    TaskRepository taskRepo;
    Task task = taskRepo.findById(taskId);

    if (task.getId() <= 0) {
        return false; // 任务不存在
    }

    // 更新审批状态
    task.setApprovalStatus(approved ? "APPROVED" : "REJECTED");
    task.setApprovalTime(QDateTime::currentDateTime());
    task.setApprovalReason(reason);

    // 如果批准，更新任务状态为待执行
    if (approved) {
        task.setStatus("APPROVED");
    } else {
        task.setStatus("REJECTED");
    }

    task.setUpdateTime(QDateTime::currentDateTime());

    bool result = taskRepo.update(task);

    // 记录审批日志
    QVariantMap logParams;
    logParams["userId"] = task.getUserId();
    logParams["action"] = approved ? "APPROVE_TASK" : "REJECT_TASK";
    logParams["resource"] = QString("Task #%1").arg(taskId);
    PluginManager::getInstance().executePlugin("SecurityPlugin", "generateAuditLog", logParams);

    return result;
}

bool TaskService::updateTaskStatus(int taskId, const QString &newStatus)
{
    TaskRepository taskRepo;
    Task task = taskRepo.findById(taskId);

    if (task.getId() <= 0) {
        return false; // 任务不存在
    }

    task.setStatus(newStatus);
    task.setUpdateTime(QDateTime::currentDateTime());

    bool result = taskRepo.update(task);

    // 记录状态变更日志
    QVariantMap logParams;
    logParams["userId"] = task.getUserId();
    logParams["action"] = "UPDATE_TASK_STATUS";
    logParams["resource"] = QString("Task #%1 to %2").arg(taskId).arg(newStatus);
    PluginManager::getInstance().executePlugin("SecurityPlugin", "generateAuditLog", logParams);

    return result;
}

Task TaskService::getTaskById(int taskId) const
{
    TaskRepository taskRepo;
    return taskRepo.findById(taskId);
}

bool TaskService::checkSensitiveWords(const QString &content) const
{
    QVariantMap params;
    params["content"] = content;

    QVariant result = PluginManager::getInstance().executePlugin("SecurityPlugin", "checkSensitiveWords", params);

    if (result.type() == QVariant::Map) {
        QVariantMap resultMap = result.toMap();
        return resultMap.value("hasSensitiveWords", false).toBool();
    }

    // 如果插件执行失败，则保守处理
    return true;
}

QVariantMap TaskService::executeSensitiveCheck(const QString &content) const
{
    QVariantMap params;
    params["content"] = content;

    QVariant result = PluginManager::getInstance().executePlugin("SecurityPlugin", "checkSensitiveWords", params);
    return result.toMap();
}