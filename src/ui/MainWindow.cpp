#include "MainWindow.h"
#include "services/AuthService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QTreeWidget>
#include <QHeaderView>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include "PrintApplicationPage.h"
#include "BurnApplicationPage.h"
#include "PersonalHomePage.h"
#include "PrintOutputPage.h"
#include "BurnOutputPage.h"
#include "TaskApprovalPage.h"
#include "SecurityLevelConfigPage.h"
#include "ApproverConfigPage.h"
#include "SerialNumberConfigPage.h"
#include "BarcodeConfigPage.h"
#include "SystemManagementPage.h"
#include "ProxyApproverConfigPage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_navigationTree(nullptr)
    , m_contentStack(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_homeItem(nullptr)
    , m_printApplyItem(nullptr)
    , m_burnApplyItem(nullptr)
    , m_printOutputItem(nullptr)
    , m_burnOutputItem(nullptr)
    , m_approvalItem(nullptr)
    , m_taskManageItem(nullptr)
    , m_logAuditItem(nullptr)
    , m_adminItem(nullptr)
    , m_securityLevelItem(nullptr)
    , m_approverConfigItem(nullptr)
    , m_serialNumberItem(nullptr)
    , m_barcodeItem(nullptr)
    , m_systemManagementItem(nullptr)
    , m_proxyApproverItem(nullptr)
    , m_currentUser("")
    , m_currentUserRoles()
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("打印刻录安全监控系统");
    resize(1024, 768);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    m_centralWidget = new QWidget(this);
    m_mainLayout = new QVBoxLayout(m_centralWidget);

    QHBoxLayout *mainContentLayout = new QHBoxLayout();

    // 创建导航栏
    setupNavigation();

    // 创建内容区域
    setupContentArea();

    mainContentLayout->addWidget(m_navigationTree, 1);
    mainContentLayout->addWidget(m_contentStack, 4);

    m_mainLayout->addLayout(mainContentLayout);
    setCentralWidget(m_centralWidget);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("文件");
    QMenu *editMenu = menuBar()->addMenu("编辑");
    QMenu *viewMenu = menuBar()->addMenu("视图");
    QMenu *helpMenu = menuBar()->addMenu("帮助");

    QAction *logoutAction = fileMenu->addAction("退出登录");
    connect(logoutAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "退出", "已退出登录");
    });

    QAction *aboutAction = helpMenu->addAction("关于");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "关于", "打印刻录安全监控系统\n版本 1.0.0");
    });
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Main Toolbar");

    QAction *refreshAction = new QAction("刷新", this);
    refreshAction->setIcon(QIcon::fromTheme("view-refresh"));
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshClicked);
    toolBar->addAction(refreshAction);

    QAction *settingsAction = new QAction("设置", this);
    settingsAction->setIcon(QIcon::fromTheme("preferences-system"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    toolBar->addAction(settingsAction);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪");
}

void MainWindow::setupNavigation()
{
    m_navigationTree = new QTreeWidget();
    m_navigationTree->setHeaderHidden(true);
    m_navigationTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_navigationTree->setAnimated(true);

    // 创建导航项（非系统管理项）
    QList<QPair<QString, QString>> navItems = {
        {"首页", "home"},
        {"打印申请", "print_apply"},
        {"刻录申请", "burn_apply"},
        {"打印输出", "print_output"},
        {"刻录输出", "burn_output"},
        {"任务审批", "approval"},
        {"任务管理", "task_manage"},
        {"日志审计", "log_audit"},
        {"密级设置", "security_level"},
        {"审批员配置", "approver_config"},
        {"流水号设置", "serial_number"},
        {"条码设置", "barcode"},
        {"代理审批", "proxy_approver"}
    };

    for (const auto &item : navItems) {
        QTreeWidgetItem *navItem = new QTreeWidgetItem(m_navigationTree);
        navItem->setText(0, item.first);
        navItem->setData(0, Qt::UserRole, item.second);

        if (item.second == "home") m_homeItem = navItem;
        else if (item.second == "print_apply") m_printApplyItem = navItem;
        else if (item.second == "burn_apply") m_burnApplyItem = navItem;
        else if (item.second == "print_output") m_printOutputItem = navItem;
        else if (item.second == "burn_output") m_burnOutputItem = navItem;
        else if (item.second == "approval") m_approvalItem = navItem;
        else if (item.second == "task_manage") m_taskManageItem = navItem;
        else if (item.second == "log_audit") m_logAuditItem = navItem;
        else if (item.second == "security_level") m_securityLevelItem = navItem;
        else if (item.second == "approver_config") m_approverConfigItem = navItem;
        else if (item.second == "serial_number") m_serialNumberItem = navItem;
        else if (item.second == "barcode") m_barcodeItem = navItem;
        else if (item.second == "proxy_approver") m_proxyApproverItem = navItem;
    }

    // 创建系统管理树形节点（带子菜单）
    m_systemManagementItem = new QTreeWidgetItem(m_navigationTree);
    m_systemManagementItem->setText(0, "系统管理");
    m_systemManagementItem->setData(0, Qt::UserRole, "system_management");

    // 添加系统管理子节点
    QTreeWidgetItem *userMgmtItem = new QTreeWidgetItem(m_systemManagementItem);
    userMgmtItem->setText(0, "用户管理");
    userMgmtItem->setData(0, Qt::UserRole, "user_management");

    QTreeWidgetItem *roleMgmtItem = new QTreeWidgetItem(m_systemManagementItem);
    roleMgmtItem->setText(0, "角色管理");
    roleMgmtItem->setData(0, Qt::UserRole, "role_management");

    QTreeWidgetItem *deptMgmtItem = new QTreeWidgetItem(m_systemManagementItem);
    deptMgmtItem->setText(0, "组织架构");
    deptMgmtItem->setData(0, Qt::UserRole, "department_management");

    QTreeWidgetItem *securityPolicyItem = new QTreeWidgetItem(m_systemManagementItem);
    securityPolicyItem->setText(0, "安全策略");
    securityPolicyItem->setData(0, Qt::UserRole, "security_policy");

    // 默认展开系统管理节点
    m_systemManagementItem->setExpanded(true);

    connect(m_navigationTree, &QTreeWidget::itemClicked,
            this, &MainWindow::onNavigationItemSelected);
}

void MainWindow::setupContentArea()
{
    m_contentStack = new QStackedWidget();

    // 创建各个界面
    PersonalHomePage *homePage = new PersonalHomePage(this);
    connect(homePage, &PersonalHomePage::navigateToPage,
            this, &MainWindow::onQuickAccessNavigate);
    m_contentStack->addWidget(homePage);

    // 添加其他界面
    m_contentStack->addWidget(new PrintApplicationPage(this));
    m_contentStack->addWidget(new BurnApplicationPage(this));
    m_contentStack->addWidget(new PrintOutputPage(this));
    m_contentStack->addWidget(new BurnOutputPage(this));
    m_contentStack->addWidget(new TaskApprovalPage(this));
    m_contentStack->addWidget(new SecurityLevelConfigPage(this));
    m_contentStack->addWidget(new ApproverConfigPage(this));
    m_contentStack->addWidget(new SerialNumberConfigPage(this));
    m_contentStack->addWidget(new BarcodeConfigPage(this));
    m_contentStack->addWidget(new ProxyApproverConfigPage(this));
    m_contentStack->addWidget(new SystemManagementPage(this));

    // 默认显示首页
    m_contentStack->setCurrentIndex(0);
}

void MainWindow::onNavigationItemSelected(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    QString navType = item->data(0, Qt::UserRole).toString();

    if (navType == "home") {
        m_contentStack->setCurrentIndex(0);
    } else if (navType == "print_apply") {
        m_contentStack->setCurrentIndex(1);
    } else if (navType == "burn_apply") {
        m_contentStack->setCurrentIndex(2);
    } else if (navType == "print_output") {
        m_contentStack->setCurrentIndex(3);
    } else if (navType == "burn_output") {
        m_contentStack->setCurrentIndex(4);
    } else if (navType == "approval") {
        m_contentStack->setCurrentIndex(5);
    } else if (navType == "security_level") {
        m_contentStack->setCurrentIndex(6);
    } else if (navType == "approver_config") {
        m_contentStack->setCurrentIndex(7);
    } else if (navType == "serial_number") {
        m_contentStack->setCurrentIndex(8);
    } else if (navType == "barcode") {
        m_contentStack->setCurrentIndex(9);
    } else if (navType == "proxy_approver") {
        m_contentStack->setCurrentIndex(10);
    } else if (navType == "system_management") {
        m_contentStack->setCurrentIndex(11);
        // 显示用户管理（第0个子页面）
        static_cast<SystemManagementPage*>(m_contentStack->widget(11))->switchToSubPage(0);
    } else if (navType == "user_management") {
        m_contentStack->setCurrentIndex(11);
        static_cast<SystemManagementPage*>(m_contentStack->widget(11))->switchToSubPage(0);
    } else if (navType == "role_management") {
        m_contentStack->setCurrentIndex(11);
        static_cast<SystemManagementPage*>(m_contentStack->widget(11))->switchToSubPage(1);
    } else if (navType == "department_management") {
        m_contentStack->setCurrentIndex(11);
        static_cast<SystemManagementPage*>(m_contentStack->widget(11))->switchToSubPage(2);
    } else if (navType == "security_policy") {
        m_contentStack->setCurrentIndex(11);
        static_cast<SystemManagementPage*>(m_contentStack->widget(11))->switchToSubPage(3);
    } else if (navType == "task_manage") {
        // 任务管理 - 显示打印输出页面作为任务管理入口
        m_contentStack->setCurrentIndex(3);
        statusBar()->showMessage("任务管理 - 打印输出", 2000);
    } else if (navType == "log_audit") {
        // 日志审计 - 暂未实现
        statusBar()->showMessage("日志审计功能开发中...", 3000);
    }
}

void MainWindow::onQuickAccessNavigate(const QString &pageName)
{
    if (pageName == "打印申请") {
        m_contentStack->setCurrentIndex(1);
    } else if (pageName == "刻录申请") {
        m_contentStack->setCurrentIndex(2);
    } else if (pageName == "任务审批") {
        m_contentStack->setCurrentIndex(5);
    } else if (pageName == "个人信息") {
        statusBar()->showMessage("个人信息页面暂未实现", 3000);
    }
}

void MainWindow::onRefreshClicked()
{
    statusBar()->showMessage("数据已刷新", 2000);
}

void MainWindow::onSettingsClicked()
{
    QMessageBox::information(this, "设置", "设置功能暂未实现");
}

void MainWindow::setCurrentUser(const QString &username, const QStringList &roles)
{
    m_currentUser = username;
    m_currentUserRoles = roles;

    // 根据角色设置导航菜单可见性
    setupNavigationByRole(roles);

    // 更新状态栏显示当前用户
    QString roleDisplay = roles.isEmpty() ? "未知角色" : roles.join(", ");
    statusBar()->showMessage(QString("当前用户: %1 (%2)").arg(username).arg(roleDisplay));
}

void MainWindow::setupNavigationByRole(const QStringList &roles)
{
    AuthService& auth = AuthService::getInstance();

    // 控制导航项的可见性
    if (m_homeItem) m_homeItem->setHidden(!auth.hasPermission("view_home"));
    if (m_printApplyItem) m_printApplyItem->setHidden(!auth.hasPermission("apply_print_task"));
    if (m_burnApplyItem) m_burnApplyItem->setHidden(!auth.hasPermission("apply_burn_task"));
    if (m_printOutputItem) m_printOutputItem->setHidden(!auth.hasPermission("view_my_tasks"));
    if (m_burnOutputItem) m_burnOutputItem->setHidden(!auth.hasPermission("view_my_tasks"));
    if (m_approvalItem) m_approvalItem->setHidden(!auth.hasPermission("approve_tasks"));
    if (m_taskManageItem) m_taskManageItem->setHidden(!auth.hasPermission("manage_tasks"));
    if (m_logAuditItem) m_logAuditItem->setHidden(!auth.hasPermission("view_audit_logs"));
    if (m_securityLevelItem) m_securityLevelItem->setHidden(!auth.hasPermission("manage_security_policies"));
    if (m_approverConfigItem) m_approverConfigItem->setHidden(!auth.hasPermission("manage_approvers"));
    if (m_serialNumberItem) m_serialNumberItem->setHidden(!auth.hasPermission("manage_system_config"));
    if (m_barcodeItem) m_barcodeItem->setHidden(!auth.hasPermission("manage_system_config"));
    if (m_proxyApproverItem) m_proxyApproverItem->setHidden(!auth.hasPermission("manage_proxy_approver"));

    // 系统管理父节点
    bool canManageSystem = auth.hasPermission("manage_users") ||
                             auth.hasPermission("manage_roles") ||
                             auth.hasPermission("manage_departments") ||
                             auth.hasPermission("manage_security_policies");
    if (m_systemManagementItem) m_systemManagementItem->setHidden(!canManageSystem);

    if (canManageSystem) {
        m_systemManagementItem->child(0)->setHidden(!auth.hasPermission("manage_users"));
        m_systemManagementItem->child(1)->setHidden(!auth.hasPermission("manage_roles"));
        m_systemManagementItem->child(2)->setHidden(!auth.hasPermission("manage_departments"));
        m_systemManagementItem->child(3)->setHidden(!auth.hasPermission("manage_security_policies"));
    }
}

