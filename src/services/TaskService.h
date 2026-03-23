#ifndef TASKSERVICE_H
#define TASKSERVICE_H

#include <QString>
#include <QList>
#include <QVariantMap>
#include "../models/task_model.h"  // 任务模型
#include "../models/user_model.h"  // 用户模型
#include "../models/device_model.h"  // 设备模型

class TaskService
{
public:
    // 单例获取方法
    static TaskService& getInstance();

    // 创建打印任务
    bool createPrintTask(const User &user, const QString &documentTitle, const QString &purpose,
                        int copies, const QString &filePath, const QString &priority);

    // 创建刻录任务
    bool createBurnTask(const User &user, const QString &documentTitle, const QString &purpose,
                       int copies, const QString &mediaType, const QStringList &filePaths,
                       const QString &sessionMode);

    // 获取用户任务列表
    QList<Task> getUserTasks(const QString &username, const QString &taskType = "",
                            const QString &status = "", int limit = -1, int offset = 0);

    // 获取待审批任务
    QList<Task> getPendingApprovalTasks(const QString &approverRole = "");

    // 提交审批
    bool submitApproval(int taskId, const QString &approver, bool approved, const QString &reason = "");

    // 更新任务状态
    bool updateTaskStatus(int taskId, const QString &newStatus);

    // 获取任务详情
    Task getTaskById(int taskId) const;

    // 检查敏感词
    bool checkSensitiveWords(const QString &content) const;

private:
    TaskService();  // 私有构造函数，确保单例
    ~TaskService();

    // 执行敏感词检查
    QVariantMap executeSensitiveCheck(const QString &content) const;
};

#endif // TASKSERVICE_H