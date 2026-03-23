#include "PersonalHomePage.h"
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
    : QWidget(parent)
{
    setupUI();
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
    m_nameLabel = new QLabel("用户名: admin", this);
    m_roleLabel = new QLabel("角色: 系统管理员", this);
    m_departmentLabel = new QLabel("部门: IT部", this);

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

        // 为按钮添加点击事件，实际实现将在后续添加
        connect(btn, &QPushButton::clicked, [action]() {
            // 这里应该跳转到对应的功能页面
        });
    }

    quickAccessLayout->addStretch();
    m_layout->addWidget(m_quickAccessGroup);
}

void PersonalHomePage::setupRecentActivitiesSection()
{
    m_recentActivitiesGroup = new QGroupBox("最近活动", this);
    QVBoxLayout *activitiesLayout = new QVBoxLayout(m_recentActivitiesGroup);

    m_activitiesTable = new QTableWidget(5, 3, this); // 5行3列的示例表格
    m_activitiesTable->setHorizontalHeaderLabels({"时间", "操作", "详情"});
    m_activitiesTable->horizontalHeader()->setStretchLastSection(true);
    m_activitiesTable->verticalHeader()->setVisible(false);
    m_activitiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_activitiesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 添加示例行数据
    QStringList activities = {
        "提交打印任务",
        "审批刻录申请",
        "查看系统日志",
        "修改个人密码",
        "上传文档"
    };

    for (int i = 0; i < 5; ++i) {
        QTableWidgetItem *timeItem = new QTableWidgetItem(QDateTime::currentDateTime().addDays(-i).toString("MM-dd hh:mm"));
        QTableWidgetItem *actionItem = new QTableWidgetItem(activities[i]);
        QTableWidgetItem *detailItem = new QTableWidgetItem("操作成功完成");

        m_activitiesTable->setItem(i, 0, timeItem);
        m_activitiesTable->setItem(i, 1, actionItem);
        m_activitiesTable->setItem(i, 2, detailItem);
    }

    activitiesLayout->addWidget(m_activitiesTable);
    m_layout->addWidget(m_recentActivitiesGroup);
}

void PersonalHomePage::setupStatisticsSection()
{
    m_statisticsGroup = new QGroupBox("统计信息", this);
    m_statsLayout = new QGridLayout(m_statisticsGroup);

    // 打印统计
    QLabel *printLabel = new QLabel("打印任务:", this);
    m_printCountLabel = new QLabel("15", this);
    m_printProgress = new QProgressBar(this);
    m_printProgress->setValue(65);

    // 刻录统计
    QLabel *burnLabel = new QLabel("刻录任务:", this);
    m_burnCountLabel = new QLabel("8", this);
    m_burnProgress = new QProgressBar(this);
    m_burnProgress->setValue(40);

    // 审批统计
    QLabel *approvalLabel = new QLabel("待审批:", this);
    m_pendingApprovalLabel = new QLabel("3", this);
    m_approvalCountLabel = new QLabel("已审批: 42", this);

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