#include "PrintOutputPage.h"
#include "../services/TaskService.h"
#include "../services/AuthService.h"
#include "../services/SerialNumberService.h"
#include "../services/BarcodeService.h"
#include "../common/repository/task_repository.h"
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
#include <QDebug>

PrintOutputPage::PrintOutputPage(QWidget *parent)
    : QWidget(parent)
    , m_currentPage(1)
    , m_pageSize(10)
    , m_totalCount(0)
{
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
            refreshTasks();
        }
    });

    connect(m_nextPageButton, &QPushButton::clicked, [this]() {
        if (m_currentPage < (m_totalCount + m_pageSize - 1) / m_pageSize) {
            m_currentPage++;
            m_pageSpinBox->setValue(m_currentPage);
            refreshTasks();
        }
    });

    connect(m_pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (value >= 1 && value <= (m_totalCount + m_pageSize - 1) / m_pageSize) {
            m_currentPage = value;
            refreshTasks();
        }
    });

    connect(m_pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_pageSize = m_pageSizeCombo->currentData().toInt();
        m_currentPage = 1;
        m_pageSpinBox->setValue(m_currentPage);
        refreshTasks();
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

void PrintOutputPage::refreshTasks()
{
    loadTasksFromDatabase();
}

void PrintOutputPage::loadTasksFromDatabase()
{
    // Clear existing items
    m_taskTable->setRowCount(0);
    m_taskList.clear();

    // Get filter values
    QString statusFilter = m_statusCombo->currentData().toString();
    QString priorityFilter = m_priorityCombo->currentData().toString();
    QString searchText = m_searchEdit->text().trimmed();
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();

    // Get task list from TaskService
    TaskService &taskService = TaskService::getInstance();

    // Get all print tasks (type = "PRINT")
    int offset = (m_currentPage - 1) * m_pageSize;
    QString statusQueryParam = statusFilter.isEmpty() ? QString() : statusFilter.toUpper();
    QList<Task> allTasks = taskService.getTasksByType("PRINT", statusQueryParam, m_pageSize, offset);

    // Store all tasks for filtering
    m_taskList = allTasks;

    // Populate table with task data
    for (const Task &task : m_taskList) {
        // Apply client-side filters
        if (!searchText.isEmpty() && !task.getTitle().contains(searchText, Qt::CaseInsensitive)) {
            continue;
        }

        if (!priorityFilter.isEmpty()) {
            QString taskPriority = task.getPriority().toLower();
            if (priorityFilter.toLower() == "normal" && taskPriority != "normal") continue;
            if (priorityFilter.toLower() == "urgent" && taskPriority != "urgent") continue;
            if (priorityFilter.toLower() == "expedited" && taskPriority != "expedited") continue;
        }

        // Date filter
        QDateTime createTime = task.getCreateTime();
        if (createTime.date() < startDate || createTime.date() > endDate) {
            continue;
        }

        int row = m_taskTable->rowCount();
        m_taskTable->insertRow(row);

        // Checkbox column
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setProperty("taskId", task.getId());
        m_taskTable->setCellWidget(row, 0, checkBox);

        // Task ID
        m_taskTable->setItem(row, 1, new QTableWidgetItem(QString::number(task.getId())));

        // Document title
        m_taskTable->setItem(row, 2, new QTableWidgetItem(task.getTitle()));

        // Applicant - get from user info, for now use user ID
        m_taskTable->setItem(row, 3, new QTableWidgetItem(QString("用户%1").arg(task.getUserId())));

        // Priority
        m_taskTable->setItem(row, 4, new QTableWidgetItem(priorityToDisplay(task.getPriority())));

        // Copies
        m_taskTable->setItem(row, 5, new QTableWidgetItem(QString::number(task.getCopies())));

        // Status
        m_taskTable->setItem(row, 6, new QTableWidgetItem(statusToDisplay(task.getStatus())));

        // Apply time
        m_taskTable->setItem(row, 7, new QTableWidgetItem(createTime.toString("yyyy-MM-dd hh:mm:ss")));
    }

    // Update pagination info
    m_totalCount = m_taskList.size();
    int totalPages = (m_totalCount + m_pageSize - 1) / m_pageSize;
    if (totalPages < 1) totalPages = 1;
    m_pageInfoLabel->setText(QString("共 %1 条记录，第 %2/%3 页")
                                .arg(m_totalCount)
                                .arg(m_currentPage)
                                .arg(totalPages));
    m_totalPagesLabel->setText(QString::number(totalPages));
    m_pageSpinBox->setMaximum(totalPages);
}

QString PrintOutputPage::statusToDisplay(const QString &status) const
{
    QString statusUpper = status.toUpper();
    if (statusUpper == "PENDING") return "待打印";
    if (statusUpper == "APPROVED") return "已批准";
    if (statusUpper == "IN_PROGRESS") return "打印中";
    if (statusUpper == "COMPLETED") return "已打印";
    if (statusUpper == "FAILED") return "执行失败";
    if (statusUpper == "CANCELLED") return "已取消";
    if (statusUpper == "REJECTED") return "已拒绝";
    return status;
}

QString PrintOutputPage::priorityToDisplay(const QString &priority) const
{
    QString priorityLower = priority.toLower();
    if (priorityLower == "normal") return "普通";
    if (priorityLower == "urgent") return "紧急";
    if (priorityLower == "expedited") return "加急";
    return priority;
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

    // Get current user for approval
    AuthService &authService = AuthService::getInstance();
    User currentUser = authService.getCurrentUser();
    QString approver = currentUser.getUsername().isEmpty() ? "admin" : currentUser.getUsername();

    TaskService &taskService = TaskService::getInstance();
    int successCount = 0;

    for (int taskId : selectedTaskIds) {
        if (taskService.submitApproval(taskId, approver, true, "批量批准")) {
            successCount++;
        }
    }

    QMessageBox::information(this, "提示", QString("已批准 %1 个任务").arg(successCount));
    refreshTasks();
}

void PrintOutputPage::rejectSelected()
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

    // Get current user for rejection
    AuthService &authService = AuthService::getInstance();
    User currentUser = authService.getCurrentUser();
    QString approver = currentUser.getUsername().isEmpty() ? "admin" : currentUser.getUsername();

    TaskService &taskService = TaskService::getInstance();
    int successCount = 0;

    for (int taskId : selectedTaskIds) {
        if (taskService.submitApproval(taskId, approver, false, "批量拒绝")) {
            successCount++;
        }
    }

    QMessageBox::information(this, "提示", QString("已拒绝 %1 个任务").arg(successCount));
    refreshTasks();
}

void PrintOutputPage::printSelected()
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
        QMessageBox::information(this, "提示", "请先选择要打印的任务");
        return;
    }

    TaskService &taskService = TaskService::getInstance();
    SerialNumberService &serialService = SerialNumberService::getInstance();
    TaskRepository taskRepo;
    int successCount = 0;
    int failCount = 0;

    for (int taskId : selectedTaskIds) {
        // Get task details
        Task task = taskService.getTaskById(taskId);
        if (task.getId() > 0 && task.getStatus().toUpper() == "APPROVED") {
            bool executionSuccess = false;
            QString failReason;

            try {
                // Generate serial number
                QString serialNumber = serialService.generateSerialNumber(SerialNumberConfigType::PRINT);
                if (serialNumber.isEmpty()) {
                    failReason = "Failed to generate serial number";
                    qDebug() << "Failed to generate serial number for task:" << taskId;
                } else {
                    // Update task status to in progress and set serial number
                    if (taskService.updateTaskStatus(taskId, "IN_PROGRESS")) {
                        // Update serial number in database
                        if (taskRepo.updateSerialNumber(taskId, serialNumber)) {
                            qDebug() << "Task" << taskId << "assigned serial number:" << serialNumber;

                            // Generate barcode
                            BarcodeService &barcodeService = BarcodeService::getInstance();
                            BarcodeConfig config = barcodeService.getConfigByType(0);  // 0 = PRINT
                            QString barcodeContent = barcodeService.generateBarcodeContent(task, config);

                            if (!barcodeContent.isEmpty()) {
                                QString barcodePath = QString("/tmp/barcode_%1.png").arg(serialNumber);
                                if (barcodeService.saveBarcodeToFile(barcodeContent, barcodePath,
                                                                      config.getBarcodeWidth(),
                                                                      config.getBarcodeHeight())) {
                                    qDebug() << "Barcode saved to:" << barcodePath;
                                } else {
                                    qDebug() << "Failed to save barcode for task:" << taskId;
                                }
                            }

                            // In real implementation, this would trigger actual printing
                            // For now, mark as completed
                            if (taskService.updateTaskStatus(taskId, "COMPLETED")) {
                                executionSuccess = true;
                                successCount++;
                            } else {
                                failReason = "Failed to update status to COMPLETED";
                            }
                        } else {
                            failReason = "Failed to update serial number";
                            qDebug() << "Failed to update serial number for task:" << taskId;
                        }
                    } else {
                        failReason = "Failed to update status to IN_PROGRESS";
                    }
                }
            } catch (const std::exception& e) {
                failReason = QString("Exception: %1").arg(e.what());
                qDebug() << "Exception during print execution for task:" << taskId << "-" << e.what();
            } catch (...) {
                failReason = "Unknown exception";
                qDebug() << "Unknown exception during print execution for task:" << taskId;
            }

            if (!executionSuccess) {
                // Update status to FAILED
                taskService.updateTaskStatus(taskId, "FAILED");
                qDebug() << "Task execution failed for task:" << taskId << "reason:" << failReason;
                failCount++;
            }
        }
    }

    QString message;
    if (failCount > 0) {
        message = QString("打印完成: 成功 %1 个, 失败 %2 个").arg(successCount).arg(failCount);
    } else {
        message = QString("正在执行 %1 个打印任务").arg(successCount);
    }
    QMessageBox::information(this, "提示", message);
    refreshTasks();
}