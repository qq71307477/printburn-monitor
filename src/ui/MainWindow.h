#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTreeWidget>
#include <QLabel>
#include <QStringList>
#include "common/PluginManager.h"

// Forward declarations for page classes
class SystemManagementPage;

// 内置角色常量定义
namespace RoleNames {
    const QString SYSTEM_ADMIN = QStringLiteral("系统管理员");
    const QString SECURITY_OFFICER = QStringLiteral("安全保密员");
    const QString SECURITY_AUDITOR = QStringLiteral("安全审计员");
    const QString NORMAL_USER = QStringLiteral("普通用户");
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 设置当前登录用户信息
    void setCurrentUser(const QString &username, const QStringList &roles);

private slots:
    void onNavigationItemSelected(QTreeWidgetItem *item, int column);
    void onQuickAccessNavigate(const QString &pageName);
    void onRefreshClicked();
    void onSettingsClicked();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupNavigation();
    void setupContentArea();

    // 根据角色设置导航菜单可见性
    void setupNavigationByRole(const QStringList &roles);



    // UI Components
    QTreeWidget *m_navigationTree;
    QStackedWidget *m_contentStack;
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;

    // Navigation items
    QTreeWidgetItem *m_homeItem;
    QTreeWidgetItem *m_printApplyItem;
    QTreeWidgetItem *m_burnApplyItem;
    QTreeWidgetItem *m_printOutputItem;
    QTreeWidgetItem *m_burnOutputItem;
    QTreeWidgetItem *m_approvalItem;
    QTreeWidgetItem *m_taskManageItem;          // 任务管理
    QTreeWidgetItem *m_logAuditItem;            // 日志审计
    QTreeWidgetItem *m_adminItem;
    QTreeWidgetItem *m_securityLevelItem;
    QTreeWidgetItem *m_approverConfigItem;
    QTreeWidgetItem *m_serialNumberItem;
    QTreeWidgetItem *m_barcodeItem;
    QTreeWidgetItem *m_systemManagementItem;
    QTreeWidgetItem *m_proxyApproverItem;       // 代理审批设置

    // Current user info
    QString m_currentUser;
    QStringList m_currentUserRoles;
};

#endif // MAINWINDOW_H
