#include "PrintOutputPage.h"
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
#include <QDateTime>
#include <QTableWidgetItem>
#include <QAction>

PrintOutputPage::PrintOutputPage(QWidget *parent)
    : QWidget(parent)
{
    mCurrentPage = 1;
    mPageSize = 10;
    mTotalCount = 0;
    setupUI();
}

void PrintOutputPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    QLabel *pageTitle = new QLabel("打印输出管理", this);
    pageTitle->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_layout->addWidget(pageTitle);

    // Filter controls
    setupFilterControls();

    // Task list
    setupTaskList();

    // Pagination
    setupPagination();

    m_layout->addStretch();

    // Load initial data
    refreshTasks();
}

void PrintOutputPage::setupFilterControls()
{
    m_filterGroup = new QGroupBox("筛选条件", this);
    m_filterLayout = new QHBoxLayout(m_filterGroup);

    // Search
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索文档标题...");

    // Status filter
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem("全部状态", "");
    m_statusCombo->addItem("待打印", "pending");
    m_statusCombo->addItem("已打印", "completed");
    m_statusCombo->addItem("已取消", "cancelled");
    m_statusCombo->addItem("已拒绝", "rejected");

    // Priority filter
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItem("全部优先级", "");
    m_priorityCombo->addItem("普通", "normal");
    m_priorityCombo->addItem("紧急", "urgent");
    m_priorityCombo->addItem("加急", "expedited");

    // Date range
    m_startDateEdit = new QDateEdit(this);
    m_startDateEdit->setDate(QDate::currentDate().addMonths(-1));
    m_startDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_startDateEdit->setCalendarPopup(true);

    m_endDateEdit = new QDateEdit(this);
    m_endDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_endDateEdit->setCalendarPopup(true);

    m_filterButton = new QPushButton("筛选", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_filterButton, &QPushButton::clicked, this, &PrintOutputPage::filterTasks);
    connect(m_refreshButton, &QPushButton::clicked, this, &PrintOutputPage::refreshTasks);

    m_filterLayout->addWidget(new QLabel("搜索:", this));
    m_filterLayout->addWidget(m_searchEdit);
    m_filterLayout->addWidget(new QLabel("状态:", this));
    m_filterLayout->addWidget(m_statusCombo);
    m_filterLayout->addWidget(new QLabel("优先级:", this));
    m_filterLayout->addWidget(m_priorityCombo);
    m_filterLayout->addWidget(new QLabel("开始日期:", this));
    m_filterLayout->addWidget(m_startDateEdit);
    m_filterLayout->addWidget(new QLabel("结束日期:", this));
    m_filterLayout->addWidget(m_endDateEdit);
    m_filterLayout->addWidget(m_filterButton);
    m_filterLayout->addWidget(m_refreshButton);
    m_filterLayout->addStretch();

    m_layout->addWidget(m_filterGroup);
}

void PrintOutputPage::setupTaskList()
{
    m_taskListGroup = new QGroupBox("打印任务列表", this);
    m_taskListLayout = new QHBoxLayout(m_taskListGroup);

    // Task table
    m_taskTable = new QTableWidget(0, 8, this);
    m_taskTable->setHorizontalHeaderLabels({"选择", "任务ID", "文档标题", "申请人", "优先级", "份数", "状态", "申请时间"});
    m_taskTable->horizontalHeader()->setStretchLastSection(true);
    m_taskTable->verticalHeader()->setVisible(false);
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set column widths
    m_taskTable->setColumnWidth(0, 50);  // Selection
    m_taskTable->setColumnWidth(1, 80);  // Task ID
    m_taskTable->setColumnWidth(4, 80);  // Priority
    m_taskTable->setColumnWidth(5, 60);  // Copies
    m_taskTable->setColumnWidth(6, 80);  // Status

    connect(m_taskTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &PrintOutputPage::sortTasks);

    // Action buttons
    m_actionLayout = new QVBoxLayout();

    m_approveButton = new QPushButton("批准选中", this);
    m_rejectButton = new QPushButton("拒绝选中", this);
    m_printButton = new QPushButton("执行打印", this);
    m_exportButton = new QPushButton("导出列表", this);

    connect(m_approveButton, &QPushButton::clicked, this, &PrintOutputPage::approveSelected);
    connect(m_rejectButton, &QPushButton::clicked, this, &PrintOutputPage::rejectSelected);
    connect(m_printButton, &QPushButton::clicked, this, &PrintOutputPage::printSelected);

    m_actionLayout->addWidget(m_approveButton);
    m_actionLayout->addWidget(m_rejectButton);
    m_actionLayout->addWidget(m_printButton);
    m_actionLayout->addWidget(m_exportButton);
    m_actionLayout->addStretch();

    m_taskListLayout->addWidget(m_taskTable);
    m_taskListLayout->addLayout(m_actionLayout);

    m_layout->addWidget(m_taskListGroup);
}

void PrintOutputPage::setupPagination()
{
    m_paginationGroup = new QGroupBox("分页", this);
    m_paginationLayout = new QHBoxLayout(m_paginationGroup);

    m_pageInfoLabel = new QLabel("共 0 条记录，第 0/0 页", this);
    m_prevPageButton = new QPushButton("上一页", this);
    m_nextPageButton = new QPushButton("下一页", this);
    m_pageSpinBox = new QSpinBox(this);
    m_pageSpinBox->setRange(1, 9999);
    m_pageSpinBox->setValue(mCurrentPage);

    m_pageSizeCombo = new QComboBox(this);
    m_pageSizeCombo->addItem("10 条/页", 10);
    m_pageSizeCombo->addItem("20 条/页", 20);
    m_pageSizeCombo->addItem("50 条/页", 50);
    m_pageSizeCombo->addItem("100 条/页", 100);
    m_pageSizeCombo->setCurrentIndex(0);

    connect(m_prevPageButton, &QPushButton::clicked, [this]() {
        if (mCurrentPage > 1) {
            mCurrentPage--;
            m_pageSpinBox->setValue(mCurrentPage);
            refreshTasks();
        }
    });

    connect(m_nextPageButton, &QPushButton::clicked, [this]() {
        if (mCurrentPage < (mTotalCount + mPageSize - 1) / mPageSize) {
            mCurrentPage++;
            m_pageSpinBox->setValue(mCurrentPage);
            refreshTasks();
        }
    });

    connect(m_pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (value >= 1 && value <= (mTotalCount + mPageSize - 1) / mPageSize) {
            mCurrentPage = value;
            refreshTasks();
        }
    });

    connect(m_pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        mPageSize = m_pageSizeCombo->currentData().toInt();
        mCurrentPage = 1;
        m_pageSpinBox->setValue(mCurrentPage);
        refreshTasks();
    });

    m_paginationLayout->addWidget(m_pageInfoLabel);
    m_paginationLayout->addStretch();
    m_paginationLayout->addWidget(m_prevPageButton);
    m_paginationLayout->addWidget(m_pageSpinBox);
    m_paginationLayout->addWidget(new QLabel("/", this));
    m_totalPagesLabel = new QLabel(QString::number((mTotalCount + mPageSize - 1) / mPageSize), this);
    m_paginationLayout->addWidget(m_totalPagesLabel);
    m_paginationLayout->addWidget(m_nextPageButton);
    m_paginationLayout->addWidget(m_pageSizeCombo);

    m_layout->addWidget(m_paginationGroup);
}

void PrintOutputPage::refreshTasks()
{
    // Clear existing items
    m_taskTable->setRowCount(0);

    // Simulate loading data - in real implementation this would fetch from database
    // For demo purposes, we'll create sample data
    for (int i = 0; i < 15; ++i) {
        int row = m_taskTable->rowCount();
        m_taskTable->insertRow(row);

        // Checkbox column
        QCheckBox *checkBox = new QCheckBox();
        m_taskTable->setCellWidget(row, 0, checkBox);

        // Task ID
        m_taskTable->setItem(row, 1, new QTableWidgetItem(QString("T%1").arg(i+1001)));

        // Document title
        m_taskTable->setItem(row, 2, new QTableWidgetItem(QString("文档标题 %1").arg(i+1)));

        // Applicant
        m_taskTable->setItem(row, 3, new QTableWidgetItem(QString("申请人 %1").arg(i%5 + 1)));

        // Priority
        QString priority = (i % 3 == 0) ? "普通" : (i % 3 == 1 ? "紧急" : "加急");
        m_taskTable->setItem(row, 4, new QTableWidgetItem(priority));

        // Copies
        m_taskTable->setItem(row, 5, new QTableWidgetItem(QString::number((i % 5) + 1)));

        // Status
        QStringList statuses = {"待打印", "已打印", "已取消", "已拒绝"};
        QString status = statuses[i % statuses.length()];
        m_taskTable->setItem(row, 6, new QTableWidgetItem(status));

        // Apply time
        QDateTime time = QDateTime::currentDateTime().addDays(-i);
        m_taskTable->setItem(row, 7, new QTableWidgetItem(time.toString("yyyy-MM-dd hh:mm:ss")));
    }

    // Update pagination info
    mTotalCount = 15; // In real implementation, this would come from data source
    int totalPages = (mTotalCount + mPageSize - 1) / mPageSize;
    m_pageInfoLabel->setText(QString("共 %1 条记录，第 %2/%3 页")
                                .arg(mTotalCount)
                                .arg(mCurrentPage)
                                .arg(totalPages));
    m_totalPagesLabel->setText(QString::number(totalPages));
    m_pageSpinBox->setMaximum(totalPages);
}

void PrintOutputPage::filterTasks()
{
    // Implementation would filter the tasks based on selected criteria
    refreshTasks(); // For now, just refresh
    QMessageBox::information(this, "提示", "已应用筛选条件");
}

void PrintOutputPage::sortTasks(int column)
{
    // Implementation would sort the tasks based on selected column
    QMessageBox::information(this, "提示", QString("按第 %1 列排序").arg(column));
}

void PrintOutputPage::approveSelected()
{
    int selectedCount = 0;
    for (int i = 0; i < m_taskTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_taskTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            selectedCount++;
        }
    }

    if (selectedCount == 0) {
        QMessageBox::information(this, "提示", "请先选择要批准的任务");
        return;
    }

    QMessageBox::information(this, "提示", QString("已批准 %1 个任务").arg(selectedCount));
}

void PrintOutputPage::rejectSelected()
{
    int selectedCount = 0;
    for (int i = 0; i < m_taskTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_taskTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            selectedCount++;
        }
    }

    if (selectedCount == 0) {
        QMessageBox::information(this, "提示", "请先选择要拒绝的任务");
        return;
    }

    QMessageBox::information(this, "提示", QString("已拒绝 %1 个任务").arg(selectedCount));
}

void PrintOutputPage::printSelected()
{
    int selectedCount = 0;
    for (int i = 0; i < m_taskTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_taskTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            selectedCount++;
        }
    }

    if (selectedCount == 0) {
        QMessageBox::information(this, "提示", "请先选择要打印的任务");
        return;
    }

    QMessageBox::information(this, "提示", QString("正在执行 %1 个打印任务").arg(selectedCount));
}