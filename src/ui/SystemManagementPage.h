#ifndef SYSTEMMANAGEMENTPAGE_H
#define SYSTEMMANAGEMENTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

class SystemManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit SystemManagementPage(QWidget* parent = nullptr);
    void switchToSubPage(int index);

private slots:
    void onSubNavItemSelected(int index);

private:
    void setupUI();
    void setupSubNavigation();
    void setupContentPages();

    // UI Components
    QHBoxLayout* m_mainLayout;
    QListWidget* m_subNavList;
    QStackedWidget* m_contentStack;

    // Sub-navigation items
    QListWidgetItem* m_userManagementItem;
    QListWidgetItem* m_roleManagementItem;
    QListWidgetItem* m_departmentManagementItem;
    QListWidgetItem* m_deviceManagementItem;
    QListWidgetItem* m_loginLogItem;
    QListWidgetItem* m_operationLogItem;
    QListWidgetItem* m_securityPolicyItem;

    // Content pages
    QWidget* m_userManagementPage;
    QWidget* m_roleManagementPage;
    QWidget* m_departmentManagementPage;
    QWidget* m_deviceManagementPage;
    QWidget* m_loginLogPage;
    QWidget* m_operationLogPage;
    QWidget* m_securityPolicyPage;
};

#endif // SYSTEMMANAGEMENTPAGE_H
