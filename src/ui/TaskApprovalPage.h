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
    int mCurrentPage;
    int mPageSize;
    int mTotalCount;
};

#endif // TASKAPPROVALPAGE_H