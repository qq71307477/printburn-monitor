#include "TaskApprovalPage.h"
#include "../services/TaskService.h"
#include "../models/task_model.h"
#include "../common/repository/user_repository.h"
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
#include <QRadioButton>
#include <QButtonGroup>
#include <QtAlgorithms>
#include <QInputDialog>
#include <QApplication>
#include <QFileDialog>
#include <QTextStream>
#include <QProgressDialog>

TaskApprovalPage::TaskApprovalPage(QWidget *parent)
    : QWidget(parent)
    , m_currentPage(1)
    , m_pageSize(10)
    , m_totalCount(0)
    , m_sortColumn(-1)
    , m_sortOrder(Qt::AscendingOrder)
    , m_useServerSidePagination(true)
{
    setupUI();
}

void TaskApprovalPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    QLabel *pageTitle = new QLabel("任务审批管理", this);
    pageTitle->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_layout->addWidget(pageTitle);

    // Filter controls
    setupFilterControls();

    // Task list
    setupTaskList();

    // Pagination
    setupPagination();

    m_layout->addStretch();

    // Load initial data (使用服务端分页)
    refreshTasksPaged();
}

void TaskApprovalPage::setupFilterControls()
{
    m_filterGroup = new QGroupBox("筛选条件", this);
    m_filterLayout = new QHBoxLayout(m_filterGroup);

    // Search
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索任务标题...");

    // Task type filter
    m_taskTypeCombo = new QComboBox(this);
    m_taskTypeCombo->addItem("全部类型", "");
    m_taskTypeCombo->addItem("打印任务", "print");
    m_taskTypeCombo->addItem("刻录任务", "burn");

    // Status filter
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem("全部状态", "");
    m_statusCombo->addItem("待审批", "pending");
    m_statusCombo->addItem("已批准", "approved");
    m_statusCombo->addItem("已拒绝", "rejected");
    m_statusCombo->addItem("已取消", "cancelled");

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

    connect(m_filterButton, &QPushButton::clicked, this, &TaskApprovalPage::filterTasks);
    connect(m_refreshButton, &QPushButton::clicked, this, &TaskApprovalPage::refreshTasks);

    m_filterLayout->addWidget(new QLabel("搜索:", this));
    m_filterLayout->addWidget(m_searchEdit);
    m_filterLayout->addWidget(new QLabel("任务类型:", this));
    m_filterLayout->addWidget(m_taskTypeCombo);
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

void TaskApprovalPage::setupTaskList()
{
    m_taskListGroup = new QGroupBox("待审批任务列表", this);
    m_taskListLayout = new QHBoxLayout(m_taskListGroup);

    // Task table
    m_taskTable = new QTableWidget(0, 9, this);
    m_taskTable->setHorizontalHeaderLabels({"选择", "任务ID", "任务类型", "文档标题", "申请人", "优先级", "状态", "申请时间", "操作"});
    m_taskTable->horizontalHeader()->setStretchLastSection(true);
    m_taskTable->verticalHeader()->setVisible(false);
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set column widths
    m_taskTable->setColumnWidth(0, 50);  // Selection
    m_taskTable->setColumnWidth(1, 80);  // Task ID
    m_taskTable->setColumnWidth(2, 80);  // Task Type
    m_taskTable->setColumnWidth(5, 80);  // Priority
    m_taskTable->setColumnWidth(6, 80);  // Status
    m_taskTable->setColumnWidth(8, 100); // Action

    connect(m_taskTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &TaskApprovalPage::sortTasks);

    // Action buttons
    m_actionLayout = new QVBoxLayout();

    m_approveButton = new QPushButton("批准选中", this);
    m_rejectButton = new QPushButton("拒绝选中", this);
    m_batchApproveButton = new QPushButton("批量批准", this);
    m_batchRejectButton = new QPushButton("批量拒绝", this);
    m_exportButton = new QPushButton("导出列表", this);

    connect(m_approveButton, &QPushButton::clicked, this, &TaskApprovalPage::approveSelected);
    connect(m_rejectButton, &QPushButton::clicked, this, &TaskApprovalPage::rejectSelected);
    connect(m_batchApproveButton, &QPushButton::clicked, this, &TaskApprovalPage::batchApprove);
    connect(m_batchRejectButton, &QPushButton::clicked, this, &TaskApprovalPage::batchReject);
    connect(m_exportButton, &QPushButton::clicked, this, &TaskApprovalPage::exportTasks);

    m_actionLayout->addWidget(m_approveButton);
    m_actionLayout->addWidget(m_rejectButton);
    m_actionLayout->addWidget(m_batchApproveButton);
    m_actionLayout->addWidget(m_batchRejectButton);
    m_actionLayout->addWidget(m_exportButton);
    m_actionLayout->addStretch();

    m_taskListLayout->addWidget(m_taskTable);
    m_taskListLayout->addLayout(m_actionLayout);

    m_layout->addWidget(m_taskListGroup);
}

void TaskApprovalPage::setupPagination()
{
    m_paginationGroup = new QGroupBox("分页", this);
    m_paginationLayout = new QHBoxLayout(m_paginationGroup);

    m_pageInfoLabel = new QLabel("共 0 条记录，第 0/0 页", this);
    m_prevPageButton = new QPushButton("上一页", this);
    m_nextPageButton = new QPushButton("下一页", this);
    m_pageSpinBox = new QSpinBox(this);
    m_pageSpinBox->setRange(1, 9999);
    m_pageSpinBox->setValue(m_currentPage);

    m_pageSizeCombo = new QComboBox(this);
    m_pageSizeCombo->addItem("10 条/页", 10);
    m_pageSizeCombo->addItem("20 条/页", 20);
    m_pageSizeCombo->addItem("50 条/页", 50);
    m_pageSizeCombo->addItem("100 条/页", 100);
    m_pageSizeCombo->setCurrentIndex(0);

    connect(m_prevPageButton, &QPushButton::clicked, [this]() {
        if (m_currentPage > 1) {
            m_currentPage--;
            m_pageSpinBox->setValue(m_currentPage);
            if (m_useServerSidePagination) {
                refreshTasksPaged();
            } else {
                refreshTasks();
            }
        }
    });

    connect(m_nextPageButton, &QPushButton::clicked, [this]() {
        if (m_currentPage < (m_totalCount + m_pageSize - 1) / m_pageSize) {
            m_currentPage++;
            m_pageSpinBox->setValue(m_currentPage);
            if (m_useServerSidePagination) {
                refreshTasksPaged();
            } else {
                refreshTasks();
            }
        }
    });

    connect(m_pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (value >= 1 && value <= (m_totalCount + m_pageSize - 1) / m_pageSize) {
            m_currentPage = value;
            if (m_useServerSidePagination) {
                refreshTasksPaged();
            } else {
                refreshTasks();
            }
        }
    });

    connect(m_pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_pageSize = m_pageSizeCombo->currentData().toInt();
        m_currentPage = 1;
        m_pageSpinBox->setValue(m_currentPage);
        if (m_useServerSidePagination) {
            refreshTasksPaged();
        } else {
            refreshTasks();
        }
    });

    m_paginationLayout->addWidget(m_pageInfoLabel);
    m_paginationLayout->addStretch();
    m_paginationLayout->addWidget(m_prevPageButton);
    m_paginationLayout->addWidget(m_pageSpinBox);
    m_paginationLayout->addWidget(new QLabel("/", this));
    m_totalPagesLabel = new QLabel(QString::number((m_totalCount + m_pageSize - 1) / m_pageSize), this);
    m_paginationLayout->addWidget(m_totalPagesLabel);
    m_paginationLayout->addWidget(m_nextPageButton);
    m_paginationLayout->addWidget(m_pageSizeCombo);

    m_layout->addWidget(m_paginationGroup);
}

void TaskApprovalPage::refreshTasks()
{
    if (m_useServerSidePagination) {
        refreshTasksPaged();
        return;
    }

    // Clear existing items
    m_taskTable->setRowCount(0);

    // Get tasks from TaskService
    TaskService &taskService = TaskService::getInstance();
    m_allTasks = taskService.getPendingApprovalTasks();

    // Store original tasks for filtering
    m_filteredTasks = m_allTasks;
    m_totalCount = m_filteredTasks.size();

    // Apply current sort if any
    if (m_sortColumn >= 0) {
        sortTasks(m_sortColumn);
    } else {
        populateTable(m_filteredTasks);
    }
}

void TaskApprovalPage::refreshTasksPaged()
{
    // 显示加载状态
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Clear existing items
    m_taskTable->setRowCount(0);

    // 使用 TaskRepository 的分页查询
    TaskRepository taskRepo;
    QString searchText = m_searchEdit->text().trimmed();
    QString taskType = m_taskTypeCombo->currentData().toString();
    QString status = m_statusCombo->currentData().toString();
    QString sortBy = getSortColumnName();
    bool sortDesc = (m_sortOrder == Qt::DescendingOrder);

    TaskRepository::PagedResult result = taskRepo.findPendingApprovalTasksPaged(
        "", searchText, taskType, status, m_currentPage, m_pageSize, sortBy, sortDesc);

    m_filteredTasks = result.tasks;
    m_totalCount = result.totalCount;

    populateTablePaged(result.tasks, result.totalCount, result.totalPages);

    // 恢复光标
    QApplication::restoreOverrideCursor();
}

QString TaskApprovalPage::getSortColumnName() const
{
    switch (m_sortColumn) {
    case 1: return "id";
    case 2: return "type";
    case 3: return "title";
    case 5: return "priority";
    case 6: return "approval_status";
    case 7: return "created_at";
    default: return "created_at";
    }
}

void TaskApprovalPage::populateTable(const QList<Task> &tasks)
{
    m_taskTable->setRowCount(0);

    // Calculate pagination
    int startIndex = (m_currentPage - 1) * m_pageSize;
    int endIndex = qMin(startIndex + m_pageSize, tasks.size());

    for (int i = startIndex; i < endIndex; ++i) {
        const Task &task = tasks[i];
        int row = m_taskTable->rowCount();
        m_taskTable->insertRow(row);

        // Checkbox column
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setProperty("taskId", task.getId());
        m_taskTable->setCellWidget(row, 0, checkBox);

        // Task ID
        m_taskTable->setItem(row, 1, new QTableWidgetItem(QString::number(task.getId())));

        // Task type
        m_taskTable->setItem(row, 2, new QTableWidgetItem(getTaskTypeName(task.getType())));

        // Document title
        m_taskTable->setItem(row, 3, new QTableWidgetItem(task.getTitle()));

        // Applicant
        m_taskTable->setItem(row, 4, new QTableWidgetItem(getUserName(task.getUserId())));

        // Priority
        m_taskTable->setItem(row, 5, new QTableWidgetItem(getPriorityName(task.getPriority())));

        // Status
        m_taskTable->setItem(row, 6, new QTableWidgetItem(getStatusDisplayName(task.getApprovalStatus())));

        // Apply time
        m_taskTable->setItem(row, 7, new QTableWidgetItem(task.getCreateTime().toString("yyyy-MM-dd hh:mm:ss")));

        // Action column
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        QPushButton *viewBtn = new QPushButton("查看", this);
        QPushButton *detailBtn = new QPushButton("详情", this);
        actionLayout->addWidget(viewBtn);
        actionLayout->addWidget(detailBtn);
        actionLayout->setContentsMargins(5, 2, 5, 2);
        actionLayout->setSpacing(2);
        actionWidget->setLayout(actionLayout);
        m_taskTable->setCellWidget(row, 8, actionWidget);
    }

    // Update pagination info
    int totalPages = (m_totalCount + m_pageSize - 1) / m_pageSize;
    if (totalPages < 1) totalPages = 1;
    m_pageInfoLabel->setText(QString("共 %1 条记录，第 %2/%3 页")
                                .arg(m_totalCount)
                                .arg(m_currentPage)
                                .arg(totalPages));
    m_totalPagesLabel->setText(QString::number(totalPages));
    m_pageSpinBox->setMaximum(totalPages);
}

void TaskApprovalPage::populateTablePaged(const QList<Task> &tasks, int totalCount, int totalPages)
{
    m_taskTable->setRowCount(0);

    for (int i = 0; i < tasks.size(); ++i) {
        const Task &task = tasks[i];
        int row = m_taskTable->rowCount();
        m_taskTable->insertRow(row);

        // Checkbox column
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setProperty("taskId", task.getId());
        m_taskTable->setCellWidget(row, 0, checkBox);

        // Task ID
        m_taskTable->setItem(row, 1, new QTableWidgetItem(QString::number(task.getId())));

        // Task type
        m_taskTable->setItem(row, 2, new QTableWidgetItem(getTaskTypeName(task.getType())));

        // Document title
        m_taskTable->setItem(row, 3, new QTableWidgetItem(task.getTitle()));

        // Applicant - 使用缓存的用户名
        m_taskTable->setItem(row, 4, new QTableWidgetItem(getUserName(task.getUserId())));

        // Priority
        m_taskTable->setItem(row, 5, new QTableWidgetItem(getPriorityName(task.getPriority())));

        // Status
        m_taskTable->setItem(row, 6, new QTableWidgetItem(getStatusDisplayName(task.getApprovalStatus())));

        // Apply time
        m_taskTable->setItem(row, 7, new QTableWidgetItem(task.getCreateTime().toString("yyyy-MM-dd hh:mm:ss")));

        // Action column
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        QPushButton *viewBtn = new QPushButton("查看", this);
        QPushButton *detailBtn = new QPushButton("详情", this);
        actionLayout->addWidget(viewBtn);
        actionLayout->addWidget(detailBtn);
        actionLayout->setContentsMargins(5, 2, 5, 2);
        actionLayout->setSpacing(2);
        actionWidget->setLayout(actionLayout);
        m_taskTable->setCellWidget(row, 8, actionWidget);
    }

    // Update pagination info
    if (totalPages < 1) totalPages = 1;
    m_pageInfoLabel->setText(QString("共 %1 条记录，第 %2/%3 页")
                                .arg(totalCount)
                                .arg(m_currentPage)
                                .arg(totalPages));
    m_totalPagesLabel->setText(QString::number(totalPages));
    m_pageSpinBox->setMaximum(totalPages);
}

void TaskApprovalPage::filterTasks()
{
    // 重置到第一页并使用服务端分页刷新
    m_currentPage = 1;
    m_pageSpinBox->setValue(1);
    refreshTasksPaged();
}

void TaskApprovalPage::sortTasks(int column)
{
    // Toggle sort order if clicking the same column
    if (m_sortColumn == column) {
        m_sortOrder = (m_sortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        m_sortColumn = column;
        m_sortOrder = Qt::AscendingOrder;
    }

    // 使用服务端分页重新加载数据（排序在服务端完成）
    if (m_useServerSidePagination) {
        refreshTasksPaged();
        return;
    }

    // 旧版本：客户端排序
    std::sort(m_filteredTasks.begin(), m_filteredTasks.end(), [this, column](const Task &a, const Task &b) {
        bool lessThan = false;

        switch (column) {
        case 1: // Task ID
            lessThan = a.getId() < b.getId();
            break;
        case 2: // Task type
            lessThan = a.getType().toLower() < b.getType().toLower();
            break;
        case 3: // Document title
            lessThan = a.getTitle().toLower() < b.getTitle().toLower();
            break;
        case 4: // Applicant
            lessThan = getUserName(a.getUserId()).toLower() < getUserName(b.getUserId()).toLower();
            break;
        case 5: // Priority
            lessThan = a.getPriority().toLower() < b.getPriority().toLower();
            break;
        case 6: // Status
            lessThan = a.getApprovalStatus().toLower() < b.getApprovalStatus().toLower();
            break;
        case 7: // Apply time
            lessThan = a.getCreateTime() < b.getCreateTime();
            break;
        default:
            lessThan = a.getId() < b.getId();
            break;
        }

        return (m_sortOrder == Qt::AscendingOrder) ? lessThan : !lessThan;
    });

    populateTable(m_filteredTasks);
}

void TaskApprovalPage::approveSelected()
{
    QList<int> selectedTaskIds;
    for (int i = 0; i < m_taskTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_taskTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            int taskId = checkBox->property("taskId").toInt();
            selectedTaskIds.append(taskId);
        }
    }

    if (selectedTaskIds.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要批准的任务");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        QString("确定要批准选中的 %1 个任务吗？").arg(selectedTaskIds.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    TaskService &taskService = TaskService::getInstance();
    int successCount = 0;
    int failCount = 0;

    for (int taskId : selectedTaskIds) {
        if (taskService.submitApproval(taskId, "approver", true, "")) {
            successCount++;
        } else {
            failCount++;
        }
    }

    QString message;
    if (failCount == 0) {
        message = QString("已成功批准 %1 个任务").arg(successCount);
    } else {
        message = QString("批准完成：成功 %1 个，失败 %2 个").arg(successCount).arg(failCount);
    }

    QMessageBox::information(this, "提示", message);

    // Refresh the task list
    if (m_useServerSidePagination) {
        refreshTasksPaged();
    } else {
        refreshTasks();
    }
}

void TaskApprovalPage::rejectSelected()
{
    QList<int> selectedTaskIds;
    for (int i = 0; i < m_taskTable->rowCount(); ++i) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_taskTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            int taskId = checkBox->property("taskId").toInt();
            selectedTaskIds.append(taskId);
        }
    }

    if (selectedTaskIds.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要拒绝的任务");
        return;
    }

    // Get rejection reason
    bool ok;
    QString reason = QInputDialog::getText(this, "拒绝原因",
        "请输入拒绝原因：", QLineEdit::Normal, "", &ok);

    if (!ok) {
        return;  // User cancelled
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        QString("确定要拒绝选中的 %1 个任务吗？").arg(selectedTaskIds.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    TaskService &taskService = TaskService::getInstance();
    int successCount = 0;
    int failCount = 0;

    for (int taskId : selectedTaskIds) {
        if (taskService.submitApproval(taskId, "approver", false, reason)) {
            successCount++;
        } else {
            failCount++;
        }
    }

    QString message;
    if (failCount == 0) {
        message = QString("已成功拒绝 %1 个任务").arg(successCount);
    } else {
        message = QString("拒绝完成：成功 %1 个，失败 %2 个").arg(successCount).arg(failCount);
    }

    QMessageBox::information(this, "提示", message);

    // Refresh the task list
    if (m_useServerSidePagination) {
        refreshTasksPaged();
    } else {
        refreshTasks();
    }
}

void TaskApprovalPage::batchApprove()
{
    if (m_filteredTasks.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有可批准的任务");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        QString("确定要批量批准当前筛选结果中的所有 %1 个任务吗？").arg(m_filteredTasks.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    TaskService &taskService = TaskService::getInstance();
    int successCount = 0;
    int failCount = 0;

    for (const Task &task : m_filteredTasks) {
        // Only approve pending tasks
        if (task.getApprovalStatus().toUpper() == "PENDING") {
            if (taskService.submitApproval(task.getId(), "approver", true, "")) {
                successCount++;
            } else {
                failCount++;
            }
        }
    }

    QString message;
    if (failCount == 0) {
        message = QString("已成功批量批准 %1 个任务").arg(successCount);
    } else {
        message = QString("批量批准完成：成功 %1 个，失败 %2 个").arg(successCount).arg(failCount);
    }

    QMessageBox::information(this, "提示", message);

    // Refresh the task list
    if (m_useServerSidePagination) {
        refreshTasksPaged();
    } else {
        refreshTasks();
    }
}

void TaskApprovalPage::batchReject()
{
    if (m_filteredTasks.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有可拒绝的任务");
        return;
    }

    // Get rejection reason
    bool ok;
    QString reason = QInputDialog::getText(this, "拒绝原因",
        "请输入批量拒绝原因：", QLineEdit::Normal, "", &ok);

    if (!ok) {
        return;  // User cancelled
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        QString("确定要批量拒绝当前筛选结果中的所有 %1 个任务吗？").arg(m_filteredTasks.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    TaskService &taskService = TaskService::getInstance();
    int successCount = 0;
    int failCount = 0;

    for (const Task &task : m_filteredTasks) {
        // Only reject pending tasks
        if (task.getApprovalStatus().toUpper() == "PENDING") {
            if (taskService.submitApproval(task.getId(), "approver", false, reason)) {
                successCount++;
            } else {
                failCount++;
            }
        }
    }

    QString message;
    if (failCount == 0) {
        message = QString("已成功批量拒绝 %1 个任务").arg(successCount);
    } else {
        message = QString("批量拒绝完成：成功 %1 个，失败 %2 个").arg(successCount).arg(failCount);
    }

    QMessageBox::information(this, "提示", message);

    // Refresh the task list
    if (m_useServerSidePagination) {
        refreshTasksPaged();
    } else {
        refreshTasks();
    }
}

QString TaskApprovalPage::getTaskTypeName(const QString &type) const
{
    QString upperType = type.toUpper();
    if (upperType == "PRINT") {
        return "打印任务";
    } else if (upperType == "BURN") {
        return "刻录任务";
    } else {
        return type;
    }
}

QString TaskApprovalPage::getPriorityName(const QString &priority) const
{
    QString upperPriority = priority.toUpper();
    if (upperPriority == "NORMAL") {
        return "普通";
    } else if (upperPriority == "URGENT") {
        return "紧急";
    } else if (upperPriority == "EXPEDITED") {
        return "加急";
    } else if (upperPriority == "HIGH") {
        return "高";
    } else if (upperPriority == "LOW") {
        return "低";
    } else {
        return priority;
    }
}

QString TaskApprovalPage::getStatusDisplayName(const QString &status) const
{
    QString upperStatus = status.toUpper();
    if (upperStatus == "PENDING") {
        return "待审批";
    } else if (upperStatus == "APPROVED") {
        return "已批准";
    } else if (upperStatus == "REJECTED") {
        return "已拒绝";
    } else if (upperStatus == "CANCELLED") {
        return "已取消";
    } else {
        return status;
    }
}

QString TaskApprovalPage::getUserName(int userId) const
{
    // 先从缓存获取
    QString cachedName = UserNameCache::getUserName(userId);
    if (!cachedName.isEmpty()) {
        return cachedName;
    }

    // 缓存未命中，查询数据库
    UserRepository userRepo;
    User user = userRepo.findById(userId);
    if (user.getId() > 0) {
        QString username = user.getUsername();
        // 存入缓存
        UserNameCache::cacheUserName(userId, username);
        return username;
    }
    return QString("用户%1").arg(userId);
}

// UserNameCache 静态成员实现
QCache<int, QString> UserNameCache::s_cache(MAX_CACHE_SIZE);

QString UserNameCache::getUserName(int userId)
{
    if (s_cache.contains(userId)) {
        return *s_cache.object(userId);
    }
    return QString();
}

void UserNameCache::cacheUserName(int userId, const QString& username)
{
    s_cache.insert(userId, new QString(username));
}

void UserNameCache::preloadUserNames()
{
    UserRepository userRepo;
    QList<User> users = userRepo.findAll();
    for (const User& user : users) {
        cacheUserName(user.getId(), user.getUsername());
    }
}

void UserNameCache::clear()
{
    s_cache.clear();
}

void TaskApprovalPage::exportTasks()
{
    // 获取保存文件路径
    QString fileName = QFileDialog::getSaveFileName(this, "导出任务列表",
        QString("任务列表_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "CSV 文件 (*.csv);;Excel 文件 (*.xlsx)");

    if (fileName.isEmpty()) {
        return;
    }

    // 显示进度对话框
    QProgressDialog progress("正在导出...", "取消", 0, m_totalCount, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建导出文件");
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);  // 添加 BOM 以支持中文

    // 写入表头
    out << "任务ID,任务类型,文档标题,申请人,优先级,状态,申请时间\n";

    // 使用分页方式导出，避免内存溢出
    int exportedCount = 0;
    int page = 1;
    int pageSize = 100;  // 每次导出100条
    TaskRepository taskRepo;

    while (exportedCount < m_totalCount) {
        if (progress.wasCanceled()) {
            break;
        }

        QString searchText = m_searchEdit->text().trimmed();
        QString taskType = m_taskTypeCombo->currentData().toString();
        QString status = m_statusCombo->currentData().toString();

        TaskRepository::PagedResult result = taskRepo.findPendingApprovalTasksPaged(
            "", searchText, taskType, status, page, pageSize, "created_at", true);

        for (const Task &task : result.tasks) {
            out << QString("%1,%2,%3,%4,%5,%6,%7\n")
                   .arg(task.getId())
                   .arg(getTaskTypeName(task.getType()))
                   .arg(task.getTitle().replace(",", "，"))  // 替换逗号避免CSV格式问题
                   .arg(getUserName(task.getUserId()))
                   .arg(getPriorityName(task.getPriority()))
                   .arg(getStatusDisplayName(task.getApprovalStatus()))
                   .arg(task.getCreateTime().toString("yyyy-MM-dd hh:mm:ss"));

            exportedCount++;
            progress.setValue(exportedCount);

            if (progress.wasCanceled()) {
                break;
            }
        }

        page++;
    }

    file.close();
    progress.setValue(m_totalCount);

    if (!progress.wasCanceled()) {
        QMessageBox::information(this, "导出完成",
            QString("已成功导出 %1 条记录到\n%2").arg(exportedCount).arg(fileName));
    } else {
        QMessageBox::information(this, "导出取消",
            QString("已导出 %1 条记录").arg(exportedCount));
    }
}