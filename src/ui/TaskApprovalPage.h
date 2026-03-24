#ifndef TASKAPPROVALPAGE_H
#define TASKAPPROVALPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QCheckBox>
#include <QProgressBar>
#include <QRadioButton>
#include <QButtonGroup>
#include <QList>
#include <QCache>

// Forward declarations
class TaskService;
class Task;

// 用户名缓存（类级别）
class UserNameCache {
private:
    static QCache<int, QString> s_cache;
    static const int MAX_CACHE_SIZE = 500;
public:
    static QString getUserName(int userId);
    static void cacheUserName(int userId, const QString& username);
    static void preloadUserNames();  // 预加载所有用户名
    static void clear();
};

class TaskApprovalPage : public QWidget
{
    Q_OBJECT

public:
    explicit TaskApprovalPage(QWidget *parent = nullptr);

private slots:
    void refreshTasks();
    void refreshTasksPaged();  // 服务端分页版本
    void filterTasks();
    void sortTasks(int column);
    void approveSelected();
    void rejectSelected();
    void batchApprove();
    void batchReject();
    void exportTasks();  // 导出功能

private:
    void setupUI();
    void setupFilterControls();
    void setupTaskList();
    void setupPagination();
    void populateTable(const QList<Task> &tasks);
    void populateTablePaged(const QList<Task> &tasks, int totalCount, int totalPages);
    QString getTaskTypeName(const QString &type) const;
    QString getPriorityName(const QString &priority) const;
    QString getStatusDisplayName(const QString &status) const;
    QString getUserName(int userId) const;
    QString getSortColumnName() const;

    // UI Components
    QVBoxLayout *m_layout;

    // Filter Controls
    QGroupBox *m_filterGroup;
    QHBoxLayout *m_filterLayout;
    QLineEdit *m_searchEdit;
    QComboBox *m_taskTypeCombo;
    QComboBox *m_statusCombo;
    QComboBox *m_priorityCombo;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QPushButton *m_filterButton;
    QPushButton *m_refreshButton;

    // Task List
    QGroupBox *m_taskListGroup;
    QHBoxLayout *m_taskListLayout;
    QTableWidget *m_taskTable;
    QVBoxLayout *m_actionLayout;

    // Action buttons
    QPushButton *m_approveButton;
    QPushButton *m_rejectButton;
    QPushButton *m_batchApproveButton;
    QPushButton *m_batchRejectButton;
    QPushButton *m_exportButton;

    // Pagination
    QGroupBox *m_paginationGroup;
    QHBoxLayout *m_paginationLayout;
    QLabel *m_pageInfoLabel;
    QLabel *m_totalPagesLabel;
    QPushButton *m_prevPageButton;
    QPushButton *m_nextPageButton;
    QSpinBox *m_pageSpinBox;
    QComboBox *m_pageSizeCombo;

    // Internal variables
    int m_currentPage;
    int m_pageSize;
    int m_totalCount;
    int m_sortColumn;
    Qt::SortOrder m_sortOrder;
    QList<Task> m_allTasks;
    QList<Task> m_filteredTasks;
    bool m_useServerSidePagination;  // 使用服务端分页标志
};

#endif // TASKAPPROVALPAGE_H