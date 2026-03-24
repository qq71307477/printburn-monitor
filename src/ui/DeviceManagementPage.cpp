#include "DeviceManagementPage.h"
#include <QHeaderView>
#include <QDateTime>
#include <QBrush>
#include "src/common/repository/user_repository.h"
#include "src/common/repository/department_repository.h"
#include "qt_compat.h"

// DeviceEditDialog Implementation
DeviceEditDialog::DeviceEditDialog(QWidget* parent, const Device& device)
    : QDialog(parent)
    , m_device(device)
    , m_isEdit(device.getId() > 0)
    , m_nameEdit(nullptr)
    , m_serialNumberEdit(nullptr)
    , m_deviceTypeCombo(nullptr)
    , m_ipAddressEdit(nullptr)
    , m_locationEdit(nullptr)
    , m_specificationsEdit(nullptr)
    , m_statusCombo(nullptr)
    , m_monitoredCheck(nullptr)
{
    setWindowTitle(m_isEdit ? "编辑设备" : "新增设备");
    setModal(true);
    resize(500, 450);
    setupUI();
}

void DeviceEditDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

    // Device Name
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(m_device.getName());
    m_nameEdit->setPlaceholderText("输入设备名称");
    formLayout->addRow("设备名称:*", m_nameEdit);

    // Serial Number
    m_serialNumberEdit = new QLineEdit(this);
    m_serialNumberEdit->setText(m_device.getSerialNumber());
    m_serialNumberEdit->setPlaceholderText("输入设备编号");
    formLayout->addRow("设备编号:*", m_serialNumberEdit);

    // Device Type
    m_deviceTypeCombo = new QComboBox(this);
    m_deviceTypeCombo->addItem("请选择", "");
    m_deviceTypeCombo->addItem("打印机", "printer");
    m_deviceTypeCombo->addItem("刻录机", "burner");
    m_deviceTypeCombo->addItem("扫描仪", "scanner");
    m_deviceTypeCombo->addItem("复印机", "copier");
    m_deviceTypeCombo->addItem("其他", "other");

    // Select current device type if editing
    if (m_isEdit && !m_device.getDeviceType().isEmpty()) {
        int index = m_deviceTypeCombo->findData(m_device.getDeviceType());
        if (index >= 0) {
            m_deviceTypeCombo->setCurrentIndex(index);
        }
    }
    formLayout->addRow("设备类型:*", m_deviceTypeCombo);

    // IP Address
    m_ipAddressEdit = new QLineEdit(this);
    m_ipAddressEdit->setText(m_device.getIpAddress());
    m_ipAddressEdit->setPlaceholderText("输入IP地址 (如: 192.168.1.100)");
    formLayout->addRow("IP地址:", m_ipAddressEdit);

    // Location
    m_locationEdit = new QLineEdit(this);
    m_locationEdit->setText(m_device.getLocation());
    m_locationEdit->setPlaceholderText("输入设备位置");
    formLayout->addRow("位置:", m_locationEdit);

    // Status
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem("可用", "available");
    m_statusCombo->addItem("使用中", "in_use");
    m_statusCombo->addItem("维护中", "maintenance");
    m_statusCombo->addItem("离线", "offline");

    // Select current status if editing
    if (m_isEdit && !m_device.getStatus().isEmpty()) {
        int index = m_statusCombo->findData(m_device.getStatus());
        if (index >= 0) {
            m_statusCombo->setCurrentIndex(index);
        }
    }
    formLayout->addRow("状态:", m_statusCombo);

    // Monitored
    m_monitoredCheck = new QCheckBox("启用监控", this);
    m_monitoredCheck->setChecked(m_isEdit ? m_device.isMonitored() : true);
    formLayout->addRow("监控:", m_monitoredCheck);

    mainLayout->addLayout(formLayout);

    // Specifications
    QLabel* specsLabel = new QLabel("设备规格:", this);
    mainLayout->addWidget(specsLabel);

    m_specificationsEdit = new QTextEdit(this);
    m_specificationsEdit->setText(m_device.getSpecifications());
    m_specificationsEdit->setPlaceholderText("输入设备规格参数 (可选)");
    m_specificationsEdit->setMaximumHeight(100);
    mainLayout->addWidget(m_specificationsEdit);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* saveButton = new QPushButton("保存", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    connect(saveButton, &QPushButton::clicked, this, &DeviceEditDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DeviceEditDialog::onCancelClicked);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void DeviceEditDialog::onSaveClicked()
{
    // Validate input
    if (m_nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "设备名称不能为空！");
        m_nameEdit->setFocus();
        return;
    }

    if (m_serialNumberEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "设备编号不能为空！");
        m_serialNumberEdit->setFocus();
        return;
    }

    if (m_deviceTypeCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "请选择设备类型！");
        m_deviceTypeCombo->setFocus();
        return;
    }

    m_device.setName(m_nameEdit->text().trimmed());
    m_device.setSerialNumber(m_serialNumberEdit->text().trimmed());
    m_device.setDeviceType(m_deviceTypeCombo->currentData().toString());
    m_device.setIpAddress(m_ipAddressEdit->text().trimmed());
    m_device.setLocation(m_locationEdit->text().trimmed());
    m_device.setStatus(m_statusCombo->currentData().toString());
    m_device.setMonitored(m_monitoredCheck->isChecked());
    m_device.setSpecifications(m_specificationsEdit->toPlainText());
    m_device.setLastUpdated(QDateTime::currentDateTime());

    accept();
}

void DeviceEditDialog::onCancelClicked()
{
    reject();
}

Device DeviceEditDialog::getDevice() const
{
    return m_device;
}

// DeviceAuthorizationDialog Implementation
DeviceAuthorizationDialog::DeviceAuthorizationDialog(QWidget* parent, int deviceId, const QString& deviceName)
    : QDialog(parent)
    , m_deviceId(deviceId)
    , m_deviceName(deviceName)
    , m_tabWidget(nullptr)
    , m_usersList(nullptr)
    , m_departmentsTree(nullptr)
{
    setWindowTitle(QString("设备授权 - %1").arg(deviceName));
    setModal(true);
    resize(500, 450);
    setupUI();
    loadCurrentAuthorizations();
}

void DeviceAuthorizationDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Device info
    QLabel* infoLabel = new QLabel(QString("正在为设备: %1 设置授权").arg(m_deviceName), this);
    infoLabel->setStyleSheet("font-weight: bold; color: #333; margin-bottom: 10px;");
    mainLayout->addWidget(infoLabel);

    // Tab widget for user/department authorization
    m_tabWidget = new QTabWidget(this);

    // User authorization tab
    QWidget* userTab = new QWidget();
    QVBoxLayout* userLayout = new QVBoxLayout(userTab);

    QLabel* userHintLabel = new QLabel("选择可使用此设备的用户:", this);
    userLayout->addWidget(userHintLabel);

    m_usersList = new QListWidget(this);
    m_usersList->setSelectionMode(QAbstractItemView::MultiSelection);
    userLayout->addWidget(m_usersList);

    m_tabWidget->addTab(userTab, "用户授权");

    // Department authorization tab
    QWidget* deptTab = new QWidget();
    QVBoxLayout* deptLayout = new QVBoxLayout(deptTab);

    QLabel* deptHintLabel = new QLabel("选择可使用此设备的部门:", this);
    deptLayout->addWidget(deptHintLabel);

    m_departmentsTree = new QTreeWidget(this);
    m_departmentsTree->setHeaderLabel("部门列表");
    m_departmentsTree->setSelectionMode(QAbstractItemView::MultiSelection);
    deptLayout->addWidget(m_departmentsTree);

    m_tabWidget->addTab(deptTab, "部门授权");

    mainLayout->addWidget(m_tabWidget);

    // Load data
    loadUsers();
    loadDepartments();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* saveButton = new QPushButton("保存", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    connect(saveButton, &QPushButton::clicked, this, &DeviceAuthorizationDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DeviceAuthorizationDialog::onCancelClicked);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void DeviceAuthorizationDialog::loadUsers()
{
    UserRepository userRepo;
    QList<User> users = userRepo.findAll();

    for (const User& user : users) {
        if (!user.isActive()) {
            continue;  // Skip inactive users
        }

        QListWidgetItem* item = new QListWidgetItem(
            QString("%1 (%2)").arg(user.getFirstName()).arg(user.getUsername()));
        item->setData(Qt::UserRole, user.getId());
        m_usersList->addItem(item);
    }
}

void DeviceAuthorizationDialog::loadDepartments()
{
    DepartmentRepository deptRepo;
    std::vector<std::unique_ptr<Department>> departments = deptRepo.find_all();

    // Create a flat list for now (can be enhanced to tree structure)
    for (const auto& dept : departments) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_departmentsTree);
        item->setText(0, QString::fromStdString(dept->name));
        item->setData(0, Qt::UserRole, dept->id);
    }

    // Add some default departments if none exist
    if (m_departmentsTree->topLevelItemCount() == 0) {
        QStringList defaultDepts = {"总部", "研发部", "市场部", "财务部", "人事部"};
        for (int i = 0; i < defaultDepts.size(); ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_departmentsTree);
            item->setText(0, defaultDepts[i]);
            item->setData(0, Qt::UserRole, i + 1);
        }
    }
}

void DeviceAuthorizationDialog::loadCurrentAuthorizations()
{
    // For now, no pre-loaded authorizations
    // This can be enhanced to load from a device_authorization table
}

void DeviceAuthorizationDialog::onSaveClicked()
{
    m_authorizedUserIds.clear();
    m_authorizedDepartmentIds.clear();

    // Get selected users
    for (int i = 0; i < m_usersList->count(); ++i) {
        QListWidgetItem* item = m_usersList->item(i);
        if (item->isSelected()) {
            m_authorizedUserIds.append(item->data(Qt::UserRole).toInt());
        }
    }

    // Get selected departments
    QList<QTreeWidgetItem*> selectedDepts = m_departmentsTree->selectedItems();
    for (QTreeWidgetItem* item : selectedDepts) {
        m_authorizedDepartmentIds.append(item->data(0, Qt::UserRole).toInt());
    }

    accept();
}

void DeviceAuthorizationDialog::onCancelClicked()
{
    reject();
}

void DeviceAuthorizationDialog::onTabChanged(int index)
{
    Q_UNUSED(index)
    // Tab changed - can be used to refresh data if needed
}

QList<int> DeviceAuthorizationDialog::getAuthorizedUserIds() const
{
    return m_authorizedUserIds;
}

QList<int> DeviceAuthorizationDialog::getAuthorizedDepartmentIds() const
{
    return m_authorizedDepartmentIds;
}

// DeviceManagementPage Implementation
DeviceManagementPage::DeviceManagementPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_searchGroup(nullptr)
    , m_keywordEdit(nullptr)
    , m_deviceTypeFilterCombo(nullptr)
    , m_statusFilterCombo(nullptr)
    , m_deviceTable(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_authorizeButton(nullptr)
    , m_refreshButton(nullptr)
{
    setupUI();
    loadData();
}

void DeviceManagementPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("设备管理", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_mainLayout->addWidget(m_titleLabel);

    // Search Area
    setupSearchArea();

    // Toolbar
    setupToolbar();

    // Table
    setupTable();

    // Status label
    QLabel* statusLabel = new QLabel("提示: 双击表格行可直接编辑设备信息。", this);
    statusLabel->setStyleSheet("color: #999; font-size: 12px;");
    m_mainLayout->addWidget(statusLabel);

    m_mainLayout->addStretch();
}

void DeviceManagementPage::setupSearchArea()
{
    m_searchGroup = new QGroupBox("搜索条件", this);
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);

    // Keyword
    searchLayout->addWidget(new QLabel("关键字:", this));
    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText("设备名称、编号、IP地址");
    searchLayout->addWidget(m_keywordEdit);

    // Device Type
    searchLayout->addWidget(new QLabel("设备类型:", this));
    m_deviceTypeFilterCombo = new QComboBox(this);
    m_deviceTypeFilterCombo->addItem("全部", "");
    m_deviceTypeFilterCombo->addItem("打印机", "printer");
    m_deviceTypeFilterCombo->addItem("刻录机", "burner");
    m_deviceTypeFilterCombo->addItem("扫描仪", "scanner");
    m_deviceTypeFilterCombo->addItem("复印机", "copier");
    m_deviceTypeFilterCombo->addItem("其他", "other");
    searchLayout->addWidget(m_deviceTypeFilterCombo);

    // Status
    searchLayout->addWidget(new QLabel("状态:", this));
    m_statusFilterCombo = new QComboBox(this);
    m_statusFilterCombo->addItem("全部", "");
    m_statusFilterCombo->addItem("可用", "available");
    m_statusFilterCombo->addItem("使用中", "in_use");
    m_statusFilterCombo->addItem("维护中", "maintenance");
    m_statusFilterCombo->addItem("离线", "offline");
    searchLayout->addWidget(m_statusFilterCombo);

    searchLayout->addStretch();

    // Search and Reset buttons
    QPushButton* searchButton = new QPushButton("查询", this);
    QPushButton* resetButton = new QPushButton("重置", this);
    connect(searchButton, &QPushButton::clicked, this, &DeviceManagementPage::onSearchClicked);
    connect(resetButton, &QPushButton::clicked, this, &DeviceManagementPage::onResetClicked);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(resetButton);

    m_mainLayout->addWidget(m_searchGroup);
}

void DeviceManagementPage::setupTable()
{
    m_deviceTable = new QTableWidget(0, 7, this);
    m_deviceTable->setHorizontalHeaderLabels({"序号", "设备名称", "设备编号", "设备类型", "IP地址", "状态", "操作"});
    m_deviceTable->horizontalHeader()->setStretchLastSection(true);
    m_deviceTable->verticalHeader()->setVisible(false);
    m_deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deviceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_deviceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set column widths
    m_deviceTable->setColumnWidth(0, 50);   // 序号
    m_deviceTable->setColumnWidth(1, 150);  // 设备名称
    m_deviceTable->setColumnWidth(2, 120);  // 设备编号
    m_deviceTable->setColumnWidth(3, 80);   // 设备类型
    m_deviceTable->setColumnWidth(4, 120);  // IP地址
    m_deviceTable->setColumnWidth(5, 80);   // 状态
    m_deviceTable->setColumnWidth(6, 150);  // 操作

    connect(m_deviceTable, &QTableWidget::cellDoubleClicked, this, &DeviceManagementPage::onTableDoubleClicked);

    m_mainLayout->addWidget(m_deviceTable);
}

void DeviceManagementPage::setupToolbar()
{
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->addStretch();

    m_addButton = new QPushButton("新增", this);
    m_editButton = new QPushButton("编辑", this);
    m_deleteButton = new QPushButton("删除", this);
    m_authorizeButton = new QPushButton("授权", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_addButton, &QPushButton::clicked, this, &DeviceManagementPage::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &DeviceManagementPage::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &DeviceManagementPage::onDeleteClicked);
    connect(m_authorizeButton, &QPushButton::clicked, this, &DeviceManagementPage::onAuthorizeClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &DeviceManagementPage::onRefreshClicked);

    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addWidget(m_authorizeButton);
    toolbarLayout->addWidget(m_refreshButton);

    m_mainLayout->addLayout(toolbarLayout);
}

void DeviceManagementPage::loadData()
{
    DeviceManagementService& service = DeviceManagementService::getInstance();
    QList<Device> devices = service.getAllDevices();
    refreshTable(devices);
}

void DeviceManagementPage::refreshTable(const QList<Device>& devices)
{
    m_deviceTable->setRowCount(0);

    for (int i = 0; i < devices.count(); ++i) {
        const Device& device = devices[i];

        m_deviceTable->insertRow(i);

        // 序号
        m_deviceTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        // 设备名称
        m_deviceTable->setItem(i, 1, new QTableWidgetItem(device.getName()));
        // 设备编号
        m_deviceTable->setItem(i, 2, new QTableWidgetItem(device.getSerialNumber()));
        // 设备类型
        m_deviceTable->setItem(i, 3, new QTableWidgetItem(getDeviceTypeDisplay(device.getDeviceType())));
        // IP地址
        m_deviceTable->setItem(i, 4, new QTableWidgetItem(device.getIpAddress()));
        // 状态
        m_deviceTable->setItem(i, 5, new QTableWidgetItem(getStatusDisplay(device.getStatus())));
        // 操作
        m_deviceTable->setItem(i, 6, new QTableWidgetItem("编辑/删除/授权"));

        // Store device ID in the row
        m_deviceTable->item(i, 0)->setData(Qt::UserRole, device.getId());

        // Color code by status
        QString status = device.getStatus();
        QColor statusColor;
        if (status == "available") {
            statusColor = QColor("#28a745");  // Green
        } else if (status == "in_use") {
            statusColor = QColor("#007bff");  // Blue
        } else if (status == "maintenance") {
            statusColor = QColor("#ffc107");  // Yellow
        } else if (status == "offline") {
            statusColor = QColor("#dc3545");  // Red
        } else {
            statusColor = QColor("#6c757d");  // Gray
        }

        QTableWidgetItem* statusItem = m_deviceTable->item(i, 5);
        if (statusItem) {
            statusItem->setForeground(QBrush(statusColor));
        }
    }
}

Device DeviceManagementPage::getSelectedDevice() const
{
    Device device;

    QList<QTableWidgetItem*> selectedItems = m_deviceTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return device;
    }

    int row = selectedItems.first()->row();
    int deviceId = m_deviceTable->item(row, 0)->data(Qt::UserRole).toInt();

    DeviceManagementService& service = DeviceManagementService::getInstance();
    return service.getDeviceById(deviceId);
}

QString DeviceManagementPage::getDeviceTypeDisplay(const QString& deviceType) const
{
    if (deviceType == "printer") return "打印机";
    if (deviceType == "burner") return "刻录机";
    if (deviceType == "scanner") return "扫描仪";
    if (deviceType == "copier") return "复印机";
    if (deviceType == "other") return "其他";
    return deviceType;
}

QString DeviceManagementPage::getStatusDisplay(const QString& status) const
{
    if (status == "available") return "可用";
    if (status == "in_use") return "使用中";
    if (status == "maintenance") return "维护中";
    if (status == "offline") return "离线";
    return status;
}

void DeviceManagementPage::onAddClicked()
{
    DeviceEditDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        Device newDevice = dialog.getDevice();

        DeviceManagementService& service = DeviceManagementService::getInstance();

        if (service.addDevice(newDevice)) {
            QMessageBox::information(this, "成功", "设备添加成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "设备添加失败，请检查设备编号是否已存在。");
        }
    }
}

void DeviceManagementPage::onEditClicked()
{
    Device device = getSelectedDevice();
    if (device.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的设备！");
        return;
    }

    DeviceEditDialog dialog(this, device);

    if (dialog.exec() == QDialog::Accepted) {
        Device updatedDevice = dialog.getDevice();

        DeviceManagementService& service = DeviceManagementService::getInstance();

        if (service.updateDevice(updatedDevice)) {
            QMessageBox::information(this, "成功", "设备更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "设备更新失败。");
        }
    }
}

void DeviceManagementPage::onDeleteClicked()
{
    Device device = getSelectedDevice();
    if (device.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的设备！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除设备 '%1' 吗？").arg(device.getName()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        DeviceManagementService& service = DeviceManagementService::getInstance();
        if (service.deleteDevice(device.getId())) {
            QMessageBox::information(this, "成功", "设备已删除！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "删除失败。");
        }
    }
}

void DeviceManagementPage::onAuthorizeClicked()
{
    Device device = getSelectedDevice();
    if (device.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要授权的设备！");
        return;
    }

    DeviceAuthorizationDialog dialog(this, device.getId(), device.getName());

    if (dialog.exec() == QDialog::Accepted) {
        QList<int> userIds = dialog.getAuthorizedUserIds();
        QList<int> deptIds = dialog.getAuthorizedDepartmentIds();

        // TODO: Save authorizations to database
        // For now, just show a success message
        QString message = QString("设备授权设置成功！\n授权用户数: %1\n授权部门数: %2")
            .arg(userIds.count())
            .arg(deptIds.count());
        QMessageBox::information(this, "授权成功", message);
    }
}

void DeviceManagementPage::onRefreshClicked()
{
    loadData();
}

void DeviceManagementPage::onSearchClicked()
{
    DeviceManagementService& service = DeviceManagementService::getInstance();
    QList<Device> devices = service.getAllDevices();

    QString keyword = m_keywordEdit->text().trimmed();
    QString deviceType = m_deviceTypeFilterCombo->currentData().toString();
    QString status = m_statusFilterCombo->currentData().toString();

    QList<Device> filteredDevices;
    for (const Device& device : devices) {
        bool match = true;

        // Keyword filter
        if (!keyword.isEmpty()) {
            if (!device.getName().contains(keyword, Qt::CaseInsensitive) &&
                !device.getSerialNumber().contains(keyword, Qt::CaseInsensitive) &&
                !device.getIpAddress().contains(keyword, Qt::CaseInsensitive)) {
                match = false;
            }
        }

        // Device type filter
        if (!deviceType.isEmpty() && device.getDeviceType() != deviceType) {
            match = false;
        }

        // Status filter
        if (!status.isEmpty() && device.getStatus() != status) {
            match = false;
        }

        if (match) {
            filteredDevices.append(device);
        }
    }

    refreshTable(filteredDevices);
}

void DeviceManagementPage::onResetClicked()
{
    m_keywordEdit->clear();
    m_deviceTypeFilterCombo->setCurrentIndex(0);
    m_statusFilterCombo->setCurrentIndex(0);
    loadData();
}

void DeviceManagementPage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    int deviceId = m_deviceTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (deviceId <= 0) {
        return;
    }

    DeviceManagementService& service = DeviceManagementService::getInstance();
    Device device = service.getDeviceById(deviceId);

    if (device.getId() <= 0) {
        return;
    }

    DeviceEditDialog dialog(this, device);

    if (dialog.exec() == QDialog::Accepted) {
        Device updatedDevice = dialog.getDevice();

        if (service.updateDevice(updatedDevice)) {
            QMessageBox::information(this, "成功", "设备更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "设备更新失败。");
        }
    }
}