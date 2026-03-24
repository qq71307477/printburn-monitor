#include "ProxyApproverConfigPage.h"
#include "services/ProxyApproverService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDateEdit>

// ================== ProxyApproverDialog ==================

ProxyApproverDialog::ProxyApproverDialog(QWidget *parent, int proxyId)
    : QDialog(parent)
    , m_proxyId(proxyId)
    , m_currentUserId(0)
    , m_proxyUserCombo(nullptr)
    , m_minSecurityLevelSpin(nullptr)
    , m_maxSecurityLevelSpin(nullptr)
    , m_taskTypeCombo(nullptr)
    , m_startDateEdit(nullptr)
    , m_endDateEdit(nullptr)
    , m_enabledCheck(nullptr)
    , m_buttonBox(nullptr)
{
    setWindowTitle(proxyId < 0 ? QString::fromUtf8("添加代理审批") : QString::fromUtf8("编辑代理审批"));
    setupUI();
    resize(450, 350);
}

void ProxyApproverDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout();

    // Proxy User (代理人)
    m_proxyUserCombo = new QComboBox(this);
    formLayout->addRow(QString::fromUtf8("代理人:"), m_proxyUserCombo);

    // Task Type (任务类型)
    m_taskTypeCombo = new QComboBox(this);
    m_taskTypeCombo->addItem(QString::fromUtf8("打印"), 0);
    m_taskTypeCombo->addItem(QString::fromUtf8("刻录"), 1);
    m_taskTypeCombo->addItem(QString::fromUtf8("全部"), 2);
    m_taskTypeCombo->setCurrentIndex(2);  // Default to "全部"
    formLayout->addRow(QString::fromUtf8("任务类型:"), m_taskTypeCombo);

    // Min Security Level
    QHBoxLayout *securityLayout = new QHBoxLayout();
    m_minSecurityLevelSpin = new QSpinBox(this);
    m_minSecurityLevelSpin->setRange(0, 9999);
    m_minSecurityLevelSpin->setValue(0);
    securityLayout->addWidget(m_minSecurityLevelSpin);
    securityLayout->addWidget(new QLabel(QString::fromUtf8(" - ")));
    m_maxSecurityLevelSpin = new QSpinBox(this);
    m_maxSecurityLevelSpin->setRange(0, 9999);
    m_maxSecurityLevelSpin->setValue(9999);
    securityLayout->addWidget(m_maxSecurityLevelSpin);
    securityLayout->addStretch();
    formLayout->addRow(QString::fromUtf8("密级范围:"), securityLayout);

    // Start Date
    m_startDateEdit = new QDateEdit(this);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDate(QDate::currentDate());
    m_startDateEdit->setDisplayFormat("yyyy-MM-dd");
    formLayout->addRow(QString::fromUtf8("开始日期:"), m_startDateEdit);

    // End Date
    m_endDateEdit = new QDateEdit(this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDate(QDate::currentDate().addMonths(1));
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd");
    formLayout->addRow(QString::fromUtf8("结束日期:"), m_endDateEdit);

    // Is Enabled
    m_enabledCheck = new QCheckBox(QString::fromUtf8("启用此代理"), this);
    m_enabledCheck->setChecked(true);
    formLayout->addRow(m_enabledCheck);

    layout->addLayout(formLayout);

    // Buttons
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        // Validate input
        if (m_proxyUserCombo->currentIndex() < 0) {
            QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请选择代理人！"));
            return;
        }
        if (m_minSecurityLevelSpin->value() > m_maxSecurityLevelSpin->value()) {
            QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("最小密级不能大于最大密级！"));
            return;
        }
        if (m_startDateEdit->date() > m_endDateEdit->date()) {
            QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("开始日期不能晚于结束日期！"));
            return;
        }
        accept();
    });
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttonBox);
}

ProxyApprover ProxyApproverDialog::getProxy() const
{
    ProxyApprover proxy;
    proxy.setId(m_proxyId < 0 ? 0 : m_proxyId);
    proxy.setOwnerUserId(m_currentUserId);
    proxy.setProxyUserId(m_proxyUserCombo->currentData().toInt());
    proxy.setMinSecurityLevelId(m_minSecurityLevelSpin->value());
    proxy.setMaxSecurityLevelId(m_maxSecurityLevelSpin->value());
    proxy.setTaskType(m_taskTypeCombo->currentData().toInt());
    proxy.setStartDate(m_startDateEdit->date());
    proxy.setEndDate(m_endDateEdit->date());
    proxy.setEnabled(m_enabledCheck->isChecked());
    return proxy;
}

void ProxyApproverDialog::setProxy(const ProxyApprover& proxy)
{
    m_proxyId = proxy.getId();
    m_currentUserId = proxy.getOwnerUserId();
    
    int userIndex = m_proxyUserCombo->findData(proxy.getProxyUserId());
    if (userIndex >= 0) {
        m_proxyUserCombo->setCurrentIndex(userIndex);
    }
    
    int taskIndex = m_taskTypeCombo->findData(proxy.getTaskType());
    if (taskIndex >= 0) {
        m_taskTypeCombo->setCurrentIndex(taskIndex);
    }
    
    m_minSecurityLevelSpin->setValue(proxy.getMinSecurityLevelId());
    m_maxSecurityLevelSpin->setValue(proxy.getMaxSecurityLevelId());
    
    if (!proxy.getStartDate().isNull()) {
        m_startDateEdit->setDate(proxy.getStartDate());
    }
    if (!proxy.getEndDate().isNull()) {
        m_endDateEdit->setDate(proxy.getEndDate());
    }
    
    m_enabledCheck->setChecked(proxy.isEnabled());
}

void ProxyApproverDialog::setUsers(const QList<QPair<int, QString>>& users)
{
    m_users = users;
    m_proxyUserCombo->clear();
    for (const auto& user : users) {
        m_proxyUserCombo->addItem(user.second, user.first);
    }
}

void ProxyApproverDialog::setSecurityLevels(const QList<QPair<int, QString>>& levels)
{
    m_securityLevels = levels;
}

void ProxyApproverDialog::setCurrentUserId(int userId)
{
    m_currentUserId = userId;
}

// ================== ProxyApproverConfigPage ==================

ProxyApproverConfigPage::ProxyApproverConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_proxyTable(nullptr)
    , m_currentUserId(1)  // Default to user ID 1, should be set from login context
{
    setupUI();
    loadData();
}

void ProxyApproverConfigPage::setCurrentUserId(int userId)
{
    m_currentUserId = userId;
    loadData();
}

void ProxyApproverConfigPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    // Header
    m_headerLayout = new QHBoxLayout();
    m_pageTitle = new QLabel(QString::fromUtf8("代理审批设置"), this);
    m_pageTitle->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_headerLayout->addWidget(m_pageTitle);
    m_headerLayout->addStretch();

    m_refreshButton = new QPushButton(QString::fromUtf8("刷新"), this);
    connect(m_refreshButton, &QPushButton::clicked, this, &ProxyApproverConfigPage::onRefreshClicked);
    m_headerLayout->addWidget(m_refreshButton);

    m_layout->addLayout(m_headerLayout);

    // Description
    QLabel* descLabel = new QLabel(QString::fromUtf8("设置代理审批人后，在您缺席时代理人可以代为审批任务。"), this);
    descLabel->setStyleSheet("color: #666; margin-bottom: 10px;");
    m_layout->addWidget(descLabel);

    // Table
    setupTable();

    // Button area
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();

    m_addButton = new QPushButton(QString::fromUtf8("添加"), this);
    m_editButton = new QPushButton(QString::fromUtf8("编辑"), this);
    m_deleteButton = new QPushButton(QString::fromUtf8("删除"), this);
    m_toggleEnabledButton = new QPushButton(QString::fromUtf8("启用/禁用"), this);

    connect(m_addButton, &QPushButton::clicked, this, &ProxyApproverConfigPage::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &ProxyApproverConfigPage::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &ProxyApproverConfigPage::onDeleteClicked);
    connect(m_toggleEnabledButton, &QPushButton::clicked, this, &ProxyApproverConfigPage::onToggleEnabledClicked);

    m_buttonLayout->addWidget(m_addButton);
    m_buttonLayout->addWidget(m_editButton);
    m_buttonLayout->addWidget(m_deleteButton);
    m_buttonLayout->addWidget(m_toggleEnabledButton);

    m_layout->addLayout(m_buttonLayout);

    // Status label
    QLabel* statusLabel = new QLabel(QString::fromUtf8("提示: 双击表格行可直接编辑。"), this);
    statusLabel->setStyleSheet("color: #999; font-size: 12px;");
    m_layout->addWidget(statusLabel);

    m_layout->addStretch();
}

void ProxyApproverConfigPage::setupTable()
{
    m_proxyTable = new QTableWidget(0, 8, this);
    m_proxyTable->setHorizontalHeaderLabels({
        QString::fromUtf8("序号"),
        QString::fromUtf8("代理人"),
        QString::fromUtf8("密级范围"),
        QString::fromUtf8("任务类型"),
        QString::fromUtf8("开始日期"),
        QString::fromUtf8("结束日期"),
        QString::fromUtf8("状态"),
        QString::fromUtf8("操作")
    });

    m_proxyTable->horizontalHeader()->setStretchLastSection(true);
    m_proxyTable->verticalHeader()->setVisible(false);
    m_proxyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_proxyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_proxyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set column widths
    m_proxyTable->setColumnWidth(0, 50);   // 序号
    m_proxyTable->setColumnWidth(1, 120);  // 代理人
    m_proxyTable->setColumnWidth(2, 100);  // 密级范围
    m_proxyTable->setColumnWidth(3, 80);   // 任务类型
    m_proxyTable->setColumnWidth(4, 100);  // 开始日期
    m_proxyTable->setColumnWidth(5, 100);  // 结束日期
    m_proxyTable->setColumnWidth(6, 80);   // 状态
    m_proxyTable->setColumnWidth(7, 80);   // 操作

    connect(m_proxyTable, &QTableWidget::cellDoubleClicked,
            this, &ProxyApproverConfigPage::onTableDoubleClicked);

    m_layout->addWidget(m_proxyTable);
}

void ProxyApproverConfigPage::loadData()
{
    m_users = loadUsers();
    m_securityLevels = loadSecurityLevels();

    ProxyApproverService& service = ProxyApproverService::getInstance();
    m_proxies = service.getProxiesByOwner(m_currentUserId);

    updateTable();
}

void ProxyApproverConfigPage::updateTable()
{
    m_proxyTable->setRowCount(0);

    for (int i = 0; i < m_proxies.size(); ++i) {
        const ProxyApprover& proxy = m_proxies[i];
        int row = m_proxyTable->rowCount();
        m_proxyTable->insertRow(row);

        // 序号
        m_proxyTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));

        // 代理人
        m_proxyTable->setItem(row, 1, new QTableWidgetItem(getUserName(proxy.getProxyUserId())));

        // 密级范围
        m_proxyTable->setItem(row, 2, new QTableWidgetItem(proxy.getSecurityLevelRange()));

        // 任务类型
        m_proxyTable->setItem(row, 3, new QTableWidgetItem(proxy.getTaskTypeName()));

        // 开始日期
        m_proxyTable->setItem(row, 4, new QTableWidgetItem(
            proxy.getStartDate().isNull() ? "-" : proxy.getStartDate().toString("yyyy-MM-dd")));

        // 结束日期
        m_proxyTable->setItem(row, 5, new QTableWidgetItem(
            proxy.getEndDate().isNull() ? "-" : proxy.getEndDate().toString("yyyy-MM-dd")));

        // 状态
        QString status = getProxyStatus(proxy);
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        if (proxy.isEnabled()) {
            QDate today = QDate::currentDate();
            bool inDateRange = (proxy.getStartDate().isNull() || proxy.getStartDate() <= today) &&
                               (proxy.getEndDate().isNull() || proxy.getEndDate() >= today);
            statusItem->setForeground(inDateRange ? QColor(Qt::darkGreen) : QColor(Qt::gray));
        } else {
            statusItem->setForeground(QColor(Qt::red));
        }
        m_proxyTable->setItem(row, 6, statusItem);

        // Store proxy ID in the first column
        m_proxyTable->item(row, 0)->setData(Qt::UserRole, proxy.getId());
    }
}

QList<QPair<int, QString>> ProxyApproverConfigPage::loadUsers()
{
    ProxyApproverService& service = ProxyApproverService::getInstance();
    return service.getUsersForSelection();
}

QList<QPair<int, QString>> ProxyApproverConfigPage::loadSecurityLevels()
{
    ProxyApproverService& service = ProxyApproverService::getInstance();
    return service.getSecurityLevelsForSelection();
}

QString ProxyApproverConfigPage::getUserName(int userId) const
{
    for (const auto& user : m_users) {
        if (user.first == userId) {
            return user.second;
        }
    }
    return QString::fromUtf8("未知用户(%1)").arg(userId);
}

QString ProxyApproverConfigPage::getProxyStatus(const ProxyApprover& proxy) const
{
    if (!proxy.isEnabled()) {
        return QString::fromUtf8("已禁用");
    }

    QDate today = QDate::currentDate();
    if (!proxy.getStartDate().isNull() && proxy.getStartDate() > today) {
        return QString::fromUtf8("未生效");
    }
    if (!proxy.getEndDate().isNull() && proxy.getEndDate() < today) {
        return QString::fromUtf8("已过期");
    }

    return QString::fromUtf8("生效中");
}

void ProxyApproverConfigPage::onAddClicked()
{
    ProxyApproverDialog dialog(this);
    dialog.setUsers(m_users);
    dialog.setSecurityLevels(m_securityLevels);
    dialog.setCurrentUserId(m_currentUserId);

    if (dialog.exec() == QDialog::Accepted) {
        ProxyApprover proxy = dialog.getProxy();

        ProxyApproverService& service = ProxyApproverService::getInstance();
        QString validationError = service.validateProxy(proxy);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, QString::fromUtf8("验证失败"), validationError);
            return;
        }

        if (service.hasConflict(proxy)) {
            QMessageBox::warning(this, QString::fromUtf8("警告"),
                QString::fromUtf8("已存在冲突的代理配置！"));
            return;
        }

        if (service.createProxy(proxy)) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("代理审批已添加！"));
            loadData();
        } else {
            QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("添加代理审批失败！"));
        }
    }
}

void ProxyApproverConfigPage::onEditClicked()
{
    int currentRow = m_proxyTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请先选择要编辑的代理！"));
        return;
    }

    int proxyId = m_proxyTable->item(currentRow, 0)->data(Qt::UserRole).toInt();

    ProxyApproverService& service = ProxyApproverService::getInstance();
    ProxyApprover proxy = service.getProxy(proxyId);

    if (proxy.getId() <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("错误"), QString::fromUtf8("未找到该代理配置！"));
        return;
    }

    ProxyApproverDialog dialog(this, proxyId);
    dialog.setUsers(m_users);
    dialog.setSecurityLevels(m_securityLevels);
    dialog.setProxy(proxy);

    if (dialog.exec() == QDialog::Accepted) {
        ProxyApprover updatedProxy = dialog.getProxy();

        QString validationError = service.validateProxy(updatedProxy);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, QString::fromUtf8("验证失败"), validationError);
            return;
        }

        if (service.hasConflict(updatedProxy, proxyId)) {
            QMessageBox::warning(this, QString::fromUtf8("警告"),
                QString::fromUtf8("已存在冲突的代理配置！"));
            return;
        }

        if (service.updateProxy(updatedProxy)) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("代理审批已更新！"));
            loadData();
        } else {
            QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("更新代理审批失败！"));
        }
    }
}

void ProxyApproverConfigPage::onDeleteClicked()
{
    int currentRow = m_proxyTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请先选择要删除的代理！"));
        return;
    }

    int proxyId = m_proxyTable->item(currentRow, 0)->data(Qt::UserRole).toInt();
    QString proxyName = m_proxyTable->item(currentRow, 1)->text();

    int ret = QMessageBox::question(this, QString::fromUtf8("确认删除"),
        QString::fromUtf8("确定要删除代理人 '%1' 吗？").arg(proxyName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        ProxyApproverService& service = ProxyApproverService::getInstance();

        if (service.deleteProxy(proxyId)) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("代理审批已删除！"));
            loadData();
        } else {
            QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("删除代理审批失败！"));
        }
    }
}

void ProxyApproverConfigPage::onToggleEnabledClicked()
{
    int currentRow = m_proxyTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请先选择要操作的代理！"));
        return;
    }

    int proxyId = m_proxyTable->item(currentRow, 0)->data(Qt::UserRole).toInt();

    ProxyApproverService& service = ProxyApproverService::getInstance();
    ProxyApprover proxy = service.getProxy(proxyId);

    if (proxy.getId() <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("错误"), QString::fromUtf8("未找到该代理配置！"));
        return;
    }

    bool success;
    if (proxy.isEnabled()) {
        success = service.disableProxy(proxyId);
        if (success) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("代理已禁用！"));
        }
    } else {
        success = service.enableProxy(proxyId);
        if (success) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("代理已启用！"));
        }
    }

    if (success) {
        loadData();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("操作失败！"));
    }
}

void ProxyApproverConfigPage::onRefreshClicked()
{
    loadData();
}

void ProxyApproverConfigPage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0) {
        onEditClicked();
    }
}
