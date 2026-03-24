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

// Forward declarations
class TaskService;
class Task;

class TaskApprovalPage : public QWidget
{
    Q_OBJECT

public:
    explicit TaskApprovalPage(QWidget *parent = nullptr);

private slots:
    void refreshTasks();
    void filterTasks();
    void sortTasks(int column);
    void approveSelected();
    void rejectSelected();
    void batchApprove();
    void batchReject();

private:
    void setupUI();
    void setupFilterControls();
    void setupTaskList();
    void setupPagination();
    void populateTable(const QList<Task> &tasks);
    QString getTaskTypeName(const QString &type) const;
    QString getPriorityName(const QString &priority) const;
    QString getStatusDisplayName(const QString &status) const;
    QString getUserName(int userId) const;

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
};

#endif // TASKAPPROVALPAGE_H