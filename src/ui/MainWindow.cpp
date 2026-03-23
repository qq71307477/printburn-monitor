#include "MainWindow.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_pluginManager(new PluginManager(this))
    , m_navigationTree(nullptr)
    , m_contentStack(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_currentUser("")
    , m_userRole("")
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
        // 这里应该触发真正的退出登录逻辑
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
    toolBar->addAction(refreshAction);

    QAction *settingsAction = new QAction("设置", this);
    settingsAction->setIcon(QIcon::fromTheme("preferences-system"));
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

    // 创建导航项
    QList<QPair<QString, QString>> navItems = {
        {"首页", "home"},
        {"打印申请", "print_apply"},
        {"刻录申请", "burn_apply"},
        {"打印输出", "print_output"},
        {"刻录输出", "burn_output"},
        {"任务审批", "approval"},
        {"系统管理", "admin"}
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
        else if (item.second == "admin") m_adminItem = navItem;
    }

    connect(m_navigationTree, &QTreeWidget::itemClicked,
            this, &MainWindow::onNavigationItemSelected);
}

void MainWindow::setupContentArea()
{
    m_contentStack = new QStackedWidget();

    // 创建各个界面
    QLabel *homePage = new QLabel("欢迎使用打印刻录安全监控系统\n请选择左侧菜单项以继续操作");
    homePage->setAlignment(Qt::AlignCenter);
    homePage->setStyleSheet("font-size: 16px; padding: 50px;");
    m_contentStack->addWidget(homePage);

    // 添加其他界面
    m_contentStack->addWidget(new PrintApplicationPage(this));
    m_contentStack->addWidget(new BurnApplicationPage(this));
    m_contentStack->addWidget(new PrintOutputPage(this));
    m_contentStack->addWidget(new BurnOutputPage(this));
    m_contentStack->addWidget(new TaskApprovalPage(this));

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
    }
    // 其他导航项将在后续实现...
}
