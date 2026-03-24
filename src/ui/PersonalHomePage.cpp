#include "PersonalHomePage.h"
#include "../services/AuthService.h"
#include "../services/TaskService.h"
#include "../services/LogAuditService.h"
#include "../common/repository/user_repository.h"
#include "../common/repository/role_repository.h"
#include "../common/repository/department_repository.h"
#include "../common/repository/task_repository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QGroupBox>
#include <QProgressBar>
#include <QFont>
#include <QPixmap>
#include <QDateTime>

PersonalHomePage::PersonalHomePage(QWidget *parent)
    : QWidget(parent), m_currentUserIdInt(0)
{
    setupUI();
}

void PersonalHomePage::setCurrentUser(const QString &userId)
{
    m_currentUserId = userId;
    m_currentUserIdInt = userId.toInt();

    // 加载用户相关数据
    loadUserInfo();
    loadStatistics();
    loadRecentActivities();
}

void PersonalHomePage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    // 设置页面标题
    QLabel *pageTitle = new QLabel("个人主页", this);
    pageTitle->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_layout->addWidget(pageTitle);

    // 用户信息区域
    setupUserInfoSection();

    // 快速访问区域
    setupQuickAccessSection();

    // 最近活动区域
    setupRecentActivitiesSection();

    // 统计信息区域
    setupStatisticsSection();

    m_layout->addStretch();
}

void PersonalHomePage::setupUserInfoSection()
{
    m_userInfoGroup = new QGroupBox("用户信息", this);
    QHBoxLayout *userInfoLayout = new QHBoxLayout(m_userInfoGroup);

    // 头像
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(80, 80);
    m_avatarLabel->setStyleSheet("border-radius: 40px; background-color: lightgray;");
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setText("头像");

    // 用户详情
    QVBoxLayout *detailsLayout = new QVBoxLayout();
    m_nameLabel = new QLabel("用户名: --", this);
    m_roleLabel = new QLabel("角色: --", this);
    m_departmentLabel = new QLabel("部门: --", this);

    QFont labelFont;
    labelFont.setPointSize(12);
    m_nameLabel->setFont(labelFont);
    m_roleLabel->setFont(labelFont);
    m_departmentLabel->setFont(labelFont);

    detailsLayout->addWidget(m_nameLabel);
    detailsLayout->addWidget(m_roleLabel);
    detailsLayout->addWidget(m_departmentLabel);
    detailsLayout->addStretch();

    userInfoLayout->addWidget(m_avatarLabel);
    userInfoLayout->addLayout(detailsLayout);
    userInfoLayout->addStretch();

    m_layout->addWidget(m_userInfoGroup);
}

void PersonalHomePage::setupQuickAccessSection()
{
    m_quickAccessGroup = new QGroupBox("快速访问", this);
    QHBoxLayout *quickAccessLayout = new QHBoxLayout(m_quickAccessGroup);

    QStringList quickActions = {"打印申请", "刻录申请", "任务审批", "个人信息"};

    for (const QString &action : quickActions) {
        QPushButton *btn = new QPushButton(action, this);
        btn->setFixedSize(100, 60);
        quickAccessLayout->addWidget(btn);

        // 为按钮添加点击事件，发射导航信号
        connect(btn, &QPushButton::clicked, this, [this, action]() {
            emit navigateToPage(action);
        });
    }

    quickAccessLayout->addStretch();
    m_layout->addWidget(m_quickAccessGroup);
}

void PersonalHomePage::setupRecentActivitiesSection()
{
    m_recentActivitiesGroup = new QGroupBox("最近活动", this);
    QVBoxLayout *activitiesLayout = new QVBoxLayout(m_recentActivitiesGroup);

    m_activitiesTable = new QTableWidget(5, 3, this);
    m_activitiesTable->setHorizontalHeaderLabels({"时间", "操作", "详情"});
    m_activitiesTable->horizontalHeader()->setStretchLastSection(true);
    m_activitiesTable->verticalHeader()->setVisible(false);
    m_activitiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_activitiesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    activitiesLayout->addWidget(m_activitiesTable);
    m_layout->addWidget(m_recentActivitiesGroup);
}

void PersonalHomePage::setupStatisticsSection()
{
    m_statisticsGroup = new QGroupBox("统计信息", this);
    m_statsLayout = new QGridLayout(m_statisticsGroup);

    // 打印统计
    QLabel *printLabel = new QLabel("打印任务:", this);
    m_printCountLabel = new QLabel("--", this);
    m_printProgress = new QProgressBar(this);
    m_printProgress->setValue(0);

    // 刻录统计
    QLabel *burnLabel = new QLabel("刻录任务:", this);
    m_burnCountLabel = new QLabel("--", this);
    m_burnProgress = new QProgressBar(this);
    m_burnProgress->setValue(0);

    // 审批统计
    QLabel *approvalLabel = new QLabel("待审批:", this);
    m_pendingApprovalLabel = new QLabel("--", this);
    m_approvalCountLabel = new QLabel("已审批: --", this);

    // 布局
    m_statsLayout->addWidget(printLabel, 0, 0);
    m_statsLayout->addWidget(m_printCountLabel, 0, 1);
    m_statsLayout->addWidget(m_printProgress, 0, 2);

    m_statsLayout->addWidget(burnLabel, 1, 0);
    m_statsLayout->addWidget(m_burnCountLabel, 1, 1);
    m_statsLayout->addWidget(m_burnProgress, 1, 2);

    m_statsLayout->addWidget(approvalLabel, 2, 0);
    m_statsLayout->addWidget(m_pendingApprovalLabel, 2, 1);
    m_statsLayout->addWidget(m_approvalCountLabel, 2, 2);

    m_statsLayout->setColumnStretch(2, 1);
    m_layout->addWidget(m_statisticsGroup);
}

void PersonalHomePage::loadUserInfo()
{
    if (m_currentUserId.isEmpty()) {
        return;
    }

    // 从 AuthService 获取当前登录用户
    AuthService &authService = AuthService::getInstance();
    User currentUser = authService.getCurrentUser();

    // 如果 AuthService 中没有用户信息，从数据库查询
    if (currentUser.getId() <= 0 && m_currentUserIdInt > 0) {
        UserRepository userRepo;
        currentUser = userRepo.findById(m_currentUserIdInt);
    }

    if (currentUser.getId() > 0) {
        // 设置用户名
        m_nameLabel->setText(QString("用户名: %1").arg(currentUser.getUsername()));

        // 获取角色名称
        QString roleName = "--";
        RoleRepository roleRepo;
        QList<Role> roles = roleRepo.findByUserId(currentUser.getId());
        if (!roles.isEmpty()) {
            // 取第一个激活的角色
            for (const Role &role : roles) {
                if (role.isActive()) {
                    roleName = role.getName();
                    break;
                }
            }
        }
        m_roleLabel->setText(QString("角色: %1").arg(roleName));

        // 获取部门名称
        QString departmentName = "--";
        int deptId = currentUser.getDepartmentId();
        if (deptId > 0) {
            DepartmentRepository deptRepo;
            std::unique_ptr<Department> dept = deptRepo.find_by_id(deptId);
            if (dept && dept->id > 0) {
                departmentName = QString::fromStdString(dept->name);
            }
        }
        m_departmentLabel->setText(QString("部门: %1").arg(departmentName));
    }
}

void PersonalHomePage::loadStatistics()
{
    if (m_currentUserId.isEmpty()) {
        return;
    }

    // 获取用户ID
    int userId = m_currentUserIdInt;
    if (userId <= 0) {
        AuthService &authService = AuthService::getInstance();
        User currentUser = authService.getCurrentUser();
        userId = currentUser.getId();
    }

    if (userId <= 0) {
        return;
    }

    TaskRepository taskRepo;

    // 获取打印任务列表
    QList<Task> printTasks = taskRepo.findByUserId(userId, "PRINT");
    int printCount = printTasks.size();
    m_printCountLabel->setText(QString::number(printCount));

    // 计算打印任务完成率
    int printCompleted = 0;
    for (const Task &task : printTasks) {
        if (task.getStatus() == "COMPLETED" || task.getStatus() == "APPROVED") {
            printCompleted++;
        }
    }
    int printProgress = printCount > 0 ? (printCompleted * 100 / printCount) : 0;
    m_printProgress->setValue(printProgress);

    // 获取刻录任务列表
    QList<Task> burnTasks = taskRepo.findByUserId(userId, "BURN");
    int burnCount = burnTasks.size();
    m_burnCountLabel->setText(QString::number(burnCount));

    // 计算刻录任务完成率
    int burnCompleted = 0;
    for (const Task &task : burnTasks) {
        if (task.getStatus() == "COMPLETED" || task.getStatus() == "APPROVED") {
            burnCompleted++;
        }
    }
    int burnProgress = burnCount > 0 ? (burnCompleted * 100 / burnCount) : 0;
    m_burnProgress->setValue(burnProgress);

    // 获取待审批任务数量
    QList<Task> pendingTasks = taskRepo.findPendingApprovalTasks("");
    int pendingCount = pendingTasks.size();
    m_pendingApprovalLabel->setText(QString::number(pendingCount));

    // 计算已审批任务数量（当前用户的任务中被审批的）
    int approvedCount = 0;
    QList<Task> allUserTasks = taskRepo.findByUserId(userId);
    for (const Task &task : allUserTasks) {
        QString approvalStatus = task.getApprovalStatus();
        if (approvalStatus == "APPROVED" || approvalStatus == "REJECTED") {
            approvedCount++;
        }
    }
    m_approvalCountLabel->setText(QString("已审批: %1").arg(approvedCount));
}

void PersonalHomePage::loadRecentActivities()
{
    if (m_currentUserId.isEmpty()) {
        return;
    }

    // 获取用户ID
    int userId = m_currentUserIdInt;
    if (userId <= 0) {
        AuthService &authService = AuthService::getInstance();
        User currentUser = authService.getCurrentUser();
        userId = currentUser.getId();
    }

    if (userId <= 0) {
        return;
    }

    // 从 LogAuditService 获取最近活动日志
    LogAuditService &logService = LogAuditService::getInstance();
    QList<AuditLogEntry> logs = logService.getLogsByUser(userId, QDateTime(), QDateTime(), 5, 0);

    // 清空表格
    m_activitiesTable->setRowCount(logs.size() > 0 ? logs.size() : 5);

    if (logs.isEmpty()) {
        // 如果没有日志数据，显示提示
        for (int i = 0; i < 5; ++i) {
            QTableWidgetItem *timeItem = new QTableWidgetItem("--");
            QTableWidgetItem *actionItem = new QTableWidgetItem("暂无活动记录");
            QTableWidgetItem *detailItem = new QTableWidgetItem("--");

            timeItem->setTextAlignment(Qt::AlignCenter);
            actionItem->setTextAlignment(Qt::AlignCenter);
            detailItem->setTextAlignment(Qt::AlignCenter);

            m_activitiesTable->setItem(i, 0, timeItem);
            m_activitiesTable->setItem(i, 1, actionItem);
            m_activitiesTable->setItem(i, 2, detailItem);
        }
    } else {
        // 填充真实数据
        for (int i = 0; i < logs.size(); ++i) {
            const AuditLogEntry &entry = logs[i];

            // 格式化时间
            QString timeStr = entry.timestamp.toString("MM-dd hh:mm");

            // 事件类型转换为中文显示
            QString actionStr = entry.eventType;
            if (actionStr == "LOGIN") {
                actionStr = "用户登录";
            } else if (actionStr == "LOGOUT") {
                actionStr = "用户登出";
            } else if (actionStr == "CREATE_TASK") {
                actionStr = "创建任务";
            } else if (actionStr == "APPROVE_TASK") {
                actionStr = "审批任务";
            } else if (actionStr == "CHANGE_PASSWORD") {
                actionStr = "修改密码";
            } else if (actionStr == "VIEW_LOG") {
                actionStr = "查看日志";
            }

            // 详情
            QString detailStr = entry.details.isEmpty() ? "操作成功" : entry.details;

            QTableWidgetItem *timeItem = new QTableWidgetItem(timeStr);
            QTableWidgetItem *actionItem = new QTableWidgetItem(actionStr);
            QTableWidgetItem *detailItem = new QTableWidgetItem(detailStr);

            m_activitiesTable->setItem(i, 0, timeItem);
            m_activitiesTable->setItem(i, 1, actionItem);
            m_activitiesTable->setItem(i, 2, detailItem);
        }
    }
}