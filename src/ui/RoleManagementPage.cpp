#include "RoleManagementPage.h"
#include "qt_compat.h"
#include <QHeaderView>
#include <QDateTime>
#include <QTextEdit>

// RoleEditDialog Implementation
RoleEditDialog::RoleEditDialog(QWidget* parent, const Role& role)
    : QDialog(parent)
    , m_role(role)
    , m_isEdit(role.getId() > 0)
    , m_nameEdit(nullptr)
    , m_displayOrderSpin(nullptr)
    , m_activeCheck(nullptr)
    , m_permissionTree(nullptr)
    , m_descriptionEdit(nullptr)
{
    setWindowTitle(m_isEdit ? "编辑角色" : "新增角色");
    setModal(true);
    resize(500, 600);
    setupUI();
}

void RoleEditDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

    // Role Name
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(m_role.getName());
    m_nameEdit->setPlaceholderText("输入角色名称");
    if (m_isEdit && m_role.getIsSystem()) {
        m_nameEdit->setReadOnly(true);
        m_nameEdit->setStyleSheet("background-color: #f0f0f0;");
    }
    formLayout->addRow("角色名称:*", m_nameEdit);

    // Description
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setText(m_role.getDescription());
    m_descriptionEdit->setPlaceholderText("输入角色描述（可选）");
    m_descriptionEdit->setMaximumHeight(60);
    formLayout->addRow("描述:", m_descriptionEdit);

    // Display Order
    m_displayOrderSpin = new QSpinBox(this);
    m_displayOrderSpin->setRange(0, 9999);
    m_displayOrderSpin->setValue(m_isEdit ? m_role.getId() : 0);
    formLayout->addRow("显示顺序:", m_displayOrderSpin);

    // Is Active
    m_activeCheck = new QCheckBox("启用", this);
    m_activeCheck->setChecked(m_isEdit ? m_role.isActive() : true);
    formLayout->addRow("状态:", m_activeCheck);

    mainLayout->addLayout(formLayout);

    // Permissions Section
    QLabel* permLabel = new QLabel("菜单权限:", this);
    mainLayout->addWidget(permLabel);

    // Permission tree buttons
    QHBoxLayout* permBtnLayout = new QHBoxLayout();
    QPushButton* selectAllBtn = new QPushButton("全选", this);
    QPushButton* clearAllBtn = new QPushButton("清空", this);
    connect(selectAllBtn, &QPushButton::clicked, this, &RoleEditDialog::onSelectAllPermissions);
    connect(clearAllBtn, &QPushButton::clicked, this, &RoleEditDialog::onClearAllPermissions);
    permBtnLayout->addWidget(selectAllBtn);
    permBtnLayout->addWidget(clearAllBtn);
    permBtnLayout->addStretch();
    mainLayout->addLayout(permBtnLayout);

    m_permissionTree = new QTreeWidget(this);
    m_permissionTree->setHeaderHidden(true);
    m_permissionTree->setSelectionMode(QAbstractItemView::MultiSelection);
    setupPermissionTree();
    mainLayout->addWidget(m_permissionTree);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* saveButton = new QPushButton("保存", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    connect(saveButton, &QPushButton::clicked, this, &RoleEditDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &RoleEditDialog::onCancelClicked);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void RoleEditDialog::setupPermissionTree()
{
    // Main modules
    QTreeWidgetItem* printApplyItem = new QTreeWidgetItem(m_permissionTree);
    printApplyItem->setText(0, "打印申请");
    printApplyItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* burnApplyItem = new QTreeWidgetItem(m_permissionTree);
    burnApplyItem->setText(0, "刻录申请");
    burnApplyItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* printOutputItem = new QTreeWidgetItem(m_permissionTree);
    printOutputItem->setText(0, "打印输出");
    printOutputItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* burnOutputItem = new QTreeWidgetItem(m_permissionTree);
    burnOutputItem->setText(0, "刻录输出");
    burnOutputItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* approvalItem = new QTreeWidgetItem(m_permissionTree);
    approvalItem->setText(0, "任务审批");
    approvalItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* taskParamsItem = new QTreeWidgetItem(m_permissionTree);
    taskParamsItem->setText(0, "任务参数");
    taskParamsItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* securityLevelItem = new QTreeWidgetItem(taskParamsItem);
    securityLevelItem->setText(0, "密级设置");
    securityLevelItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* approverConfigItem = new QTreeWidgetItem(taskParamsItem);
    approverConfigItem->setText(0, "审批员配置");
    approverConfigItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* serialNumberItem = new QTreeWidgetItem(taskParamsItem);
    serialNumberItem->setText(0, "打印/刻录流水号设置");
    serialNumberItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* barcodeItem = new QTreeWidgetItem(taskParamsItem);
    barcodeItem->setText(0, "条码设置");
    barcodeItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* taskLogItem = new QTreeWidgetItem(m_permissionTree);
    taskLogItem->setText(0, "任务日志");
    taskLogItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* deviceItem = new QTreeWidgetItem(m_permissionTree);
    deviceItem->setText(0, "设备管理");
    deviceItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* authItem = new QTreeWidgetItem(m_permissionTree);
    authItem->setText(0, "授权管理");
    authItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* taskMgmtItem = new QTreeWidgetItem(m_permissionTree);
    taskMgmtItem->setText(0, "任务管理");
    taskMgmtItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* sysMgmtItem = new QTreeWidgetItem(m_permissionTree);
    sysMgmtItem->setText(0, "系统管理");
    sysMgmtItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* userMgmtItem = new QTreeWidgetItem(sysMgmtItem);
    userMgmtItem->setText(0, "用户管理");
    userMgmtItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* roleMgmtItem = new QTreeWidgetItem(sysMgmtItem);
    roleMgmtItem->setText(0, "角色管理");
    roleMgmtItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* deptMgmtItem = new QTreeWidgetItem(sysMgmtItem);
    deptMgmtItem->setText(0, "组织架构");
    deptMgmtItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* loginLogItem = new QTreeWidgetItem(sysMgmtItem);
    loginLogItem->setText(0, "登录日志");
    loginLogItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* opLogItem = new QTreeWidgetItem(sysMgmtItem);
    opLogItem->setText(0, "操作日志");
    opLogItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem* securityPolicyItem = new QTreeWidgetItem(sysMgmtItem);
    securityPolicyItem->setText(0, "安全策略");
    securityPolicyItem->setCheckState(0, Qt::Unchecked);

    // Load current permissions if editing
    if (m_isEdit) {
        QStringList perms = m_role.getPermissions().split(",", SPLIT_SKIP_EMPTY);
        for (int i = 0; i < m_permissionTree->topLevelItemCount(); ++i) {
            checkPermissionRecursive(m_permissionTree->topLevelItem(i), perms);
        }
    }
}

void RoleEditDialog::checkPermissionRecursive(QTreeWidgetItem* item, const QStringList& perms)
{
    QString text = item->text(0);
    if (perms.contains(text) || perms.contains("*")) {
        item->setCheckState(0, Qt::Checked);
    }
    for (int i = 0; i < item->childCount(); ++i) {
        checkPermissionRecursive(item->child(i), perms);
    }
}

void RoleEditDialog::onSelectAllPermissions()
{
    for (int i = 0; i < m_permissionTree->topLevelItemCount(); ++i) {
        setCheckStateRecursive(m_permissionTree->topLevelItem(i), Qt::Checked);
    }
}

void RoleEditDialog::onClearAllPermissions()
{
    for (int i = 0; i < m_permissionTree->topLevelItemCount(); ++i) {
        setCheckStateRecursive(m_permissionTree->topLevelItem(i), Qt::Unchecked);
    }
}

void RoleEditDialog::setCheckStateRecursive(QTreeWidgetItem* item, Qt::CheckState state)
{
    item->setCheckState(0, state);
    for (int i = 0; i < item->childCount(); ++i) {
        setCheckStateRecursive(item->child(i), state);
    }
}

void RoleEditDialog::onSaveClicked()
{
    if (m_nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "角色名称不能为空！");
        m_nameEdit->setFocus();
        return;
    }

    m_role.setName(m_nameEdit->text().trimmed());
    m_role.setDescription(m_descriptionEdit->toPlainText().trimmed());
    m_role.setActive(m_activeCheck->isChecked());

    // Collect permissions
    QStringList selectedPerms;
    for (int i = 0; i < m_permissionTree->topLevelItemCount(); ++i) {
        collectPermissionsRecursive(m_permissionTree->topLevelItem(i), selectedPerms);
    }
    m_role.setPermissions(selectedPerms.join(","));

    accept();
}

void RoleEditDialog::collectPermissionsRecursive(QTreeWidgetItem* item, QStringList& perms)
{
    if (item->checkState(0) == Qt::Checked) {
        perms.append(item->text(0));
    }
    for (int i = 0; i < item->childCount(); ++i) {
        collectPermissionsRecursive(item->child(i), perms);
    }
}

void RoleEditDialog::onCancelClicked()
{
    reject();
}

Role RoleEditDialog::getRole() const
{
    return m_role;
}

// RoleManagementPage Implementation
RoleManagementPage::RoleManagementPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_keywordEdit(nullptr)
    , m_statusFilterCombo(nullptr)
    , m_roleTable(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
{
    setupUI();
    loadData();
}

void RoleManagementPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("角色管理", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_mainLayout->addWidget(m_titleLabel);

    // Search Area
    setupSearchArea();

    // Toolbar
    setupToolbar();

    // Table
    setupTable();

    // Status label
    QLabel* statusLabel = new QLabel("提示: 双击表格行可直接编辑角色，系统内置角色不能删除。", this);
    statusLabel->setStyleSheet("color: #999; font-size: 12px;");
    m_mainLayout->addWidget(statusLabel);

    m_mainLayout->addStretch();
}

void RoleManagementPage::setupSearchArea()
{
    QHBoxLayout* row1Layout = new QHBoxLayout();

    row1Layout->addWidget(new QLabel("角色名称:", this));
    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText("输入角色名称关键字");
    row1Layout->addWidget(m_keywordEdit);

    row1Layout->addWidget(new QLabel("状态:", this));
    m_statusFilterCombo = new QComboBox(this);
    m_statusFilterCombo->addItem("全部", "");
    m_statusFilterCombo->addItem("启用", "ACTIVE");
    m_statusFilterCombo->addItem("禁用", "INACTIVE");
    row1Layout->addWidget(m_statusFilterCombo);

    row1Layout->addStretch();

    QHBoxLayout* row2Layout = new QHBoxLayout();

    // Time Range
    row2Layout->addWidget(new QLabel("创建时间:", this));
    m_startTimeEdit = new QDateTimeEdit(this);
    m_startTimeEdit->setCalendarPopup(true);
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addMonths(-1));
    row2Layout->addWidget(m_startTimeEdit);

    row2Layout->addWidget(new QLabel("至", this));
    m_endTimeEdit = new QDateTimeEdit(this);
    m_endTimeEdit->setCalendarPopup(true);
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    row2Layout->addWidget(m_endTimeEdit);

    row2Layout->addStretch();

    QPushButton* searchButton = new QPushButton("查询", this);
    QPushButton* resetButton = new QPushButton("重置", this);
    connect(searchButton, &QPushButton::clicked, this, &RoleManagementPage::onSearchClicked);
    connect(resetButton, &QPushButton::clicked, this, &RoleManagementPage::onResetClicked);
    row2Layout->addWidget(searchButton);
    row2Layout->addWidget(resetButton);

    QVBoxLayout* searchMainLayout = new QVBoxLayout();
    searchMainLayout->addLayout(row1Layout);
    searchMainLayout->addLayout(row2Layout);

    m_mainLayout->addLayout(searchMainLayout);
}

void RoleManagementPage::setupToolbar()
{
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->addStretch();

    m_addButton = new QPushButton("新增", this);
    m_editButton = new QPushButton("编辑", this);
    m_deleteButton = new QPushButton("删除", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_addButton, &QPushButton::clicked, this, &RoleManagementPage::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &RoleManagementPage::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &RoleManagementPage::onDeleteClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &RoleManagementPage::onRefreshClicked);

    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addWidget(m_refreshButton);

    m_mainLayout->addLayout(toolbarLayout);
}

void RoleManagementPage::setupTable()
{
    m_roleTable = new QTableWidget(0, 7, this);
    m_roleTable->setHorizontalHeaderLabels({"序号", "角色名称", "状态", "创建时间", "显示顺序", "系统角色", "操作"});
    m_roleTable->horizontalHeader()->setStretchLastSection(true);
    m_roleTable->verticalHeader()->setVisible(false);
    m_roleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_roleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_roleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_roleTable->setColumnWidth(0, 50);
    m_roleTable->setColumnWidth(1, 150);
    m_roleTable->setColumnWidth(2, 60);
    m_roleTable->setColumnWidth(3, 120);
    m_roleTable->setColumnWidth(4, 80);
    m_roleTable->setColumnWidth(5, 80);

    connect(m_roleTable, &QTableWidget::cellDoubleClicked, this, &RoleManagementPage::onTableDoubleClicked);

    m_mainLayout->addWidget(m_roleTable);
}

void RoleManagementPage::loadData()
{
    RoleManagementService& service = RoleManagementService::getInstance();
    QList<Role> roles = service.getAllRoles();
    refreshTable(roles);
}

void RoleManagementPage::refreshTable(const QList<Role>& roles)
{
    m_roleTable->setRowCount(0);

    for (int i = 0; i < roles.count(); ++i) {
        const Role& role = roles[i];

        m_roleTable->insertRow(i);

        m_roleTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_roleTable->setItem(i, 1, new QTableWidgetItem(role.getName()));
        m_roleTable->setItem(i, 2, new QTableWidgetItem(role.isActive() ? "启用" : "禁用"));
        m_roleTable->setItem(i, 3, new QTableWidgetItem(role.getCreateTime().toString("yyyy-MM-dd hh:mm")));
        m_roleTable->setItem(i, 4, new QTableWidgetItem(QString::number(role.getId())));
        m_roleTable->setItem(i, 5, new QTableWidgetItem(role.getIsSystem() ? "是" : "否"));
        m_roleTable->setItem(i, 6, new QTableWidgetItem("编辑/删除"));

        m_roleTable->item(i, 0)->setData(Qt::UserRole, role.getId());

        if (!role.isActive()) {
            for (int col = 0; col < m_roleTable->columnCount(); ++col) {
                QTableWidgetItem* item = m_roleTable->item(i, col);
                if (item) {
                    item->setForeground(QBrush(Qt::gray));
                }
            }
        }
    }
}

Role RoleManagementPage::getSelectedRole() const
{
    Role role;

    QList<QTableWidgetItem*> selectedItems = m_roleTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return role;
    }

    int row = selectedItems.first()->row();
    int roleId = m_roleTable->item(row, 0)->data(Qt::UserRole).toInt();

    RoleManagementService& service = RoleManagementService::getInstance();
    return service.getRoleById(roleId);
}

void RoleManagementPage::onAddClicked()
{
    RoleEditDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        Role newRole = dialog.getRole();

        RoleManagementService& service = RoleManagementService::getInstance();

        if (service.createRole(newRole, 1)) {
            QMessageBox::information(this, "成功", "角色添加成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "角色添加失败。");
        }
    }
}

void RoleManagementPage::onEditClicked()
{
    Role role = getSelectedRole();
    if (role.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的角色！");
        return;
    }

    RoleEditDialog dialog(this, role);

    if (dialog.exec() == QDialog::Accepted) {
        Role updatedRole = dialog.getRole();

        RoleManagementService& service = RoleManagementService::getInstance();

        if (service.updateRole(updatedRole, 1)) {
            QMessageBox::information(this, "成功", "角色更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "角色更新失败。");
        }
    }
}

void RoleManagementPage::onDeleteClicked()
{
    Role role = getSelectedRole();
    if (role.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的角色！");
        return;
    }

    // Check if it's a system role
    if (role.getIsSystem()) {
        QMessageBox::warning(this, "提示", "系统内置角色不能删除！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除角色 '%1' 吗？").arg(role.getName()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        RoleManagementService& service = RoleManagementService::getInstance();
        if (service.deleteRole(role.getId(), 1)) {
            QMessageBox::information(this, "成功", "角色已删除！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "删除失败。");
        }
    }
}

void RoleManagementPage::onRefreshClicked()
{
    loadData();
}

void RoleManagementPage::onSearchClicked()
{
    RoleManagementService& service = RoleManagementService::getInstance();
    QList<Role> roles = service.getAllRoles();

    QString keyword = m_keywordEdit->text().trimmed();
    QString status = m_statusFilterCombo->currentData().toString();
    QDateTime startTime = m_startTimeEdit->dateTime();
    QDateTime endTime = m_endTimeEdit->dateTime();

    QList<Role> filteredRoles;
    for (const Role& role : roles) {
        bool match = true;

        if (!keyword.isEmpty() && !role.getName().contains(keyword, Qt::CaseInsensitive)) {
            match = false;
        }

        if (!status.isEmpty()) {
            bool isActive = (status == "ACTIVE");
            if (role.isActive() != isActive) {
                match = false;
            }
        }

        // Time range filter
        if (role.getCreateTime() < startTime || role.getCreateTime() > endTime) {
            match = false;
        }

        if (match) {
            filteredRoles.append(role);
        }
    }

    refreshTable(filteredRoles);
}

void RoleManagementPage::onResetClicked()
{
    m_keywordEdit->clear();
    m_statusFilterCombo->setCurrentIndex(0);
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addMonths(-1));
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    loadData();
}

void RoleManagementPage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    int roleId = m_roleTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (roleId <= 0) {
        return;
    }

    RoleManagementService& service = RoleManagementService::getInstance();
    Role role = service.getRoleById(roleId);

    if (role.getId() <= 0) {
        return;
    }

    RoleEditDialog dialog(this, role);

    if (dialog.exec() == QDialog::Accepted) {
        Role updatedRole = dialog.getRole();

        if (service.updateRole(updatedRole, 1)) {
            QMessageBox::information(this, "成功", "角色更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "角色更新失败。");
        }
    }
}
