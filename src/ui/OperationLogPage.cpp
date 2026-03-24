#include "OperationLogPage.h"
#include "qt_compat.h"
#include <QHeaderView>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFormLayout>
#include "services/UserManagementService.h"

// Helper function to escape CSV fields
static QString escapeCsvField(const QString &field)
{
    if (field.contains(',') || field.contains('"') || field.contains('\n')) {
        QString escaped = field;
        escaped.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return field;
}

// LogDetailDialog Implementation
LogDetailDialog::LogDetailDialog(QWidget* parent, const AuditLogEntry& log)
    : QDialog(parent)
    , m_log(log)
    , m_detailText(nullptr)
{
    setWindowTitle(QString("日志详情 - ID: %1").arg(log.id));
    setModal(true);
    resize(600, 400);
    setupUI();
}

void LogDetailDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_detailText = new QTextEdit(this);
    m_detailText->setReadOnly(true);
    m_detailText->setHtml(formatLogDetails(m_log));
    mainLayout->addWidget(m_detailText);

    // Close button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* closeButton = new QPushButton("关闭", this);
    connect(closeButton, &QPushButton::clicked, this, &LogDetailDialog::onCloseClicked);
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);
}

QString LogDetailDialog::formatLogDetails(const AuditLogEntry& log) const
{
    QString html;
    html += "<style>table { width: 100%; border-collapse: collapse; } ";
    html += "td { padding: 8px; border: 1px solid #ddd; } ";
    html += "td:first-child { background: #f5f5f5; width: 120px; font-weight: bold; }</style>";
    html += "<table>";

    html += QString("<tr><td>日志ID</td><td>%1</td></tr>").arg(log.id);
    html += QString("<tr><td>操作时间</td><td>%1</td></tr>").arg(log.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
    html += QString("<tr><td>操作模块</td><td>%1</td></tr>").arg(log.eventType);
    html += QString("<tr><td>用户名</td><td>%1</td></tr>").arg(log.username);
    html += QString("<tr><td>用户ID</td><td>%1</td></tr>").arg(log.userId);
    html += QString("<tr><td>登录IP</td><td>%1</td></tr>").arg(log.ipAddress);
    html += QString("<tr><td>操作系统</td><td>%1</td></tr>").arg(log.userAgent);
    html += QString("<tr><td>严重级别</td><td>%1</td></tr>").arg(log.severity);
    html += QString("<tr><td>操作详情</td><td>%1</td></tr>").arg(log.details.toHtmlEscaped());

    html += "</table>";
    return html;
}

void LogDetailDialog::onCloseClicked()
{
    accept();
}

// OperationLogPage Implementation
OperationLogPage::OperationLogPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_searchGroup(nullptr)
    , m_moduleFilterCombo(nullptr)
    , m_usernameEdit(nullptr)
    , m_resultFilterCombo(nullptr)
    , m_startTimeEdit(nullptr)
    , m_endTimeEdit(nullptr)
    , m_logTable(nullptr)
    , m_exportButton(nullptr)
    , m_refreshButton(nullptr)
    , m_paginationWidget(nullptr)
    , m_firstPageBtn(nullptr)
    , m_prevPageBtn(nullptr)
    , m_nextPageBtn(nullptr)
    , m_lastPageBtn(nullptr)
    , m_pageInfoLabel(nullptr)
    , m_pageSizeCombo(nullptr)
    , m_pageSpinBox(nullptr)
    , m_currentPage(1)
    , m_pageSize(20)
    , m_totalCount(0)
{
    setupUI();
    loadData();
}

void OperationLogPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("操作日志", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_mainLayout->addWidget(m_titleLabel);

    // Search Area
    setupSearchArea();

    // Table
    setupTable();

    // Pagination
    setupPagination();

    // Toolbar
    setupToolbar();

    m_mainLayout->addStretch();
}

void OperationLogPage::setupSearchArea()
{
    m_searchGroup = new QGroupBox("搜索条件", this);
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);

    // Operation Module
    searchLayout->addWidget(new QLabel("操作模块:", this));
    m_moduleFilterCombo = new QComboBox(this);
    m_moduleFilterCombo->addItem("全部", "");
    m_moduleFilterCombo->addItem("用户管理", "USER_MANAGEMENT");
    m_moduleFilterCombo->addItem("角色管理", "ROLE_MANAGEMENT");
    m_moduleFilterCombo->addItem("组织管理", "ORG_MANAGEMENT");
    m_moduleFilterCombo->addItem("设备管理", "DEVICE_MANAGEMENT");
    m_moduleFilterCombo->addItem("任务管理", "TASK_MANAGEMENT");
    m_moduleFilterCombo->addItem("系统配置", "SYSTEM_CONFIG");
    m_moduleFilterCombo->addItem("登录登出", "LOGIN");
    searchLayout->addWidget(m_moduleFilterCombo);

    // Username
    searchLayout->addWidget(new QLabel("用户名:", this));
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("输入用户名");
    m_usernameEdit->setMaximumWidth(150);
    searchLayout->addWidget(m_usernameEdit);

    // Operation Result
    searchLayout->addWidget(new QLabel("操作结果:", this));
    m_resultFilterCombo = new QComboBox(this);
    m_resultFilterCombo->addItem("全部", "");
    m_resultFilterCombo->addItem("成功", "INFO");
    m_resultFilterCombo->addItem("警告", "WARN");
    m_resultFilterCombo->addItem("失败", "ERROR");
    searchLayout->addWidget(m_resultFilterCombo);

    // Time Range
    searchLayout->addWidget(new QLabel("时间范围:", this));
    m_startTimeEdit = new QDateTimeEdit(this);
    m_startTimeEdit->setCalendarPopup(true);
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
    m_startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    searchLayout->addWidget(m_startTimeEdit);

    searchLayout->addWidget(new QLabel("至", this));

    m_endTimeEdit = new QDateTimeEdit(this);
    m_endTimeEdit->setCalendarPopup(true);
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    searchLayout->addWidget(m_endTimeEdit);

    // Search and Reset buttons
    QPushButton* searchButton = new QPushButton("查询", this);
    QPushButton* resetButton = new QPushButton("重置", this);
    connect(searchButton, &QPushButton::clicked, this, &OperationLogPage::onSearchClicked);
    connect(resetButton, &QPushButton::clicked, this, &OperationLogPage::onResetClicked);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(resetButton);

    searchLayout->addStretch();

    m_mainLayout->addWidget(m_searchGroup);
}

void OperationLogPage::setupTable()
{
    m_logTable = new QTableWidget(0, 11, this);
    m_logTable->setHorizontalHeaderLabels({
        "日志ID", "操作模块", "用户名", "所属部门", "用户角色",
        "登录IP", "操作系统", "操作类型", "操作结果", "操作详情", "操作时间"
    });
    m_logTable->horizontalHeader()->setStretchLastSection(true);
    m_logTable->verticalHeader()->setVisible(false);
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->setAlternatingRowColors(true);

    // Set column widths
    m_logTable->setColumnWidth(0, 70);   // 日志ID
    m_logTable->setColumnWidth(1, 90);   // 操作模块
    m_logTable->setColumnWidth(2, 90);   // 用户名
    m_logTable->setColumnWidth(3, 90);   // 所属部门
    m_logTable->setColumnWidth(4, 90);   // 用户角色
    m_logTable->setColumnWidth(5, 110);  // 登录IP
    m_logTable->setColumnWidth(6, 100);  // 操作系统
    m_logTable->setColumnWidth(7, 80);   // 操作类型
    m_logTable->setColumnWidth(8, 70);   // 操作结果
    m_logTable->setColumnWidth(9, 200);  // 操作详情
    m_logTable->setColumnWidth(10, 140); // 操作时间

    connect(m_logTable, &QTableWidget::cellDoubleClicked, this, &OperationLogPage::onTableDoubleClicked);

    m_mainLayout->addWidget(m_logTable);
}

void OperationLogPage::setupPagination()
{
    m_paginationWidget = new QWidget(this);
    QHBoxLayout* paginationLayout = new QHBoxLayout(m_paginationWidget);
    paginationLayout->setContentsMargins(0, 5, 0, 5);

    // Page size selector
    paginationLayout->addWidget(new QLabel("每页显示:", this));
    m_pageSizeCombo = new QComboBox(this);
    m_pageSizeCombo->addItem("10", 10);
    m_pageSizeCombo->addItem("20", 20);
    m_pageSizeCombo->addItem("50", 50);
    m_pageSizeCombo->addItem("100", 100);
    m_pageSizeCombo->setCurrentText("20");
    connect(m_pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OperationLogPage::onPageSizeChanged);
    paginationLayout->addWidget(m_pageSizeCombo);

    paginationLayout->addSpacing(20);

    // Navigation buttons
    m_firstPageBtn = new QPushButton("首页", this);
    m_prevPageBtn = new QPushButton("上一页", this);
    m_nextPageBtn = new QPushButton("下一页", this);
    m_lastPageBtn = new QPushButton("末页", this);

    connect(m_firstPageBtn, &QPushButton::clicked, this, &OperationLogPage::onFirstPageClicked);
    connect(m_prevPageBtn, &QPushButton::clicked, this, &OperationLogPage::onPrevPageClicked);
    connect(m_nextPageBtn, &QPushButton::clicked, this, &OperationLogPage::onNextPageClicked);
    connect(m_lastPageBtn, &QPushButton::clicked, this, &OperationLogPage::onLastPageClicked);

    paginationLayout->addWidget(m_firstPageBtn);
    paginationLayout->addWidget(m_prevPageBtn);
    paginationLayout->addWidget(m_nextPageBtn);
    paginationLayout->addWidget(m_lastPageBtn);

    paginationLayout->addSpacing(20);

    // Page info
    m_pageInfoLabel = new QLabel("第 1 页 / 共 1 页 (共 0 条记录)", this);
    paginationLayout->addWidget(m_pageInfoLabel);

    // Page jump
    paginationLayout->addSpacing(20);
    paginationLayout->addWidget(new QLabel("跳转到:", this));
    m_pageSpinBox = new QSpinBox(this);
    m_pageSpinBox->setMinimum(1);
    m_pageSpinBox->setMaximum(1);
    m_pageSpinBox->setValue(1);
    paginationLayout->addWidget(m_pageSpinBox);
    QPushButton* goButton = new QPushButton("GO", this);
    connect(goButton, &QPushButton::clicked, [this]() {
        m_currentPage = m_pageSpinBox->value();
        loadData();
    });
    paginationLayout->addWidget(goButton);

    paginationLayout->addStretch();

    m_mainLayout->addWidget(m_paginationWidget);
}

void OperationLogPage::setupToolbar()
{
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->addStretch();

    m_exportButton = new QPushButton("导出Excel", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_exportButton, &QPushButton::clicked, this, &OperationLogPage::onExportClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &OperationLogPage::onRefreshClicked);

    toolbarLayout->addWidget(m_exportButton);
    toolbarLayout->addWidget(m_refreshButton);

    m_mainLayout->addLayout(toolbarLayout);
}

void OperationLogPage::loadData()
{
    LogAuditService& service = LogAuditService::getInstance();

    QString eventType = m_moduleFilterCombo->currentData().toString();
    QString severity = m_resultFilterCombo->currentData().toString();
    QDateTime startTime = m_startTimeEdit->dateTime();
    QDateTime endTime = m_endTimeEdit->dateTime();

    // Calculate offset
    int offset = (m_currentPage - 1) * m_pageSize;

    // Query logs with pagination
    m_allLogs = service.queryLogs(eventType, -1, startTime, endTime, severity, m_pageSize, offset);

    // Get total count (we'll estimate based on the returned data for simplicity)
    // In a real implementation, you'd have a separate count query
    m_totalCount = m_allLogs.count() >= m_pageSize ? m_pageSize * m_currentPage + 1 : offset + m_allLogs.count();

    refreshTable(m_allLogs);
    updatePaginationInfo();
}

void OperationLogPage::refreshTable(const QList<AuditLogEntry>& logs)
{
    m_logTable->setRowCount(0);

    for (int i = 0; i < logs.count(); ++i) {
        const AuditLogEntry& log = logs[i];

        m_logTable->insertRow(i);

        // 日志ID
        m_logTable->setItem(i, 0, new QTableWidgetItem(QString::number(log.id)));
        // 操作模块
        m_logTable->setItem(i, 1, new QTableWidgetItem(getOperationModule(log.eventType)));
        // 用户名
        m_logTable->setItem(i, 2, new QTableWidgetItem(log.username));
        // 所属部门
        m_logTable->setItem(i, 3, new QTableWidgetItem(getDepartmentName(log.userId)));
        // 用户角色
        m_logTable->setItem(i, 4, new QTableWidgetItem(getRoleNames(log.userId)));
        // 登录IP
        m_logTable->setItem(i, 5, new QTableWidgetItem(log.ipAddress));
        // 操作系统
        m_logTable->setItem(i, 6, new QTableWidgetItem(parseOSFromUserAgent(log.userAgent)));
        // 操作类型
        m_logTable->setItem(i, 7, new QTableWidgetItem(log.eventType));
        // 操作结果
        QString resultText = getOperationResult(log.severity);
        QTableWidgetItem* resultItem = new QTableWidgetItem(resultText);
        if (log.severity == "ERROR") {
            resultItem->setForeground(QBrush(Qt::red));
        } else if (log.severity == "WARN") {
            resultItem->setForeground(QBrush(QColor(255, 165, 0))); // Orange
        } else {
            resultItem->setForeground(QBrush(Qt::darkGreen));
        }
        m_logTable->setItem(i, 8, resultItem);
        // 操作详情
        QString details = log.details;
        if (details.length() > 50) {
            details = details.left(50) + "...";
        }
        m_logTable->setItem(i, 9, new QTableWidgetItem(details));
        // 操作时间
        m_logTable->setItem(i, 10, new QTableWidgetItem(log.timestamp.toString("yyyy-MM-dd hh:mm:ss")));

        // Store log ID in the row
        m_logTable->item(i, 0)->setData(Qt::UserRole, log.id);
    }
}

void OperationLogPage::updatePaginationInfo()
{
    int totalPages = (m_totalCount + m_pageSize - 1) / m_pageSize;
    if (totalPages < 1) totalPages = 1;

    m_pageInfoLabel->setText(QString("第 %1 页 / 共 %2 页 (共 %3 条记录)")
                             .arg(m_currentPage)
                             .arg(totalPages)
                             .arg(m_totalCount));

    m_pageSpinBox->setMaximum(totalPages);
    m_pageSpinBox->setValue(m_currentPage);

    // Enable/disable navigation buttons
    m_firstPageBtn->setEnabled(m_currentPage > 1);
    m_prevPageBtn->setEnabled(m_currentPage > 1);
    m_nextPageBtn->setEnabled(m_currentPage < totalPages);
    m_lastPageBtn->setEnabled(m_currentPage < totalPages);
}

AuditLogEntry OperationLogPage::getSelectedLog() const
{
    AuditLogEntry log;

    QList<QTableWidgetItem*> selectedItems = m_logTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return log;
    }

    int row = selectedItems.first()->row();
    int logId = m_logTable->item(row, 0)->data(Qt::UserRole).toInt();

    // Find the log in our cached data
    for (const AuditLogEntry& entry : m_allLogs) {
        if (entry.id == logId) {
            return entry;
        }
    }

    return log;
}

QString OperationLogPage::getOperationModule(const QString& eventType) const
{
    if (eventType.startsWith("USER") || eventType == "CREATE_USER" || eventType == "UPDATE_USER" || eventType == "DELETE_USER") {
        return "用户管理";
    } else if (eventType.startsWith("ROLE") || eventType.contains("ROLE")) {
        return "角色管理";
    } else if (eventType.startsWith("ORG") || eventType.startsWith("DEPT") || eventType.contains("DEPARTMENT")) {
        return "组织管理";
    } else if (eventType.startsWith("DEVICE") || eventType.contains("DEVICE")) {
        return "设备管理";
    } else if (eventType.startsWith("TASK") || eventType.contains("TASK") || eventType == "APPROVE_TASK" || eventType == "REJECT_TASK") {
        return "任务管理";
    } else if (eventType.contains("CONFIG") || eventType.contains("SECURITY_LEVEL") || eventType.contains("SERIAL") || eventType.contains("BARCODE")) {
        return "系统配置";
    } else if (eventType == "LOGIN" || eventType == "LOGOUT") {
        return "登录登出";
    } else {
        return "其他";
    }
}

QString OperationLogPage::getOperationResult(const QString& severity) const
{
    if (severity == "INFO") {
        return "成功";
    } else if (severity == "WARN") {
        return "警告";
    } else if (severity == "ERROR") {
        return "失败";
    }
    return severity;
}

QString OperationLogPage::getDepartmentName(int userId) const
{
    if (userId <= 0) {
        return "-";
    }

    UserManagementService& service = UserManagementService::getInstance();
    User user = service.getUserById(userId);

    if (user.getId() <= 0) {
        return "-";
    }

    int deptId = user.getDepartmentId();
    switch (deptId) {
        case 1: return "总部";
        case 2: return "研发部";
        case 3: return "市场部";
        case 4: return "财务部";
        case 5: return "人事部";
        default: return "-";
    }
}

QString OperationLogPage::getRoleNames(int userId) const
{
    if (userId <= 0) {
        return "-";
    }

    UserManagementService& service = UserManagementService::getInstance();
    QList<Role> roles = service.getUserRoles(userId);

    QStringList roleNames;
    for (const Role& role : roles) {
        roleNames.append(role.getName());
    }

    if (roleNames.isEmpty()) {
        return "-";
    }
    return roleNames.join(", ");
}

QString OperationLogPage::parseOSFromUserAgent(const QString& userAgent) const
{
    if (userAgent.isEmpty()) {
        return "-";
    }

    if (userAgent.contains("Windows NT 10", Qt::CaseInsensitive)) {
        return "Windows 10";
    } else if (userAgent.contains("Windows NT 6.3", Qt::CaseInsensitive)) {
        return "Windows 8.1";
    } else if (userAgent.contains("Windows NT 6.2", Qt::CaseInsensitive)) {
        return "Windows 8";
    } else if (userAgent.contains("Windows NT 6.1", Qt::CaseInsensitive)) {
        return "Windows 7";
    } else if (userAgent.contains("Mac OS X", Qt::CaseInsensitive)) {
        return "macOS";
    } else if (userAgent.contains("Linux", Qt::CaseInsensitive)) {
        return "Linux";
    } else if (userAgent.contains("Android", Qt::CaseInsensitive)) {
        return "Android";
    } else if (userAgent.contains("iPhone") || userAgent.contains("iPad", Qt::CaseInsensitive)) {
        return "iOS";
    }

    return userAgent.left(20);
}

void OperationLogPage::onSearchClicked()
{
    m_currentPage = 1;
    loadData();
}

void OperationLogPage::onResetClicked()
{
    m_moduleFilterCombo->setCurrentIndex(0);
    m_usernameEdit->clear();
    m_resultFilterCombo->setCurrentIndex(0);
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_currentPage = 1;
    loadData();
}

void OperationLogPage::onExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出操作日志", "operation_logs.csv",
        "CSV文件 (*.csv);;Excel文件 (*.xlsx);;文本文件 (*.txt)");

    if (filePath.isEmpty()) {
        return;
    }

    LogAuditService& service = LogAuditService::getInstance();
    QDateTime startTime = m_startTimeEdit->dateTime();
    QDateTime endTime = m_endTimeEdit->dateTime();

    // Export using service
    QString format = "CSV";
    if (filePath.endsWith(".xlsx", Qt::CaseInsensitive)) {
        format = "EXCEL";
    }

    if (service.exportLogs(filePath, startTime, endTime, format)) {
        QMessageBox::information(this, "成功", QString("操作日志导出成功！\n文件: %1").arg(filePath));
    } else {
        // Manual CSV export as fallback
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("UTF-8");

            // Write header
            out << "日志ID,操作模块,用户名,所属部门,用户角色,登录IP,操作系统,操作类型,操作结果,操作详情,操作时间\n";

            // Write data
            for (const AuditLogEntry& log : m_allLogs) {
                out << escapeCsvField(QString::number(log.id)) << ","
                    << escapeCsvField(getOperationModule(log.eventType)) << ","
                    << escapeCsvField(log.username) << ","
                    << escapeCsvField(getDepartmentName(log.userId)) << ","
                    << escapeCsvField(getRoleNames(log.userId)) << ","
                    << escapeCsvField(log.ipAddress) << ","
                    << escapeCsvField(parseOSFromUserAgent(log.userAgent)) << ","
                    << escapeCsvField(log.eventType) << ","
                    << escapeCsvField(getOperationResult(log.severity)) << ","
                    << escapeCsvField(log.details) << ","
                    << escapeCsvField(log.timestamp.toString("yyyy-MM-dd hh:mm:ss")) << "\n";
            }

            file.close();
            QMessageBox::information(this, "成功", QString("操作日志导出成功！\n文件: %1").arg(filePath));
        } else {
            QMessageBox::critical(this, "错误", "导出失败，无法创建文件。");
        }
    }
}

void OperationLogPage::onRefreshClicked()
{
    loadData();
}

void OperationLogPage::onPrevPageClicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        loadData();
    }
}

void OperationLogPage::onNextPageClicked()
{
    int totalPages = (m_totalCount + m_pageSize - 1) / m_pageSize;
    if (m_currentPage < totalPages) {
        m_currentPage++;
        loadData();
    }
}

void OperationLogPage::onFirstPageClicked()
{
    m_currentPage = 1;
    loadData();
}

void OperationLogPage::onLastPageClicked()
{
    int totalPages = (m_totalCount + m_pageSize - 1) / m_pageSize;
    m_currentPage = totalPages;
    loadData();
}

void OperationLogPage::onPageSizeChanged(int index)
{
    Q_UNUSED(index);
    m_pageSize = m_pageSizeCombo->currentData().toInt();
    m_currentPage = 1;
    loadData();
}

void OperationLogPage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    if (row < 0 || row >= m_allLogs.count()) {
        return;
    }

    AuditLogEntry log = m_allLogs[row];
    LogDetailDialog dialog(this, log);
    dialog.exec();
}