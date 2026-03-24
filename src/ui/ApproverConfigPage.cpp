#include "ApproverConfigPage.h"
#include "services/ApproverConfigService.h"
#include "src/common/repository/role_repository.h"
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

// ================== ApproverConfigDialog ==================

ApproverConfigDialog::ApproverConfigDialog(QWidget *parent, int configId)
    : QDialog(parent)
    , m_configId(configId)
{
    setWindowTitle(configId < 0 ? QString::fromUtf8("添加审批配置") : QString::fromUtf8("编辑审批配置"));
    setupUI();
    resize(400, 250);
}

void ApproverConfigDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout();

    // 任务类型
    m_taskTypeCombo = new QComboBox(this);
    m_taskTypeCombo->addItem(QString::fromUtf8("打印"), 0);
    m_taskTypeCombo->addItem(QString::fromUtf8("刻录"), 1);
    formLayout->addRow(QString::fromUtf8("任务类型:"), m_taskTypeCombo);

    // 审批角色
    m_approverRoleCombo = new QComboBox(this);
    formLayout->addRow(QString::fromUtf8("审批角色:"), m_approverRoleCombo);

    // 最小密级
    m_minSecurityLevelSpin = new QSpinBox(this);
    m_minSecurityLevelSpin->setRange(0, 9999);
    m_minSecurityLevelSpin->setValue(0);
    formLayout->addRow(QString::fromUtf8("最小密级:"), m_minSecurityLevelSpin);

    // 最大密级
    m_maxSecurityLevelSpin = new QSpinBox(this);
    m_maxSecurityLevelSpin->setRange(0, 9999);
    m_maxSecurityLevelSpin->setValue(9999);
    formLayout->addRow(QString::fromUtf8("最大密级:"), m_maxSecurityLevelSpin);

    // 是否激活
    m_activeCheck = new QCheckBox(QString::fromUtf8("启用此配置"), this);
    m_activeCheck->setChecked(true);
    formLayout->addRow(m_activeCheck);

    layout->addLayout(formLayout);

    // 按钮
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        // 验证输入
        if (m_approverRoleCombo->currentIndex() < 0) {
            QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请选择审批角色！"));
            return;
        }
        if (m_minSecurityLevelSpin->value() > m_maxSecurityLevelSpin->value()) {
            QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("最小密级不能大于最大密级！"));
            return;
        }
        accept();
    });
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttonBox);
}

ApproverConfig ApproverConfigDialog::getConfig() const
{
    ApproverConfig config;
    config.setId(m_configId < 0 ? 0 : m_configId);
    config.setTaskType(m_taskTypeCombo->currentData().toInt());
    config.setApproverRoleId(m_approverRoleCombo->currentData().toInt());
    config.setMinSecurityLevelId(m_minSecurityLevelSpin->value());
    config.setMaxSecurityLevelId(m_maxSecurityLevelSpin->value());
    config.setActive(m_activeCheck->isChecked());
    return config;
}

void ApproverConfigDialog::setConfig(const ApproverConfig& config)
{
    m_configId = config.getId();
    int index = m_taskTypeCombo->findData(config.getTaskType());
    if (index >= 0) {
        m_taskTypeCombo->setCurrentIndex(index);
    }
    int roleIndex = m_approverRoleCombo->findData(config.getApproverRoleId());
    if (roleIndex >= 0) {
        m_approverRoleCombo->setCurrentIndex(roleIndex);
    }
    m_minSecurityLevelSpin->setValue(config.getMinSecurityLevelId());
    m_maxSecurityLevelSpin->setValue(config.getMaxSecurityLevelId());
    m_activeCheck->setChecked(config.isActive());
}

void ApproverConfigDialog::setApproverRoles(const QList<QPair<int, QString>>& roles)
{
    m_approverRoleCombo->clear();
    for (const auto& role : roles) {
        m_approverRoleCombo->addItem(role.second, role.first);
    }
}

// ================== ApproverConfigPage ==================

ApproverConfigPage::ApproverConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_configTable(nullptr)
{
    setupUI();
    loadData();
}

void ApproverConfigPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    // Header
    m_headerLayout = new QHBoxLayout();
    m_pageTitle = new QLabel(QString::fromUtf8("审批员配置"), this);
    m_pageTitle->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_headerLayout->addWidget(m_pageTitle);
    m_headerLayout->addStretch();

    m_refreshButton = new QPushButton(QString::fromUtf8("刷新"), this);
    connect(m_refreshButton, &QPushButton::clicked, this, &ApproverConfigPage::onRefreshClicked);
    m_headerLayout->addWidget(m_refreshButton);

    m_layout->addLayout(m_headerLayout);

    // Table
    setupTable();

    // Button area
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();

    m_addButton = new QPushButton(QString::fromUtf8("添加"), this);
    m_editButton = new QPushButton(QString::fromUtf8("编辑"), this);
    m_deleteButton = new QPushButton(QString::fromUtf8("删除"), this);

    connect(m_addButton, &QPushButton::clicked, this, &ApproverConfigPage::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &ApproverConfigPage::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &ApproverConfigPage::onDeleteClicked);

    m_buttonLayout->addWidget(m_addButton);
    m_buttonLayout->addWidget(m_editButton);
    m_buttonLayout->addWidget(m_deleteButton);

    m_layout->addLayout(m_buttonLayout);

    m_layout->addStretch();
}

void ApproverConfigPage::setupTable()
{
    m_configTable = new QTableWidget(0, 6, this);
    m_configTable->setHorizontalHeaderLabels({
        QString::fromUtf8("ID"),
        QString::fromUtf8("任务类型"),
        QString::fromUtf8("审批角色"),
        QString::fromUtf8("密级范围"),
        QString::fromUtf8("状态"),
        QString::fromUtf8("更新时间")
    });

    m_configTable->horizontalHeader()->setStretchLastSection(true);
    m_configTable->verticalHeader()->setVisible(false);
    m_configTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_configTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_configTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置列宽
    m_configTable->setColumnWidth(0, 50);
    m_configTable->setColumnWidth(1, 100);
    m_configTable->setColumnWidth(2, 150);
    m_configTable->setColumnWidth(3, 150);
    m_configTable->setColumnWidth(4, 80);

    connect(m_configTable, &QTableWidget::cellDoubleClicked,
            this, &ApproverConfigPage::onTableDoubleClicked);

    m_layout->addWidget(m_configTable);
}

void ApproverConfigPage::loadData()
{
    m_approverRoles = loadApproverRoles();

    ApproverConfigService& service = ApproverConfigService::getInstance();
    m_configs = service.getAllConfigs();

    updateTable();
}

void ApproverConfigPage::updateTable()
{
    m_configTable->setRowCount(0);

    for (int i = 0; i < m_configs.size(); ++i) {
        const ApproverConfig& config = m_configs[i];
        m_configTable->insertRow(i);

        m_configTable->setItem(i, 0, new QTableWidgetItem(QString::number(config.getId())));
        m_configTable->setItem(i, 1, new QTableWidgetItem(config.getTaskTypeName()));
        m_configTable->setItem(i, 2, new QTableWidgetItem(getRoleName(config.getApproverRoleId())));

        QString securityRange = QString::fromUtf8("%1 - %2")
            .arg(config.getMinSecurityLevelId())
            .arg(config.getMaxSecurityLevelId());
        m_configTable->setItem(i, 3, new QTableWidgetItem(securityRange));

        QString status = config.isActive() ? QString::fromUtf8("启用") : QString::fromUtf8("停用");
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(config.isActive() ? QColor(Qt::darkGreen) : QColor(Qt::red));
        m_configTable->setItem(i, 4, statusItem);

        m_configTable->setItem(i, 5, new QTableWidgetItem(config.getUpdatedAt().toString("yyyy-MM-dd hh:mm")));
    }
}

QList<QPair<int, QString>> ApproverConfigPage::loadApproverRoles()
{
    ApproverConfigService& service = ApproverConfigService::getInstance();
    return service.getApproverRoles();
}

QString ApproverConfigPage::getRoleName(int roleId) const
{
    for (const auto& role : m_approverRoles) {
        if (role.first == roleId) {
            return role.second;
        }
    }
    return QString::fromUtf8("未知角色(%1)").arg(roleId);
}

void ApproverConfigPage::onAddClicked()
{
    ApproverConfigDialog dialog(this);
    dialog.setApproverRoles(m_approverRoles);

    if (dialog.exec() == QDialog::Accepted) {
        ApproverConfig config = dialog.getConfig();

        ApproverConfigService& service = ApproverConfigService::getInstance();
        QString validationError = service.validateConfig(config);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, QString::fromUtf8("验证失败"), validationError);
            return;
        }

        if (service.hasDuplicateConfig(config)) {
            QMessageBox::warning(this, QString::fromUtf8("警告"),
                QString::fromUtf8("已存在相同的审批配置！"));
            return;
        }

        // TODO: 获取当前用户ID
        int currentUserId = 1;
        if (service.createConfig(config, currentUserId)) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("审批配置已添加！"));
            loadData();
        } else {
            QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("添加审批配置失败！"));
        }
    }
}

void ApproverConfigPage::onEditClicked()
{
    int currentRow = m_configTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请先选择要编辑的配置！"));
        return;
    }

    int configId = m_configTable->item(currentRow, 0)->text().toInt();

    ApproverConfigService& service = ApproverConfigService::getInstance();
    ApproverConfig config = service.getConfig(configId);

    if (config.getId() <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("错误"), QString::fromUtf8("未找到该配置！"));
        return;
    }

    ApproverConfigDialog dialog(this, configId);
    dialog.setApproverRoles(m_approverRoles);
    dialog.setConfig(config);

    if (dialog.exec() == QDialog::Accepted) {
        ApproverConfig updatedConfig = dialog.getConfig();

        QString validationError = service.validateConfig(updatedConfig);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, QString::fromUtf8("验证失败"), validationError);
            return;
        }

        if (service.hasDuplicateConfig(updatedConfig, configId)) {
            QMessageBox::warning(this, QString::fromUtf8("警告"),
                QString::fromUtf8("已存在相同的审批配置！"));
            return;
        }

        // TODO: 获取当前用户ID
        int currentUserId = 1;
        if (service.updateConfig(updatedConfig, currentUserId)) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("审批配置已更新！"));
            loadData();
        } else {
            QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("更新审批配置失败！"));
        }
    }
}

void ApproverConfigPage::onDeleteClicked()
{
    int currentRow = m_configTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请先选择要删除的配置！"));
        return;
    }

    int configId = m_configTable->item(currentRow, 0)->text().toInt();
    QString taskType = m_configTable->item(currentRow, 1)->text();
    QString roleName = m_configTable->item(currentRow, 2)->text();

    int ret = QMessageBox::question(this, QString::fromUtf8("确认删除"),
        QString::fromUtf8("确定要删除以下审批配置吗？\n\n任务类型: %1\n审批角色: %2")
            .arg(taskType)
            .arg(roleName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        ApproverConfigService& service = ApproverConfigService::getInstance();

        // TODO: 获取当前用户ID
        int currentUserId = 1;
        if (service.deleteConfig(configId, currentUserId)) {
            QMessageBox::information(this, QString::fromUtf8("成功"), QString::fromUtf8("审批配置已删除！"));
            loadData();
        } else {
            QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("删除审批配置失败！"));
        }
    }
}

void ApproverConfigPage::onRefreshClicked()
{
    loadData();
}

void ApproverConfigPage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0) {
        onEditClicked();
    }
}
