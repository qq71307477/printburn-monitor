#include "BurnOutputPage.h"
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
#include <QRadioButton>
#include <QButtonGroup>

BurnOutputPage::BurnOutputPage(QWidget *parent)
    : QWidget(parent)
{
    mCurrentPage = 1;
    mPageSize = 10;
    mTotalCount = 0;
    setupUI();
}

void BurnOutputPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    QLabel *pageTitle = new QLabel("刻录输出管理", this);
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

void BurnOutputPage::setupFilterControls()
{
    m_filterGroup = new QGroupBox("筛选条件", this);
    m_filterLayout = new QHBoxLayout(m_filterGroup);

    // Search
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索文档标题...");

    // Status filter
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem("全部状态", "");
    m_statusCombo->addItem("待刻录", "pending");
    m_statusCombo->addItem("刻录中", "burning");
    m_statusCombo->addItem("已完成", "completed");
    m_statusCombo->addItem("已取消", "cancelled");
    m_statusCombo->addItem("已拒绝", "rejected");

    // Media type filter
    m_mediaTypeCombo = new QComboBox(this);
    m_mediaTypeCombo->addItem("全部介质", "");
    m_mediaTypeCombo->addItem("CD-R", "cdr");
    m_mediaTypeCombo->addItem("DVD-R", "dvdr");
    m_mediaTypeCombo->addItem("BD-R", "bdr");
    m_mediaTypeCombo->addItem("CD-RW", "cdrw");
    m_mediaTypeCombo->addItem("DVD-RW", "dvdrw");

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

    connect(m_filterButton, &QPushButton::clicked, this, &BurnOutputPage::filterTasks);
    connect(m_refreshButton, &QPushButton::clicked, this, &BurnOutputPage::refreshTasks);

    m_filterLayout->addWidget(new QLabel("搜索:", this));
    m_filterLayout->addWidget(m_searchEdit);
    m_filterLayout->addWidget(new QLabel("状态:", this));
    m_filterLayout->addWidget(m_statusCombo);
    m_filterLayout->addWidget(new QLabel("介质类型:", this));
    m_filterLayout->addWidget(m_mediaTypeCombo);
    m_filterLayout->addWidget(new QLabel("开始日期:", this));
    m_filterLayout->addWidget(m_startDateEdit);
    m_filterLayout->addWidget(new QLabel("结束日期:", this));
    m_filterLayout->addWidget(m_endDateEdit);
    m_filterLayout->addWidget(m_filterButton);
    m_filterLayout->addWidget(m_refreshButton);
    m_filterLayout->addStretch();

    m_layout->addWidget(m_filterGroup);
}

void BurnOutputPage::setupTaskList()
{
    m_taskListGroup = new QGroupBox("刻录任务列表", this);
    m_taskListLayout = new QHBoxLayout(m_taskListGroup);

    // Task table
    m_taskTable = new QTableWidget(0, 8, this);
    m_taskTable->setHorizontalHeaderLabels({"选择", "任务ID", "文档标题", "申请人", "介质类型", "份数", "状态", "申请时间"});
    m_taskTable->horizontalHeader()->setStretchLastSection(true);
    m_taskTable->verticalHeader()->setVisible(false);
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set column widths
    m_taskTable->setColumnWidth(0, 50);  // Selection
    m_taskTable->setColumnWidth(1, 80);  // Task ID
    m_taskTable->setColumnWidth(4, 80);  // Media type
    m_taskTable->setColumnWidth(5, 60);  // Copies
    m_taskTable->setColumnWidth(6, 80);  // Status

    connect(m_taskTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &BurnOutputPage::sortTasks);

    // Action buttons
    m_actionLayout = new QVBoxLayout();

    m_approveButton = new QPushButton("批准选中", this);
    m_rejectButton = new QPushButton("拒绝选中", this);
    m_burnButton = new QPushButton("执行刻录", this);
    m_exportButton = new QPushButton("导出列表", this);

    connect(m_approveButton, &QPushButton::clicked, this, &BurnOutputPage::approveSelected);
    connect(m_rejectButton, &QPushButton::clicked, this, &BurnOutputPage::rejectSelected);
    connect(m_burnButton, &QPushButton::clicked, this, &BurnOutputPage::burnSelected);

    m_actionLayout->addWidget(m_approveButton);
    m_actionLayout->addWidget(m_rejectButton);
    m_actionLayout->addWidget(m_burnButton);
    m_actionLayout->addWidget(m_exportButton);
    m_actionLayout->addStretch();

    m_taskListLayout->addWidget(m_taskTable);
    m_taskListLayout->addLayout(m_actionLayout);

    m_layout->addWidget(m_taskListGroup);
}

void BurnOutputPage::setupPagination()
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
    QLabel *totalPagesLabel = new QLabel(QString::number((mTotalCount + mPageSize - 1) / mPageSize), this);
    m_paginationLayout->addWidget(totalPagesLabel);
    m_paginationLayout->addWidget(m_nextPageButton);
    m_paginationLayout->addWidget(m_pageSizeCombo);

    m_layout->addWidget(m_paginationGroup);
}

void BurnOutputPage::refreshTasks()
{
    // Clear existing items
    m_taskTable->setRowCount(0);

    // Simulate loading data - in real implementation this would fetch from database
    // For demo purposes, we'll create sample data
    for (int i = 0; i < 12; ++i) {
        int row = m_taskTable->rowCount();
        m_taskTable->insertRow(row);

        // Checkbox column
        QCheckBox *checkBox = new QCheckBox();
        m_taskTable->setCellWidget(row, 0, checkBox);

        // Task ID
        m_taskTable->setItem(row, 1, new QTableWidgetItem(QString("BT%1").arg(i+2001)));

        // Document title
        m_taskTable->setItem(row, 2, new QTableWidgetItem(QString("光盘内容 %1").arg(i+1)));

        // Applicant
        m_taskTable->setItem(row, 3, new QTableWidgetItem(QString("申请人 %1").arg(i%5 + 1)));

        // Media type
        QStringList mediaTypes = {"CD-R", "DVD-R", "BD-R", "CD-RW", "DVD-RW"};
        QString mediaType = mediaTypes[i % mediaTypes.length()];
        m_taskTable->setItem(row, 4, new QTableWidgetItem(mediaType));

        // Copies
        m_taskTable->setItem(row, 5, new QTableWidgetItem(QString::number((i % 10) + 1)));

        // Status
        QStringList statuses = {"待刻录", "刻录中", "已完成", "已取消", "已拒绝"};
        QString status = statuses[i % statuses.length()];
        m_taskTable->setItem(row, 6, new QTableWidgetItem(status));

        // Apply time
        QDateTime time = QDateTime::currentDateTime().addDays(-i);
        m_taskTable->setItem(row, 7, new QTableWidgetItem(time.toString("yyyy-MM-dd hh:mm:ss")));
    }

    // Update pagination info
    mTotalCount = 12; // In real implementation, this would come from data source
    m_pageInfoLabel->setText(QString("共 %1 条记录，第 %2/%3 页")
                                .arg(mTotalCount)
                                .arg(mCurrentPage)
                                .arg((mTotalCount + mPageSize - 1) / mPageSize));
    m_pageSpinBox->setMaximum((mTotalCount + mPageSize - 1) / mPageSize);
}

void BurnOutputPage::filterTasks()
{
    // Implementation would filter the tasks based on selected criteria
    refreshTasks(); // For now, just refresh
    QMessageBox::information(this, "提示", "已应用筛选条件");
}

void BurnOutputPage::sortTasks(int column)
{
    // Implementation would sort the tasks based on selected column
    QMessageBox::information(this, "提示", QString("按第 %1 列排序").arg(column));
}

void BurnOutputPage::approveSelected()
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

void BurnOutputPage::rejectSelected()
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

void BurnOutputPage::burnSelected()
{
    int selectedCount = 0;
    for (int i = 0; i < m_taskTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_taskTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            selectedCount++;
        }
    }

    if (selectedCount == 0) {
        QMessageBox::information(this, "提示", "请先选择要刻录的任务");
        return;
    }

    QMessageBox::information(this, "提示", QString("正在执行 %1 个刻录任务").arg(selectedCount));
}