#include "SystemManagementPage.h"
#include "UserManagementPage.h"
#include "RoleManagementPage.h"
#include "DepartmentManagementPage.h"
#include "DeviceManagementPage.h"
#include "LoginLogPage.h"
#include "OperationLogPage.h"
#include "SecurityPolicyConfigPage.h"
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>

SystemManagementPage::SystemManagementPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_subNavList(nullptr)
    , m_contentStack(nullptr)
    , m_userManagementItem(nullptr)
    , m_roleManagementItem(nullptr)
    , m_departmentManagementItem(nullptr)
    , m_deviceManagementItem(nullptr)
    , m_loginLogItem(nullptr)
    , m_operationLogItem(nullptr)
    , m_securityPolicyItem(nullptr)
    , m_userManagementPage(nullptr)
    , m_roleManagementPage(nullptr)
    , m_departmentManagementPage(nullptr)
    , m_deviceManagementPage(nullptr)
    , m_loginLogPage(nullptr)
    , m_operationLogPage(nullptr)
    , m_securityPolicyPage(nullptr)
{
    setupUI();
}

void SystemManagementPage::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // Setup left sub-navigation
    setupSubNavigation();

    // Setup right content area
    setupContentPages();

    // Add to main layout
    m_mainLayout->addWidget(m_subNavList, 1);
    m_mainLayout->addWidget(m_contentStack, 4);
}

void SystemManagementPage::setupSubNavigation()
{
    m_subNavList = new QListWidget(this);
    m_subNavList->setMaximumWidth(180);
    m_subNavList->setMinimumWidth(150);
    m_subNavList->setSelectionMode(QAbstractItemView::SingleSelection);

    // Set stylesheet for better appearance
    m_subNavList->setStyleSheet(
        "QListWidget {"
        "  border: 1px solid #d0d0d0;"
        "  border-top: none;"
        "  border-bottom: none;"
        "  border-left: none;"
        "  background-color: #f5f5f5;"
        "}"
        "QListWidget::item {"
        "  padding: 12px 15px;"
        "  border-bottom: 1px solid #e0e0e0;"
        "  color: #333;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #e3f2fd;"
        "  color: #1976d2;"
        "  font-weight: bold;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #e8f4f8;"
        "}"
    );

    // Create sub-navigation items
    m_userManagementItem = new QListWidgetItem("用户管理", m_subNavList);
    m_userManagementItem->setData(Qt::UserRole, "user_management");

    m_roleManagementItem = new QListWidgetItem("角色管理", m_subNavList);
    m_roleManagementItem->setData(Qt::UserRole, "role_management");

    m_departmentManagementItem = new QListWidgetItem("组织架构", m_subNavList);
    m_departmentManagementItem->setData(Qt::UserRole, "department_management");

    m_deviceManagementItem = new QListWidgetItem("设备管理", m_subNavList);
    m_deviceManagementItem->setData(Qt::UserRole, "device_management");

    m_loginLogItem = new QListWidgetItem("登录日志", m_subNavList);
    m_loginLogItem->setData(Qt::UserRole, "login_log");

    m_operationLogItem = new QListWidgetItem("操作日志", m_subNavList);
    m_operationLogItem->setData(Qt::UserRole, "operation_log");

    m_securityPolicyItem = new QListWidgetItem("安全策略", m_subNavList);
    m_securityPolicyItem->setData(Qt::UserRole, "security_policy");

    // Connect selection signal
    connect(m_subNavList, &QListWidget::currentRowChanged,
            this, &SystemManagementPage::onSubNavItemSelected);

    // Select first item by default
    m_subNavList->setCurrentRow(0);
}

void SystemManagementPage::setupContentPages()
{
    m_contentStack = new QStackedWidget(this);

    // Create pages for each sub-module

    // User Management Page
    m_userManagementPage = new UserManagementPage(this);
    m_contentStack->addWidget(m_userManagementPage);

    // Role Management Page
    m_roleManagementPage = new RoleManagementPage(this);
    m_contentStack->addWidget(m_roleManagementPage);

    // Department Management Page
    m_departmentManagementPage = new DepartmentManagementPage(this);
    m_contentStack->addWidget(m_departmentManagementPage);

    // Device Management Page
    m_deviceManagementPage = new DeviceManagementPage(this);
    m_contentStack->addWidget(m_deviceManagementPage);

    // Login Log Page
    m_loginLogPage = new LoginLogPage(this);
    m_contentStack->addWidget(m_loginLogPage);

    // Operation Log Page
    m_operationLogPage = new OperationLogPage(this);
    m_contentStack->addWidget(m_operationLogPage);

    // Security Policy Page
    m_securityPolicyPage = new SecurityPolicyConfigPage(this);
    m_contentStack->addWidget(m_securityPolicyPage);
}

void SystemManagementPage::onSubNavItemSelected(int index)
{
    if (index >= 0 && index < m_contentStack->count()) {
        m_contentStack->setCurrentIndex(index);
    }
}

void SystemManagementPage::switchToSubPage(int index)
{
    if (index >= 0 && index < m_subNavList->count()) {
        m_subNavList->setCurrentRow(index);
        m_contentStack->setCurrentIndex(index);
    }
}
