#include "UserManagementPage.h"
#include <QHeaderView>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QCryptographicHash>
#include "src/common/repository/department_repository.h"
#include "src/common/repository/role_repository.h"

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

// UserEditDialog Implementation
UserEditDialog::UserEditDialog(QWidget* parent, const User& user)
    : QDialog(parent)
    , m_user(user)
    , m_isEdit(user.getId() > 0)
    , m_usernameEdit(nullptr)
    , m_firstNameEdit(nullptr)
    , m_lastNameEdit(nullptr)
    , m_emailEdit(nullptr)
    , m_phoneEdit(nullptr)
    , m_cardNumberEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_departmentCombo(nullptr)
    , m_securityLevelCombo(nullptr)
    , m_activeCheck(nullptr)
    , m_rolesList(nullptr)
    , m_displayOrderSpin(nullptr)
{
    setWindowTitle(m_isEdit ? "编辑用户" : "新增用户");
    setModal(true);
    resize(500, 550);
    setupUI();
}

void UserEditDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

    // Username
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setText(m_user.getUsername());
    m_usernameEdit->setPlaceholderText("输入登录账号");
    if (m_isEdit) {
        m_usernameEdit->setReadOnly(true);
        m_usernameEdit->setStyleSheet("background-color: #f0f0f0;");
    }
    formLayout->addRow("登录账号:*", m_usernameEdit);

    // Password (only for new user)
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(m_isEdit ? "不修改请留空" : "输入密码");
    formLayout->addRow(m_isEdit ? "新密码:" : "密码:*", m_passwordEdit);

    // First Name
    m_firstNameEdit = new QLineEdit(this);
    m_firstNameEdit->setText(m_user.getFirstName());
    m_firstNameEdit->setPlaceholderText("输入用户姓名");
    formLayout->addRow("用户姓名:*", m_firstNameEdit);

    // Last Name
    m_lastNameEdit = new QLineEdit(this);
    m_lastNameEdit->setText(m_user.getLastName());
    m_lastNameEdit->setPlaceholderText("输入姓氏（可选）");
    formLayout->addRow("姓氏:", m_lastNameEdit);

    // Email
    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setText(m_user.getEmail());
    m_emailEdit->setPlaceholderText("输入邮箱地址");
    formLayout->addRow("邮箱:", m_emailEdit);

    // Phone
    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setText(m_user.getPhone());
    m_phoneEdit->setPlaceholderText("输入联系电话");
    formLayout->addRow("联系电话:", m_phoneEdit);

    // Card Number
    m_cardNumberEdit = new QLineEdit(this);
    m_cardNumberEdit->setPlaceholderText("输入卡号");
    formLayout->addRow("卡号:", m_cardNumberEdit);

    // Department
    m_departmentCombo = new QComboBox(this);
    loadDepartments();
    formLayout->addRow("所属组织:", m_departmentCombo);

    // Security Level
    m_securityLevelCombo = new QComboBox(this);
    m_securityLevelCombo->addItem("请选择", 0);
    m_securityLevelCombo->addItem("绝密", 1);
    m_securityLevelCombo->addItem("机密", 2);
    m_securityLevelCombo->addItem("秘密", 3);
    m_securityLevelCombo->addItem("内部", 4);
    m_securityLevelCombo->addItem("公开", 5);
    formLayout->addRow("密级:", m_securityLevelCombo);

    // Display Order
    m_displayOrderSpin = new QSpinBox(this);
    m_displayOrderSpin->setRange(0, 9999);
    m_displayOrderSpin->setValue(0);
    formLayout->addRow("显示顺序:", m_displayOrderSpin);

    // Is Active
    m_activeCheck = new QCheckBox("启用", this);
    m_activeCheck->setChecked(m_isEdit ? m_user.isActive() : true);
    formLayout->addRow("状态:", m_activeCheck);

    mainLayout->addLayout(formLayout);

    // Roles Section
    QLabel* rolesLabel = new QLabel("用户角色:", this);
    mainLayout->addWidget(rolesLabel);

    m_rolesList = new QListWidget(this);
    m_rolesList->setSelectionMode(QAbstractItemView::MultiSelection);
    loadRoles();
    mainLayout->addWidget(m_rolesList);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* saveButton = new QPushButton("保存", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    connect(saveButton, &QPushButton::clicked, this, &UserEditDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &UserEditDialog::onCancelClicked);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void UserEditDialog::loadDepartments()
{
    // Get departments from repository
    // For now, add some default options
    m_departmentCombo->addItem("请选择", 0);
    m_departmentCombo->addItem("总部", 1);
    m_departmentCombo->addItem("研发部", 2);
    m_departmentCombo->addItem("市场部", 3);
    m_departmentCombo->addItem("财务部", 4);
    m_departmentCombo->addItem("人事部", 5);

    // Select user's department if editing
    if (m_isEdit && m_user.getDepartmentId() > 0) {
        int index = m_departmentCombo->findData(m_user.getDepartmentId());
        if (index >= 0) {
            m_departmentCombo->setCurrentIndex(index);
        }
    }
}

void UserEditDialog::loadRoles()
{
    RoleRepository roleRepo;
    QList<Role> roles = roleRepo.findAll();

    if (roles.isEmpty()) {
        // Add default roles if none exist
        m_rolesList->addItem("普通用户");
        m_rolesList->addItem("任务审批员");
        m_rolesList->addItem("任务管理员");
        m_rolesList->addItem("部门管理员");
        m_rolesList->addItem("回收员");
    } else {
        for (const Role& role : roles) {
            QListWidgetItem* item = new QListWidgetItem(role.getName());
            item->setData(Qt::UserRole, role.getId());
            m_rolesList->addItem(item);
        }
    }

    // Select user's current roles if editing
    if (m_isEdit && m_user.getId() > 0) {
        UserManagementService& service = UserManagementService::getInstance();
        QList<Role> userRoles = service.getUserRoles(m_user.getId());
        for (int i = 0; i < m_rolesList->count(); ++i) {
            QListWidgetItem* item = m_rolesList->item(i);
            int roleId = item->data(Qt::UserRole).toInt();
            for (const Role& userRole : userRoles) {
                if (userRole.getId() == roleId) {
                    item->setSelected(true);
                    break;
                }
            }
        }
    }
}

void UserEditDialog::onSaveClicked()
{
    // Validate input
    if (m_usernameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "登录账号不能为空！");
        m_usernameEdit->setFocus();
        return;
    }

    if (m_firstNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "用户姓名不能为空！");
        m_firstNameEdit->setFocus();
        return;
    }

    if (!m_isEdit && m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "密码不能为空！");
        m_passwordEdit->setFocus();
        return;
    }

    m_user.setUsername(m_usernameEdit->text().trimmed());
    if (!m_passwordEdit->text().isEmpty()) {
        m_user.setPassword(m_passwordEdit->text());
    }
    m_user.setFirstName(m_firstNameEdit->text().trimmed());
    m_user.setLastName(m_lastNameEdit->text().trimmed());
    m_user.setEmail(m_emailEdit->text().trimmed());
    m_user.setPhone(m_phoneEdit->text().trimmed());
    m_user.setDepartmentId(m_departmentCombo->currentData().toInt());
    m_user.setActive(m_activeCheck->isChecked());

    // Store selected roles in user data for processing
    QVariantList selectedRoleIds;
    for (int i = 0; i < m_rolesList->count(); ++i) {
        if (m_rolesList->item(i)->isSelected()) {
            selectedRoleIds.append(m_rolesList->item(i)->data(Qt::UserRole).toInt());
        }
    }
    m_user.setRoleId(selectedRoleIds.isEmpty() ? 0 : selectedRoleIds.first().toInt());

    accept();
}

void UserEditDialog::onCancelClicked()
{
    reject();
}

User UserEditDialog::getUser() const
{
    return m_user;
}

// ImportUsersDialog Implementation
ImportUsersDialog::ImportUsersDialog(QWidget* parent)
    : QDialog(parent)
    , m_filePathEdit(nullptr)
{
    setWindowTitle("批量导入用户");
    setModal(true);
    resize(500, 150);
    setupUI();
}

void ImportUsersDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* fileLayout = new QHBoxLayout();

    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setReadOnly(true);
    m_filePathEdit->setPlaceholderText("请选择要导入的Excel文件");
    fileLayout->addWidget(m_filePathEdit);

    QPushButton* browseButton = new QPushButton("浏览...", this);
    connect(browseButton, &QPushButton::clicked, this, &ImportUsersDialog::onBrowseClicked);
    fileLayout->addWidget(browseButton);

    mainLayout->addLayout(fileLayout);

    // Template download link
    QLabel* templateLabel = new QLabel("<a href=\"#\">下载导入模板</a>", this);
    templateLabel->setTextFormat(Qt::RichText);
    templateLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    mainLayout->addWidget(templateLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* importButton = new QPushButton("导入", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    connect(importButton, &QPushButton::clicked, this, &ImportUsersDialog::onImportClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ImportUsersDialog::onCancelClicked);

    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void ImportUsersDialog::onBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择导入文件", "",
        "Excel文件 (*.xlsx *.xls);;CSV文件 (*.csv);;所有文件 (*.*)");
    if (!filePath.isEmpty()) {
        m_filePath = filePath;
        m_filePathEdit->setText(filePath);
    }
}

void ImportUsersDialog::onImportClicked()
{
    if (m_filePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要导入的文件！");
        return;
    }
    accept();
}

void ImportUsersDialog::onCancelClicked()
{
    reject();
}

QString ImportUsersDialog::getFilePath() const
{
    return m_filePath;
}

// ResetPasswordDialog Implementation
ResetPasswordDialog::ResetPasswordDialog(QWidget* parent, const QString& username)
    : QDialog(parent)
    , m_username(username)
    , m_newPasswordEdit(nullptr)
    , m_confirmPasswordEdit(nullptr)
{
    setWindowTitle("重置密码");
    setModal(true);
    resize(400, 200);
    setupUI();
}

void ResetPasswordDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Show username
    QLabel* usernameLabel = new QLabel(QString("正在重置用户: %1").arg(m_username), this);
    usernameLabel->setStyleSheet("font-weight: bold; color: #333;");
    mainLayout->addWidget(usernameLabel);

    QFormLayout* formLayout = new QFormLayout();

    m_newPasswordEdit = new QLineEdit(this);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setPlaceholderText("输入新密码");
    formLayout->addRow("新密码:*", m_newPasswordEdit);

    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setPlaceholderText("再次输入新密码");
    formLayout->addRow("确认密码:*", m_confirmPasswordEdit);

    mainLayout->addLayout(formLayout);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* confirmButton = new QPushButton("确认", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    connect(confirmButton, &QPushButton::clicked, this, &ResetPasswordDialog::onConfirmClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ResetPasswordDialog::onCancelClicked);

    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void ResetPasswordDialog::onConfirmClicked()
{
    QString newPassword = m_newPasswordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();

    if (newPassword.isEmpty()) {
        QMessageBox::warning(this, "验证错误", "新密码不能为空！");
        m_newPasswordEdit->setFocus();
        return;
    }

    if (confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "验证错误", "确认密码不能为空！");
        m_confirmPasswordEdit->setFocus();
        return;
    }

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "验证错误", "两次输入的密码不一致！");
        m_confirmPasswordEdit->setFocus();
        return;
    }

    accept();
}

void ResetPasswordDialog::onCancelClicked()
{
    reject();
}

QString ResetPasswordDialog::getNewPassword() const
{
    return m_newPasswordEdit->text();
}

// UserManagementPage Implementation
UserManagementPage::UserManagementPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_searchGroup(nullptr)
    , m_keywordEdit(nullptr)
    , m_departmentFilterCombo(nullptr)
    , m_statusFilterCombo(nullptr)
    , m_roleFilterCombo(nullptr)
    , m_startTimeEdit(nullptr)
    , m_endTimeEdit(nullptr)
    , m_userTable(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
    , m_resetPasswordButton(nullptr)
    , m_batchImportButton(nullptr)
    , m_batchExportButton(nullptr)
{
    setupUI();
    loadData();
}

void UserManagementPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("用户管理", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_mainLayout->addWidget(m_titleLabel);

    // Search Area
    setupSearchArea();

    // Toolbar
    setupToolbar();

    // Table
    setupTable();

    // Status label
    QLabel* statusLabel = new QLabel("提示: 双击表格行可直接编辑用户，三员账号（admin等）不能删除。", this);
    statusLabel->setStyleSheet("color: #999; font-size: 12px;");
    m_mainLayout->addWidget(statusLabel);

    m_mainLayout->addStretch();
}

void UserManagementPage::setupSearchArea()
{
    m_searchGroup = new QGroupBox("搜索条件", this);
    QVBoxLayout* searchMainLayout = new QVBoxLayout(m_searchGroup);

    QHBoxLayout* row1Layout = new QHBoxLayout();

    // Keyword
    row1Layout->addWidget(new QLabel("关键字:", this));
    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText("用户姓名、账号");
    row1Layout->addWidget(m_keywordEdit);

    // Department
    row1Layout->addWidget(new QLabel("部门:", this));
    m_departmentFilterCombo = new QComboBox(this);
    m_departmentFilterCombo->addItem("全部", 0);
    m_departmentFilterCombo->addItem("总部", 1);
    m_departmentFilterCombo->addItem("研发部", 2);
    m_departmentFilterCombo->addItem("市场部", 3);
    m_departmentFilterCombo->addItem("财务部", 4);
    m_departmentFilterCombo->addItem("人事部", 5);
    row1Layout->addWidget(m_departmentFilterCombo);

    // Status
    row1Layout->addWidget(new QLabel("状态:", this));
    m_statusFilterCombo = new QComboBox(this);
    m_statusFilterCombo->addItem("全部", "");
    m_statusFilterCombo->addItem("启用", "ACTIVE");
    m_statusFilterCombo->addItem("禁用", "INACTIVE");
    row1Layout->addWidget(m_statusFilterCombo);

    // Role
    row1Layout->addWidget(new QLabel("角色:", this));
    m_roleFilterCombo = new QComboBox(this);
    m_roleFilterCombo->addItem("全部", 0);
    m_roleFilterCombo->addItem("普通用户", 1);
    m_roleFilterCombo->addItem("任务审批员", 2);
    m_roleFilterCombo->addItem("任务管理员", 3);
    m_roleFilterCombo->addItem("部门管理员", 4);
    m_roleFilterCombo->addItem("回收员", 5);
    row1Layout->addWidget(m_roleFilterCombo);

    searchMainLayout->addLayout(row1Layout);

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

    // Search and Reset buttons
    QPushButton* searchButton = new QPushButton("查询", this);
    QPushButton* resetButton = new QPushButton("重置", this);
    connect(searchButton, &QPushButton::clicked, this, &UserManagementPage::onSearchClicked);
    connect(resetButton, &QPushButton::clicked, this, &UserManagementPage::onResetClicked);
    row2Layout->addWidget(searchButton);
    row2Layout->addWidget(resetButton);

    searchMainLayout->addLayout(row2Layout);

    m_mainLayout->addWidget(m_searchGroup);
}

void UserManagementPage::setupTable()
{
    m_userTable = new QTableWidget(0, 9, this);
    m_userTable->setHorizontalHeaderLabels({"序号", "用户姓名", "登录账号", "卡号", "组织", "用户角色", "创建时间", "状态", "操作"});
    m_userTable->horizontalHeader()->setStretchLastSection(true);
    m_userTable->verticalHeader()->setVisible(false);
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set column widths
    m_userTable->setColumnWidth(0, 50);
    m_userTable->setColumnWidth(1, 100);
    m_userTable->setColumnWidth(2, 100);
    m_userTable->setColumnWidth(3, 80);
    m_userTable->setColumnWidth(4, 80);
    m_userTable->setColumnWidth(5, 100);
    m_userTable->setColumnWidth(6, 120);
    m_userTable->setColumnWidth(7, 60);
    m_userTable->setColumnWidth(8, 100);

    connect(m_userTable, &QTableWidget::cellDoubleClicked, this, &UserManagementPage::onTableDoubleClicked);

    m_mainLayout->addWidget(m_userTable);
}

void UserManagementPage::setupToolbar()
{
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->addStretch();

    m_addButton = new QPushButton("新增", this);
    m_editButton = new QPushButton("编辑", this);
    m_deleteButton = new QPushButton("删除", this);
    m_resetPasswordButton = new QPushButton("重置密码", this);
    m_batchImportButton = new QPushButton("批量导入", this);
    m_batchExportButton = new QPushButton("批量导出", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_addButton, &QPushButton::clicked, this, &UserManagementPage::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &UserManagementPage::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &UserManagementPage::onDeleteClicked);
    connect(m_resetPasswordButton, &QPushButton::clicked, this, &UserManagementPage::onResetPasswordClicked);
    connect(m_batchImportButton, &QPushButton::clicked, this, &UserManagementPage::onBatchImportClicked);
    connect(m_batchExportButton, &QPushButton::clicked, this, &UserManagementPage::onBatchExportClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &UserManagementPage::onRefreshClicked);

    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addWidget(m_resetPasswordButton);
    toolbarLayout->addWidget(m_batchImportButton);
    toolbarLayout->addWidget(m_batchExportButton);
    toolbarLayout->addWidget(m_refreshButton);

    m_mainLayout->addLayout(toolbarLayout);
}

void UserManagementPage::loadData()
{
    UserManagementService& service = UserManagementService::getInstance();
    QList<User> users = service.getAllUsers();
    refreshTable(users);
}

void UserManagementPage::refreshTable(const QList<User>& users)
{
    m_userTable->setRowCount(0);

    for (int i = 0; i < users.count(); ++i) {
        const User& user = users[i];

        m_userTable->insertRow(i);

        // 序号
        m_userTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        // 用户姓名
        m_userTable->setItem(i, 1, new QTableWidgetItem(user.getFirstName()));
        // 登录账号
        m_userTable->setItem(i, 2, new QTableWidgetItem(user.getUsername()));
        // 卡号
        m_userTable->setItem(i, 3, new QTableWidgetItem(""));
        // 组织
        m_userTable->setItem(i, 4, new QTableWidgetItem(getDepartmentName(user.getDepartmentId())));
        // 用户角色
        m_userTable->setItem(i, 5, new QTableWidgetItem(getRoleNames(user.getId())));
        // 创建时间
        m_userTable->setItem(i, 6, new QTableWidgetItem(user.getCreateTime().toString("yyyy-MM-dd hh:mm")));
        // 状态
        m_userTable->setItem(i, 7, new QTableWidgetItem(user.isActive() ? "启用" : "禁用"));
        // 操作
        m_userTable->setItem(i, 8, new QTableWidgetItem("编辑/删除"));

        // Store user ID in the row
        m_userTable->item(i, 0)->setData(Qt::UserRole, user.getId());

        // Gray out inactive rows
        if (!user.isActive()) {
            for (int col = 0; col < m_userTable->columnCount(); ++col) {
                QTableWidgetItem* item = m_userTable->item(i, col);
                if (item) {
                    item->setForeground(QBrush(Qt::gray));
                }
            }
        }
    }
}

User UserManagementPage::getSelectedUser() const
{
    User user;

    QList<QTableWidgetItem*> selectedItems = m_userTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return user;
    }

    int row = selectedItems.first()->row();
    int userId = m_userTable->item(row, 0)->data(Qt::UserRole).toInt();

    UserManagementService& service = UserManagementService::getInstance();
    return service.getUserById(userId);
}

QString UserManagementPage::getDepartmentName(int departmentId) const
{
    // Return department name by ID
    switch (departmentId) {
        case 1: return "总部";
        case 2: return "研发部";
        case 3: return "市场部";
        case 4: return "财务部";
        case 5: return "人事部";
        default: return "未知";
    }
}

QString UserManagementPage::getRoleNames(int userId) const
{
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

void UserManagementPage::onAddClicked()
{
    UserEditDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        User newUser = dialog.getUser();

        UserManagementService& service = UserManagementService::getInstance();

        // Use operator ID 1 (current admin) for now
        if (service.createUser(newUser, 1)) {
            QMessageBox::information(this, "成功", "用户添加成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "用户添加失败，请检查账号是否已存在。");
        }
    }
}

void UserManagementPage::onEditClicked()
{
    User user = getSelectedUser();
    if (user.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的用户！");
        return;
    }

    UserEditDialog dialog(this, user);

    if (dialog.exec() == QDialog::Accepted) {
        User updatedUser = dialog.getUser();

        UserManagementService& service = UserManagementService::getInstance();

        if (service.updateUser(updatedUser, 1)) {
            QMessageBox::information(this, "成功", "用户更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "用户更新失败。");
        }
    }
}

void UserManagementPage::onDeleteClicked()
{
    User user = getSelectedUser();
    if (user.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的用户！");
        return;
    }

    // Check if it's a system user (三员账号)
    QString username = user.getUsername();
    if (username == "admin" || username == "security" || username == "auditor") {
        QMessageBox::warning(this, "提示", "三员账号不能删除！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除用户 '%1' 吗？").arg(user.getFirstName()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        UserManagementService& service = UserManagementService::getInstance();
        if (service.deleteUser(user.getId(), 1)) {
            QMessageBox::information(this, "成功", "用户已删除！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "删除失败。");
        }
    }
}

void UserManagementPage::onRefreshClicked()
{
    loadData();
}

void UserManagementPage::onSearchClicked()
{
    UserManagementService& service = UserManagementService::getInstance();
    QList<User> users = service.getAllUsers();

    QString keyword = m_keywordEdit->text().trimmed();
    QString status = m_statusFilterCombo->currentData().toString();
    int departmentId = m_departmentFilterCombo->currentData().toInt();
    int roleId = m_roleFilterCombo->currentData().toInt();
    QDateTime startTime = m_startTimeEdit->dateTime();
    QDateTime endTime = m_endTimeEdit->dateTime();

    QList<User> filteredUsers;
    for (const User& user : users) {
        bool match = true;

        // Keyword filter
        if (!keyword.isEmpty()) {
            if (!user.getUsername().contains(keyword, Qt::CaseInsensitive) &&
                !user.getFirstName().contains(keyword, Qt::CaseInsensitive)) {
                match = false;
            }
        }

        // Status filter
        if (!status.isEmpty() && user.getStatus() != status) {
            match = false;
        }

        // Department filter
        if (departmentId > 0 && user.getDepartmentId() != departmentId) {
            match = false;
        }

        // Time range filter
        if (user.getCreateTime() < startTime || user.getCreateTime() > endTime) {
            match = false;
        }

        if (match) {
            filteredUsers.append(user);
        }
    }

    refreshTable(filteredUsers);
}

void UserManagementPage::onResetClicked()
{
    m_keywordEdit->clear();
    m_departmentFilterCombo->setCurrentIndex(0);
    m_statusFilterCombo->setCurrentIndex(0);
    m_roleFilterCombo->setCurrentIndex(0);
    m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addMonths(-1));
    m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
    loadData();
}

void UserManagementPage::onResetPasswordClicked()
{
    User user = getSelectedUser();
    if (user.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要重置密码的用户！");
        return;
    }

    ResetPasswordDialog dialog(this, user.getUsername());

    if (dialog.exec() == QDialog::Accepted) {
        QString newPassword = dialog.getNewPassword();

        UserManagementService& service = UserManagementService::getInstance();
        if (service.resetUserPassword(user.getId(), newPassword, 1)) {
            QMessageBox::information(this, "成功", "密码重置成功！");
        } else {
            QMessageBox::critical(this, "错误", "密码重置失败。");
        }
    }
}

void UserManagementPage::onBatchImportClicked()
{
    ImportUsersDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString filePath = dialog.getFilePath();
        QMessageBox::information(this, "导入", "批量导入功能需要进一步实现Excel解析。\n文件: " + filePath);
    }
}

void UserManagementPage::onBatchExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出用户", "users.csv",
        "CSV文件 (*.csv);;Excel文件 (*.xlsx);;JSON文件 (*.json)");

    if (filePath.isEmpty()) {
        return;
    }

    UserManagementService& service = UserManagementService::getInstance();

    // Get all user IDs for export
    QList<User> users = service.getAllUsers();
    QList<int> userIds;
    for (const User& user : users) {
        userIds.append(user.getId());
    }

    if (service.bulkExportUsers(userIds, filePath)) {
        QMessageBox::information(this, "成功", "用户导出成功！\n文件: " + filePath);
    } else {
        QMessageBox::critical(this, "错误", "导出失败。");
    }
}

void UserManagementPage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    int userId = m_userTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (userId <= 0) {
        return;
    }

    UserManagementService& service = UserManagementService::getInstance();
    User user = service.getUserById(userId);

    if (user.getId() <= 0) {
        return;
    }

    UserEditDialog dialog(this, user);

    if (dialog.exec() == QDialog::Accepted) {
        User updatedUser = dialog.getUser();

        if (service.updateUser(updatedUser, 1)) {
            QMessageBox::information(this, "成功", "用户更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "用户更新失败。");
        }
    }
}
