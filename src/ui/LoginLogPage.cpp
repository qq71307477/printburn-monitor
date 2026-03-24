#include "LoginLogPage.h"
#include <QHeaderView>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "qt_compat.h"
#include "src/common/repository/department_repository.h"
#include "src/common/repository/role_repository.h"
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

// LoginLogPage Implementation
LoginLogPage::LoginLogPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_searchGroup(nullptr)
    , m_usernameEdit(nullptr)
    , m_startTimeEdit(nullptr)
    , m_endTimeEdit(nullptr)
    , m_operationTypeCombo(nullptr)
    , m_resultCombo(nullptr)
    , m_logTable(nullptr)
    , m_currentPage(1)
    , m_pageSize(20)
    , m_totalCount(0)
    , m_pageSpinBox(nullptr)
    , m_pageInfoLabel(nullptr)
    , m_prevPageButton(nullptr)
    , m_nextPageButton(nullptr)
    , m_pageSizeCombo(nullptr)
    , m_searchButton(nullptr)
    , m_resetButton(nullptr)
    , m_exportButton(nullptr)
    , m_refreshButton(nullptr)
{
    setupUI();
    loadData();
}

void LoginLogPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("登录日志", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_mainLayout->addWidget(m_titleLabel);

    // Search Area
    setupSearchArea();

    // Toolbar
    setupToolbar();

    // Table
    setupTable();

    // Pagination
    setupPagination();

    m_mainLayout->addStretch();
}

void LoginLogPage::setupSearchArea()
{
    m_searchGroup = new QGroupBox("搜索条件", this);
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);

    // Username
    searchLayout->addWidget(new QLabel("用户名:", this));
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("输入用户名");
    m_usernameEdit->setMaximumWidth(150);
    searchLayout->addWidget(m_usernameEdit);

    // Operation Type
    searchLayout->addWidget(new QLabel("操作类型:", this));
    m_operationTypeCombo = new QComboBox(this);
    m_operationTypeCombo->addItem("全部", "");
    m_operationTypeCombo->addItem("登录", "LOGIN");
    m_operationTypeCombo->addItem("登出", "LOGOUT");
    m_operationTypeCombo->addItem("登录失败", "LOGIN_FAILED");
    m_operationTypeCombo->setMaximumWidth(120);
    searchLayout->addWidget(m_operationTypeCombo);

    // Operation Result
    searchLayout->addWidget(new QLabel("操作结果:", this));
    m_resultCombo = new QComboBox(this);
    m_resultCombo->addItem("全部", "");
    m_resultCombo->addItem("成功", "INFO");
    m_resultCombo->addItem("失败", "ERROR");
    m_resultCombo->addItem("警告", "WARN");
    m_resultCombo->setMaximumWidth(100);
    searchLayout->addWidget(m_resultCombo);

    // Time Range
    searchLayout->addWidget(new QLabel("操作时间:", this));
    m_startTimeEdit = new QDateTimeEdit(this);
    m_startTimeEdit->setCalendarPopup(true);
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
    m_startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_startTimeEdit->setMaximumWidth(150);
    searchLayout->addWidget(m_startTimeEdit);

    searchLayout->addWidget(new QLabel("至", this));
    m_endTimeEdit = new QDateTimeEdit(this);
    m_endTimeEdit->setCalendarPopup(true);
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_endTimeEdit->setMaximumWidth(150);
    searchLayout->addWidget(m_endTimeEdit);

    searchLayout->addStretch();

    m_mainLayout->addWidget(m_searchGroup);
}

void LoginLogPage::setupTable()
{
    m_logTable = new QTableWidget(0, 9, this);
    m_logTable->setHorizontalHeaderLabels({
        "日志ID", "用户名", "所属部门", "用户角色",
        "登录IP", "操作系统", "操作类型", "操作结果", "操作时间"
    });
    m_logTable->horizontalHeader()->setStretchLastSection(true);
    m_logTable->verticalHeader()->setVisible(false);
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->setAlternatingRowColors(true);

    // Set column widths
    m_logTable->setColumnWidth(0, 70);   // 日志ID
    m_logTable->setColumnWidth(1, 100);  // 用户名
    m_logTable->setColumnWidth(2, 100);  // 所属部门
    m_logTable->setColumnWidth(3, 100);  // 用户角色
    m_logTable->setColumnWidth(4, 120);  // 登录IP
    m_logTable->setColumnWidth(5, 100);  // 操作系统
    m_logTable->setColumnWidth(6, 80);   // 操作类型
    m_logTable->setColumnWidth(7, 80);   // 操作结果
    m_logTable->setColumnWidth(8, 150);  // 操作时间

    m_mainLayout->addWidget(m_logTable);
}

void LoginLogPage::setupToolbar()
{
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->addStretch();

    m_searchButton = new QPushButton("查询", this);
    m_resetButton = new QPushButton("重置", this);
    m_exportButton = new QPushButton("导出Excel", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_searchButton, &QPushButton::clicked, this, &LoginLogPage::onSearchClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &LoginLogPage::onResetClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &LoginLogPage::onExportClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &LoginLogPage::onRefreshClicked);

    toolbarLayout->addWidget(m_searchButton);
    toolbarLayout->addWidget(m_resetButton);
    toolbarLayout->addWidget(m_exportButton);
    toolbarLayout->addWidget(m_refreshButton);

    m_mainLayout->addLayout(toolbarLayout);
}

void LoginLogPage::setupPagination()
{
    QHBoxLayout* paginationLayout = new QHBoxLayout();

    // Page size selector
    paginationLayout->addWidget(new QLabel("每页显示:", this));
    m_pageSizeCombo = new QComboBox(this);
    m_pageSizeCombo->addItem("20", 20);
    m_pageSizeCombo->addItem("50", 50);
    m_pageSizeCombo->addItem("100", 100);
    m_pageSizeCombo->addItem("200", 200);
    m_pageSizeCombo->setCurrentIndex(0);
    connect(m_pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginLogPage::onPageSizeChanged);
    paginationLayout->addWidget(m_pageSizeCombo);

    paginationLayout->addStretch();

    // Page navigation
    m_prevPageButton = new QPushButton("上一页", this);
    connect(m_prevPageButton, &QPushButton::clicked, this, &LoginLogPage::onPrevPageClicked);
    paginationLayout->addWidget(m_prevPageButton);

    paginationLayout->addWidget(new QLabel("第", this));
    m_pageSpinBox = new QSpinBox(this);
    m_pageSpinBox->setMinimum(1);
    m_pageSpinBox->setMaximum(9999);
    m_pageSpinBox->setValue(1);
    m_pageSpinBox->setMinimumWidth(60);
    paginationLayout->addWidget(m_pageSpinBox);
    paginationLayout->addWidget(new QLabel("页", this));

    m_nextPageButton = new QPushButton("下一页", this);
    connect(m_nextPageButton, &QPushButton::clicked, this, &LoginLogPage::onNextPageClicked);
    paginationLayout->addWidget(m_nextPageButton);

    paginationLayout->addStretch();

    // Page info
    m_pageInfoLabel = new QLabel("共 0 条记录", this);
    paginationLayout->addWidget(m_pageInfoLabel);

    m_mainLayout->addLayout(paginationLayout);
}

void LoginLogPage::loadData()
{
    LogAuditService& service = LogAuditService::getInstance();

    QString eventType = m_operationTypeCombo->currentData().toString();
    QString severity = m_resultCombo->currentData().toString();
    QDateTime startTime = m_startTimeEdit->dateTime();
    QDateTime endTime = m_endTimeEdit->dateTime();

    // Calculate offset based on current page
    int offset = (m_currentPage - 1) * m_pageSize;

    // Get login-related logs
    QList<AuditLogEntry> logs;
    if (eventType.isEmpty()) {
        // Get all login-related events
        logs = service.queryLogs("", -1, startTime, endTime, severity, m_pageSize, offset);
        // Filter to only login-related events
        QList<AuditLogEntry> loginLogs;
        for (const AuditLogEntry& log : logs) {
            QString type = log.eventType.toUpper();
            if (type == "LOGIN" || type == "LOGOUT" || type == "LOGIN_FAILED" ||
                type.contains("LOGIN") || type.contains("LOGOUT")) {
                loginLogs.append(log);
            }
        }
        logs = loginLogs;
    } else {
        logs = service.getLogsByEventType(eventType, startTime, endTime, m_pageSize, offset);
    }

    // Filter by username if provided
    QString username = m_usernameEdit->text().trimmed();
    if (!username.isEmpty()) {
        QList<AuditLogEntry> filteredLogs;
        for (const AuditLogEntry& log : logs) {
            if (log.username.contains(username, Qt::CaseInsensitive)) {
                filteredLogs.append(log);
            }
        }
        logs = filteredLogs;
    }

    // For total count, get all logs without pagination
    QList<AuditLogEntry> allLogs = service.queryLogs("", -1, startTime, endTime, severity, 10000, 0);
    QList<AuditLogEntry> allLoginLogs;
    for (const AuditLogEntry& log : allLogs) {
        QString type = log.eventType.toUpper();
        if (type == "LOGIN" || type == "LOGOUT" || type == "LOGIN_FAILED" ||
            type.contains("LOGIN") || type.contains("LOGOUT")) {
            if (!username.isEmpty() && !log.username.contains(username, Qt::CaseInsensitive)) {
                continue;
            }
            allLoginLogs.append(log);
        }
    }
    m_totalCount = allLoginLogs.count();

    refreshTable(logs);
    updatePaginationInfo();
}

void LoginLogPage::refreshTable(const QList<AuditLogEntry>& logs)
{
    m_logTable->setRowCount(0);

    for (int i = 0; i < logs.count(); ++i) {
        const AuditLogEntry& log = logs[i];

        m_logTable->insertRow(i);

        // 日志ID
        m_logTable->setItem(i, 0, new QTableWidgetItem(QString::number(log.id)));
        // 用户名
        m_logTable->setItem(i, 1, new QTableWidgetItem(log.username));
        // 所属部门
        m_logTable->setItem(i, 2, new QTableWidgetItem(getDepartmentName(log.userId)));
        // 用户角色
        m_logTable->setItem(i, 3, new QTableWidgetItem(getRoleNames(log.userId)));
        // 登录IP
        m_logTable->setItem(i, 4, new QTableWidgetItem(log.ipAddress));
        // 操作系统
        m_logTable->setItem(i, 5, new QTableWidgetItem(getOperatingSystem(log.userAgent)));
        // 操作类型
        m_logTable->setItem(i, 6, new QTableWidgetItem(getOperationType(log.eventType)));
        // 操作结果
        m_logTable->setItem(i, 7, new QTableWidgetItem(getOperationResult(log.severity)));
        // 操作时间
        m_logTable->setItem(i, 8, new QTableWidgetItem(log.timestamp.toString("yyyy-MM-dd hh:mm:ss")));

        // Color coding for operation result
        QTableWidgetItem* resultItem = m_logTable->item(i, 7);
        if (log.severity == "ERROR") {
            resultItem->setForeground(QBrush(Qt::red));
        } else if (log.severity == "WARN") {
            resultItem->setForeground(QBrush(QColor(255, 140, 0))); // Orange
        } else {
            resultItem->setForeground(QBrush(QColor(0, 128, 0))); // Green
        }
    }
}

void LoginLogPage::updatePaginationInfo()
{
    int totalPages = (m_totalCount + m_pageSize - 1) / m_pageSize;
    if (totalPages < 1) totalPages = 1;

    m_pageSpinBox->setMaximum(totalPages);
    m_pageSpinBox->setValue(m_currentPage);

    m_pageInfoLabel->setText(QString("共 %1 条记录，第 %2/%3 页")
                             .arg(m_totalCount)
                             .arg(m_currentPage)
                             .arg(totalPages));

    m_prevPageButton->setEnabled(m_currentPage > 1);
    m_nextPageButton->setEnabled(m_currentPage < totalPages);
}

QString LoginLogPage::getDepartmentName(int userId) const
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
        default: return "未知";
    }
}

QString LoginLogPage::getRoleNames(int userId) const
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
        return "普通用户";
    }
    return roleNames.join(", ");
}

QString LoginLogPage::getOperatingSystem(const QString& userAgent) const
{
    if (userAgent.isEmpty()) {
        return "未知";
    }

    QString ua = userAgent.toLower();

    if (ua.contains("windows nt 10")) return "Windows 10";
    if (ua.contains("windows nt 6.3")) return "Windows 8.1";
    if (ua.contains("windows nt 6.2")) return "Windows 8";
    if (ua.contains("windows nt 6.1")) return "Windows 7";
    if (ua.contains("windows")) return "Windows";
    if (ua.contains("mac os x")) return "macOS";
    if (ua.contains("linux")) return "Linux";
    if (ua.contains("android")) return "Android";
    if (ua.contains("iphone") || ua.contains("ipad")) return "iOS";

    return "其他";
}

QString LoginLogPage::getOperationType(const QString& eventType) const
{
    QString type = eventType.toUpper();

    if (type == "LOGIN") return "登录";
    if (type == "LOGOUT") return "登出";
    if (type == "LOGIN_FAILED") return "登录失败";

    return eventType;
}

QString LoginLogPage::getOperationResult(const QString& severity) const
{
    QString sev = severity.toUpper();

    if (sev == "INFO") return "成功";
    if (sev == "ERROR") return "失败";
    if (sev == "WARN") return "警告";

    return severity;
}

void LoginLogPage::onSearchClicked()
{
    m_currentPage = 1;
    loadData();
}

void LoginLogPage::onResetClicked()
{
    m_usernameEdit->clear();
    m_operationTypeCombo->setCurrentIndex(0);
    m_resultCombo->setCurrentIndex(0);
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_currentPage = 1;
    m_pageSize = 20;
    m_pageSizeCombo->setCurrentIndex(0);
    loadData();
}

void LoginLogPage::onExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出登录日志", "login_logs.csv",
        "CSV文件 (*.csv);;Excel文件 (*.xlsx);;所有文件 (*.*)");

    if (filePath.isEmpty()) {
        return;
    }

    // Ensure .csv extension
    if (!filePath.endsWith(".csv", Qt::CaseInsensitive) && !filePath.endsWith(".xlsx", Qt::CaseInsensitive)) {
        filePath += ".csv";
    }

    LogAuditService& service = LogAuditService::getInstance();

    // Get all logs based on current filters
    QString eventType = m_operationTypeCombo->currentData().toString();
    QString severity = m_resultCombo->currentData().toString();
    QDateTime startTime = m_startTimeEdit->dateTime();
    QDateTime endTime = m_endTimeEdit->dateTime();
    QString username = m_usernameEdit->text().trimmed();

    QList<AuditLogEntry> logs = service.queryLogs("", -1, startTime, endTime, severity, 10000, 0);

    // Filter to login-related events
    QList<AuditLogEntry> exportLogs;
    for (const AuditLogEntry& log : logs) {
        QString type = log.eventType.toUpper();
        if (type == "LOGIN" || type == "LOGOUT" || type == "LOGIN_FAILED" ||
            type.contains("LOGIN") || type.contains("LOGOUT")) {
            if (!username.isEmpty() && !log.username.contains(username, Qt::CaseInsensitive)) {
                continue;
            }
            exportLogs.append(log);
        }
    }

    // Export to CSV
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建导出文件！");
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // Write BOM for Excel compatibility
    out << "\xEF\xBB\xBF";

    // Header
    out << "日志ID,用户名,所属部门,用户角色,登录IP,操作系统,操作类型,操作结果,操作时间\n";

    // Data rows
    for (const AuditLogEntry& log : exportLogs) {
        out << escapeCsvField(QString::number(log.id)) << ","
            << escapeCsvField(log.username) << ","
            << escapeCsvField(getDepartmentName(log.userId)) << ","
            << escapeCsvField(getRoleNames(log.userId)) << ","
            << escapeCsvField(log.ipAddress) << ","
            << escapeCsvField(getOperatingSystem(log.userAgent)) << ","
            << escapeCsvField(getOperationType(log.eventType)) << ","
            << escapeCsvField(getOperationResult(log.severity)) << ","
            << escapeCsvField(log.timestamp.toString("yyyy-MM-dd hh:mm:ss")) << "\n";
    }

    file.close();

    QMessageBox::information(this, "成功",
        QString("登录日志导出成功！\n共导出 %1 条记录\n文件: %2")
        .arg(exportLogs.count())
        .arg(filePath));
}

void LoginLogPage::onRefreshClicked()
{
    loadData();
}

void LoginLogPage::onPrevPageClicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        loadData();
    }
}

void LoginLogPage::onNextPageClicked()
{
    int totalPages = (m_totalCount + m_pageSize - 1) / m_pageSize;
    if (m_currentPage < totalPages) {
        m_currentPage++;
        loadData();
    }
}

void LoginLogPage::onPageSizeChanged(int index)
{
    Q_UNUSED(index);
    m_pageSize = m_pageSizeCombo->currentData().toInt();
    m_currentPage = 1;
    loadData();
}