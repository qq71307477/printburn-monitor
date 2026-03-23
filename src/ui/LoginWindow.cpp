#include "LoginWindow.h"
#include "../services/AuthService.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QIcon>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , m_usernameEdit(new QLineEdit(this))
    , m_passwordEdit(new QLineEdit(this))
    , m_loginButton(new QPushButton("登录", this))
    , m_cancelButton(new QPushButton("取消", this))
    , m_rememberMeCheckBox(new QCheckBox("记住账号", this))
    , m_errorLabel(new QLabel("", this))
    , m_togglePasswordButton(new QPushButton("显示", this))
    , m_settings(new QSettings("PrintBurnMonitor", "Client", this))
{
    setupUI();
    loadSavedCredentials();
}

void LoginWindow::setupUI()
{
    setWindowTitle("用户登录 - 打印刻录安全监控系统");
    setModal(true);
    resize(400, 200);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel *titleLabel = new QLabel("打印刻录安全监控系统\n用户登录", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

    // 表单布局
    QFormLayout *formLayout = new QFormLayout();

    m_usernameEdit->setPlaceholderText("请输入用户名");
    formLayout->addRow("用户名:", m_usernameEdit);

    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("请输入密码");

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(m_passwordEdit);
    passwordLayout->addWidget(m_togglePasswordButton);

    formLayout->addRow("密码:", passwordLayout);

    mainLayout->addLayout(formLayout);

    // 记住账号和错误提示
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(m_rememberMeCheckBox);
    bottomLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::attemptLogin);
    connect(m_cancelButton, &QPushButton::clicked, this, &LoginWindow::reject);
    connect(m_togglePasswordButton, &QPushButton::clicked, this, &LoginWindow::togglePasswordVisibility);

    // 回车键登录
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::attemptLogin);
}

void LoginWindow::attemptLogin()
{
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();

    // 简单验证
    if (username.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText("用户名和密码不能为空");
        m_errorLabel->show();
        return;
    }

    // 使用 AuthService 进行数据库验证
    if (AuthService::getInstance().authenticate(username, password)) {
        User user = AuthService::getInstance().getCurrentUser();
        QString role = QString("角色%1").arg(user.getRoleId());
        saveCredentialsIfNeeded();
        emit loginSuccessful(username, role);
        accept();
    } else {
        m_errorLabel->setText("用户名或密码错误");
        m_errorLabel->show();
    }
}

void LoginWindow::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_togglePasswordButton->setText("隐藏");
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_togglePasswordButton->setText("显示");
    }
}

void LoginWindow::loadSavedCredentials()
{
    QString savedUsername = m_settings->value("username", "").toString();
    bool rememberMe = m_settings->value("rememberMe", false).toBool();

    if (!savedUsername.isEmpty()) {
        m_usernameEdit->setText(savedUsername);
        m_rememberMeCheckBox->setChecked(rememberMe);
    }
}

void LoginWindow::saveCredentialsIfNeeded()
{
    if (m_rememberMeCheckBox->isChecked()) {
        m_settings->setValue("username", m_usernameEdit->text());
        m_settings->setValue("rememberMe", true);
    } else {
        m_settings->remove("username");
        m_settings->remove("rememberMe");
    }
}