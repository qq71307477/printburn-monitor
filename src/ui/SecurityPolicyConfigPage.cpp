#include "SecurityPolicyConfigPage.h"
#include <QHeaderView>
#include <QSettings>
#include <QCoreApplication>

SecurityPolicyConfigPage::SecurityPolicyConfigPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_authGroup(nullptr)
    , m_authAttemptsSpin(nullptr)
    , m_lockDurationCombo(nullptr)
    , m_passwordGroup(nullptr)
    , m_requireUpperCase(nullptr)
    , m_requireLowerCase(nullptr)
    , m_requireDigit(nullptr)
    , m_requireSpecial(nullptr)
    , m_passwordLengthSpin(nullptr)
    , m_passwordExpirySpin(nullptr)
    , m_sessionGroup(nullptr)
    , m_sessionTimeoutSpin(nullptr)
    , m_saveButton(nullptr)
    , m_resetButton(nullptr)
{
    setupUI();
    loadSettings();
}

void SecurityPolicyConfigPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    m_titleLabel = new QLabel("安全策略配置", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_mainLayout->addWidget(m_titleLabel);

    // Description label
    QLabel* descLabel = new QLabel("配置系统登录相关的安全策略，包括身份鉴别、口令策略和会话超时设置。", this);
    descLabel->setStyleSheet("color: #666; margin-bottom: 10px;");
    m_mainLayout->addWidget(descLabel);

    // Group 1: Authentication Settings
    setupAuthGroup();
    m_mainLayout->addWidget(m_authGroup);

    // Group 2: Password Policy Settings
    setupPasswordGroup();
    m_mainLayout->addWidget(m_passwordGroup);

    // Group 3: Session Timeout Settings
    setupSessionGroup();
    m_mainLayout->addWidget(m_sessionGroup);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_saveButton = new QPushButton("保存", this);
    m_saveButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px 20px;"
        "  border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #45a049;"
        "}"
    );

    m_resetButton = new QPushButton("重置", this);
    m_resetButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #f0f0f0;"
        "  color: #333;"
        "  border: 1px solid #ccc;"
        "  padding: 8px 20px;"
        "  border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #e0e0e0;"
        "}"
    );

    connect(m_saveButton, &QPushButton::clicked, this, &SecurityPolicyConfigPage::onSaveClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &SecurityPolicyConfigPage::onResetClicked);

    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_resetButton);

    m_mainLayout->addLayout(buttonLayout);
    m_mainLayout->addStretch();
}

void SecurityPolicyConfigPage::setupAuthGroup()
{
    m_authGroup = new QGroupBox("身份鉴别设置", this);
    m_authGroup->setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  margin-top: 12px;"
        "  padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 5px;"
        "}"
    );

    QFormLayout* formLayout = new QFormLayout(m_authGroup);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // Authentication attempts
    m_authAttemptsSpin = new QSpinBox(this);
    m_authAttemptsSpin->setRange(1, 5);
    m_authAttemptsSpin->setValue(5);
    m_authAttemptsSpin->setSuffix(" 次");
    formLayout->addRow("身份鉴别尝试次数:", m_authAttemptsSpin);

    // Lock duration
    m_lockDurationCombo = new QComboBox(this);
    m_lockDurationCombo->addItem("10分钟", 10);
    m_lockDurationCombo->addItem("15分钟", 15);
    m_lockDurationCombo->addItem("30分钟", 30);
    m_lockDurationCombo->addItem("1小时", 60);
    m_lockDurationCombo->addItem("2小时", 120);
    m_lockDurationCombo->setCurrentIndex(0); // Default: 10 minutes
    formLayout->addRow("锁定时长:", m_lockDurationCombo);

    // Add description labels
    QLabel* authAttemptsDesc = new QLabel("范围: 1-5次，达到次数后账户将被锁定", this);
    authAttemptsDesc->setStyleSheet("color: #999; font-size: 12px;");
    formLayout->addRow("", authAttemptsDesc);

    QLabel* lockDurationDesc = new QLabel("账户锁定后需要等待的解锁时间", this);
    lockDurationDesc->setStyleSheet("color: #999; font-size: 12px;");
    formLayout->addRow("", lockDurationDesc);
}

void SecurityPolicyConfigPage::setupPasswordGroup()
{
    m_passwordGroup = new QGroupBox("口令策略设置", this);
    m_passwordGroup->setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  margin-top: 12px;"
        "  padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 5px;"
        "}"
    );

    QFormLayout* formLayout = new QFormLayout(m_passwordGroup);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // Password complexity checkboxes
    QVBoxLayout* complexityLayout = new QVBoxLayout();
    m_requireUpperCase = new QCheckBox("大写字母 (A-Z)", this);
    m_requireLowerCase = new QCheckBox("小写字母 (a-z)", this);
    m_requireDigit = new QCheckBox("数字 (0-9)", this);
    m_requireSpecial = new QCheckBox("特殊字符 (!@#$%^&*等)", this);

    // Default: all checked
    m_requireUpperCase->setChecked(true);
    m_requireLowerCase->setChecked(true);
    m_requireDigit->setChecked(true);
    m_requireSpecial->setChecked(true);

    complexityLayout->addWidget(m_requireUpperCase);
    complexityLayout->addWidget(m_requireLowerCase);
    complexityLayout->addWidget(m_requireDigit);
    complexityLayout->addWidget(m_requireSpecial);

    formLayout->addRow("口令复杂度要求:", complexityLayout);

    // Password length
    m_passwordLengthSpin = new QSpinBox(this);
    m_passwordLengthSpin->setRange(8, 20);
    m_passwordLengthSpin->setValue(10);
    m_passwordLengthSpin->setSuffix(" 位");
    formLayout->addRow("用户口令长度:", m_passwordLengthSpin);

    // Password expiry
    m_passwordExpirySpin = new QSpinBox(this);
    m_passwordExpirySpin->setRange(1, 7);
    m_passwordExpirySpin->setValue(7);
    m_passwordExpirySpin->setSuffix(" 天");
    formLayout->addRow("口令更换周期:", m_passwordExpirySpin);

    // Add description labels
    QLabel* complexityDesc = new QLabel("新密码必须包含选定的字符类型", this);
    complexityDesc->setStyleSheet("color: #999; font-size: 12px;");
    formLayout->addRow("", complexityDesc);

    QLabel* lengthDesc = new QLabel("范围: 8-20位", this);
    lengthDesc->setStyleSheet("color: #999; font-size: 12px;");
    formLayout->addRow("", lengthDesc);

    QLabel* expiryDesc = new QLabel("范围: 1-7天，到期后需要更换密码", this);
    expiryDesc->setStyleSheet("color: #999; font-size: 12px;");
    formLayout->addRow("", expiryDesc);
}

void SecurityPolicyConfigPage::setupSessionGroup()
{
    m_sessionGroup = new QGroupBox("会话超时设置", this);
    m_sessionGroup->setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  margin-top: 12px;"
        "  padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 5px;"
        "}"
    );

    QFormLayout* formLayout = new QFormLayout(m_sessionGroup);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // Session timeout
    m_sessionTimeoutSpin = new QSpinBox(this);
    m_sessionTimeoutSpin->setRange(5, 15);
    m_sessionTimeoutSpin->setValue(5);
    m_sessionTimeoutSpin->setSuffix(" 分钟");
    formLayout->addRow("会话超时认证:", m_sessionTimeoutSpin);

    // Add description label
    QLabel* timeoutDesc = new QLabel("范围: 5-15分钟，无操作达到此时长后需要重新认证", this);
    timeoutDesc->setStyleSheet("color: #999; font-size: 12px;");
    formLayout->addRow("", timeoutDesc);
}

void SecurityPolicyConfigPage::loadSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup("SecurityPolicy");

    // Load authentication settings
    int authAttempts = settings.value("authAttempts", 5).toInt();
    m_authAttemptsSpin->setValue(authAttempts);

    int lockDuration = settings.value("lockDuration", 10).toInt();
    int lockIndex = m_lockDurationCombo->findData(lockDuration);
    if (lockIndex >= 0) {
        m_lockDurationCombo->setCurrentIndex(lockIndex);
    }

    // Load password complexity settings
    m_requireUpperCase->setChecked(settings.value("requireUpperCase", true).toBool());
    m_requireLowerCase->setChecked(settings.value("requireLowerCase", true).toBool());
    m_requireDigit->setChecked(settings.value("requireDigit", true).toBool());
    m_requireSpecial->setChecked(settings.value("requireSpecial", true).toBool());

    // Load password settings
    m_passwordLengthSpin->setValue(settings.value("passwordLength", 10).toInt());
    m_passwordExpirySpin->setValue(settings.value("passwordExpiry", 7).toInt());

    // Load session settings
    m_sessionTimeoutSpin->setValue(settings.value("sessionTimeout", 5).toInt());

    settings.endGroup();
}

void SecurityPolicyConfigPage::saveSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup("SecurityPolicy");

    // Save authentication settings
    settings.setValue("authAttempts", m_authAttemptsSpin->value());
    settings.setValue("lockDuration", m_lockDurationCombo->currentData().toInt());

    // Save password complexity settings
    settings.setValue("requireUpperCase", m_requireUpperCase->isChecked());
    settings.setValue("requireLowerCase", m_requireLowerCase->isChecked());
    settings.setValue("requireDigit", m_requireDigit->isChecked());
    settings.setValue("requireSpecial", m_requireSpecial->isChecked());

    // Save password settings
    settings.setValue("passwordLength", m_passwordLengthSpin->value());
    settings.setValue("passwordExpiry", m_passwordExpirySpin->value());

    // Save session settings
    settings.setValue("sessionTimeout", m_sessionTimeoutSpin->value());

    settings.endGroup();
    settings.sync();
}

bool SecurityPolicyConfigPage::validateInput()
{
    // Validate password complexity - at least one option must be selected
    if (!m_requireUpperCase->isChecked() &&
        !m_requireLowerCase->isChecked() &&
        !m_requireDigit->isChecked() &&
        !m_requireSpecial->isChecked()) {
        QMessageBox::warning(this, "验证错误",
            "口令复杂度要求至少选择一项！");
        return false;
    }

    return true;
}

void SecurityPolicyConfigPage::onSaveClicked()
{
    if (!validateInput()) {
        return;
    }

    saveSettings();
    QMessageBox::information(this, "保存成功", "安全策略配置已保存！");
}

void SecurityPolicyConfigPage::onResetClicked()
{
    int ret = QMessageBox::question(this, "确认重置",
        "确定要重置为默认配置吗？\n\n"
        "这将恢复以下默认值:\n"
        "- 身份鉴别尝试次数: 5次\n"
        "- 锁定时长: 10分钟\n"
        "- 口令复杂度: 全部启用\n"
        "- 口令长度: 10位\n"
        "- 口令更换周期: 7天\n"
        "- 会话超时: 5分钟",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // Reset to defaults
        m_authAttemptsSpin->setValue(5);
        m_lockDurationCombo->setCurrentIndex(0);

        m_requireUpperCase->setChecked(true);
        m_requireLowerCase->setChecked(true);
        m_requireDigit->setChecked(true);
        m_requireSpecial->setChecked(true);

        m_passwordLengthSpin->setValue(10);
        m_passwordExpirySpin->setValue(7);

        m_sessionTimeoutSpin->setValue(5);

        // Save the reset values
        saveSettings();

        QMessageBox::information(this, "重置成功", "安全策略配置已重置为默认值！");
    }
}
