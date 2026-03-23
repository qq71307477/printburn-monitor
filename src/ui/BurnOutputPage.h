#ifndef BURNOUTPUTPAGE_H
#define BURNOUTPUTPAGE_H

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

class BurnOutputPage : public QWidget
{
    Q_OBJECT

public:
    explicit BurnOutputPage(QWidget *parent = nullptr);

private slots:
    void refreshTasks();
    void filterTasks();
    void sortTasks(int column);
    void approveSelected();
    void rejectSelected();
    void burnSelected();

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
    QComboBox *m_statusCombo;
    QComboBox *m_mediaTypeCombo;
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
    QPushButton *m_burnButton;
    QPushButton *m_exportButton;

    // Pagination
    QGroupBox *m_paginationGroup;
    QHBoxLayout *m_paginationLayout;
    QLabel *m_pageInfoLabel;
    QPushButton *m_prevPageButton;
    QPushButton *m_nextPageButton;
    QSpinBox *m_pageSpinBox;
    QComboBox *m_pageSizeCombo;

    // Internal variables
    int mCurrentPage;
    int mPageSize;
    int mTotalCount;
};

#endif // BURNOUTPUTPAGE_H