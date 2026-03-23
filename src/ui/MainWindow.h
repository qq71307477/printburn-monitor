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
#include "common/PluginManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNavigationItemSelected(QTreeWidgetItem *item, int column);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupNavigation();
    void setupContentArea();

    PluginManager *m_pluginManager;

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
    QTreeWidgetItem *m_adminItem;

    // Current user info
    QString m_currentUser;
    QString m_userRole;
};

#endif // MAINWINDOW_H