#include "PrintApplicationPage.h"
#include "src/services/DeviceManagementService.h"
#include "src/services/SecurityLevelService.h"
#include "src/services/ApproverConfigService.h"
#include "src/services/TaskService.h"
#include "src/services/AuthService.h"
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
#include <QFileDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>
#include <QPixmap>
#include <QMimeDatabase>
#include <QMimeType>

PrintApplicationPage::PrintApplicationPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void PrintApplicationPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    QLabel *pageTitle = new QLabel("打印申请", this);
    pageTitle->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_layout->addWidget(pageTitle);

    // 申请表单
    setupApplicationForm();

    // 文档预览
    setupDocumentPreview();

    // 申请历史
    setupApplicationHistory();

    m_layout->addStretch();
}

void PrintApplicationPage::setupApplicationForm()
{
    m_applicationFormGroup = new QGroupBox("打印申请表单", this);
    m_formLayout = new QFormLayout(m_applicationFormGroup);

    // 文档标题
    m_documentTitleEdit = new QLineEdit(this);
    m_documentTitleEdit->setPlaceholderText("请输入文档标题");
    m_formLayout->addRow("文档标题:", m_documentTitleEdit);

    // 打印目的
    m_purposeEdit = new QTextEdit(this);
    m_purposeEdit->setFixedHeight(60);
    m_purposeEdit->setPlaceholderText("请简要说明打印目的");
    m_formLayout->addRow("打印目的:", m_purposeEdit);

    // 设备选择（打印机）
    m_deviceCombo = new QComboBox(this);
    loadDevices();
    m_formLayout->addRow("打印机:", m_deviceCombo);

    // 密级选择
    m_securityLevelCombo = new QComboBox(this);
    loadSecurityLevels();
    m_formLayout->addRow("密级:", m_securityLevelCombo);

    // 审批员选择
    m_approverCombo = new QComboBox(this);
    loadApprovers();
    m_formLayout->addRow("审批员:", m_approverCombo);

    // 优先级
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItem("普通", 0);
    m_priorityCombo->addItem("紧急", 1);
    m_priorityCombo->addItem("加急", 2);
    m_formLayout->addRow("优先级:", m_priorityCombo);

    // 打印份数
    m_copiesSpin = new QSpinBox(this);
    m_copiesSpin->setRange(1, 999);
    m_copiesSpin->setValue(1);
    m_formLayout->addRow("打印份数:", m_copiesSpin);

    // 截止日期
    m_deadlineEdit = new QDateEdit(this);
    m_deadlineEdit->setDate(QDate::currentDate().addDays(7));
    m_deadlineEdit->setDisplayFormat("yyyy-MM-dd");
    m_deadlineEdit->setCalendarPopup(true);
    m_formLayout->addRow("截止日期:", m_deadlineEdit);

    // 文件路径
    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setReadOnly(true);
    m_filePathEdit->setPlaceholderText("请选择要打印的文档");
    m_browseButton = new QPushButton("浏览", this);
    connect(m_browseButton, &QPushButton::clicked, this, &PrintApplicationPage::browseFile);

    fileLayout->addWidget(m_filePathEdit);
    fileLayout->addWidget(m_browseButton);
    m_formLayout->addRow("选择文件:", fileLayout);

    // 提交按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_submitButton = new QPushButton("提交申请", this);
    m_clearButton = new QPushButton("清空", this);

    connect(m_submitButton, &QPushButton::clicked, this, &PrintApplicationPage::submitApplication);
    connect(m_clearButton, &QPushButton::clicked, [this]() {
        m_documentTitleEdit->clear();
        m_purposeEdit->clear();
        m_deviceCombo->setCurrentIndex(-1);
        m_securityLevelCombo->setCurrentIndex(-1);
        m_approverCombo->setCurrentIndex(-1);
        m_priorityCombo->setCurrentIndex(0);
        m_copiesSpin->setValue(1);
        m_deadlineEdit->setDate(QDate::currentDate().addDays(7));
        m_filePathEdit->clear();
    });

    buttonLayout->addWidget(m_submitButton);
    buttonLayout->addWidget(m_clearButton);
    m_formLayout->addRow(buttonLayout);

    m_layout->addWidget(m_applicationFormGroup);
}

void PrintApplicationPage::setupDocumentPreview()
{
    m_documentPreviewGroup = new QGroupBox("文档预览", this);
    QVBoxLayout *previewLayout = new QVBoxLayout(m_documentPreviewGroup);

    m_previewLabel = new QLabel("文档预览将在选择文件后显示", this);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("border: 1px solid gray; padding: 20px; background-color: #f5f5f5;");
    previewLayout->addWidget(m_previewLabel);

    m_layout->addWidget(m_documentPreviewGroup);
}

void PrintApplicationPage::setupApplicationHistory()
{
    m_historyGroup = new QGroupBox("申请历史", this);
    QVBoxLayout *historyLayout = new QVBoxLayout(m_historyGroup);

    m_historyTable = new QTableWidget(0, 5, this);
    m_historyTable->setHorizontalHeaderLabels({"申请时间", "文档标题", "状态", "份数", "操作人"});
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 示例历史记录
    for (int i = 0; i < 3; ++i) {
        m_historyTable->insertRow(i);
        m_historyTable->setItem(i, 0, new QTableWidgetItem(QDateTime::currentDateTime().addDays(-i).toString("MM-dd hh:mm")));
        m_historyTable->setItem(i, 1, new QTableWidgetItem("文档标题 " + QString::number(i+1)));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(i == 0 ? "已批准" : (i == 1 ? "待审批" : "已拒绝")));
        m_historyTable->setItem(i, 3, new QTableWidgetItem(QString::number(i+1)));
        m_historyTable->setItem(i, 4, new QTableWidgetItem("申请人 " + QString::number(i+1)));
    }

    historyLayout->addWidget(m_historyTable);
    m_layout->addWidget(m_historyGroup);
}

void PrintApplicationPage::browseFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择要打印的文档",
        "",
        "文档文件 (*.pdf *.doc *.docx *.txt *.xls *.xlsx);;PDF文件 (*.pdf);;文本文件 (*.txt);;所有文件 (*)"
    );

    if (!fileName.isEmpty()) {
        m_filePathEdit->setText(fileName);
        updatePreview(fileName);
    }
}

void PrintApplicationPage::updatePreview(const QString& filePath)
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        m_previewLabel->setText("文件不存在");
        return;
    }

    QString suffix = fileInfo.suffix().toLower();

    // 图片文件：直接显示缩略图
    if (suffix == "png" || suffix == "jpg" || suffix == "jpeg" ||
        suffix == "bmp" || suffix == "gif" || suffix == "tiff") {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            QPixmap scaled = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_previewLabel->setPixmap(scaled);
            return;
        }
        m_previewLabel->setText("无法加载图片");
        return;
    }

    // 其他文件类型：显示文件图标和基本信息
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);

    QString sizeStr;
    qint64 size = fileInfo.size();
    if (size < 1024) {
        sizeStr = QString("%1 B").arg(size);
    } else if (size < 1024 * 1024) {
        sizeStr = QString("%1 KB").arg(size / 1024.0, 0, 'f', 2);
    } else {
        sizeStr = QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 2);
    }

    QString previewText = QString(
        "文件名: %1\n"
        "类型: %2\n"
        "大小: %3\n"
        "修改时间: %4"
    ).arg(fileInfo.fileName())
     .arg(mimeType.comment())
     .arg(sizeStr)
     .arg(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));

    m_previewLabel->setText(previewText);
}

void PrintApplicationPage::submitApplication()
{
    if (m_documentTitleEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入文档标题！");
        return;
    }

    if (m_filePathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要打印的文档！");
        return;
    }

    if (m_deviceCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "警告", "请选择打印机！");
        return;
    }

    if (m_securityLevelCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "警告", "请选择密级！");
        return;
    }

    if (m_approverCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "警告", "请选择审批员！");
        return;
    }

    // 获取当前用户
    User currentUser = AuthService::getInstance().getCurrentUser();

    // 获取优先级文本
    QString priority = m_priorityCombo->currentText();

    // 创建打印任务
    bool success = TaskService::getInstance().createPrintTask(
        currentUser,
        m_documentTitleEdit->text(),
        m_purposeEdit->toPlainText(),
        m_copiesSpin->value(),
        m_filePathEdit->text(),
        priority
    );

    if (success) {
        QMessageBox::information(this, "成功", "打印申请已提交，请等待审批！");

        // 清空表单
        m_documentTitleEdit->clear();
        m_purposeEdit->clear();
        m_deviceCombo->setCurrentIndex(-1);
        m_securityLevelCombo->setCurrentIndex(-1);
        m_approverCombo->setCurrentIndex(-1);
        m_priorityCombo->setCurrentIndex(0);
        m_copiesSpin->setValue(1);
        m_deadlineEdit->setDate(QDate::currentDate().addDays(7));
        m_filePathEdit->clear();
        m_previewLabel->clear();
        m_previewLabel->setText("文档预览将在选择文件后显示");
    } else {
        QMessageBox::warning(this, "错误", "打印申请提交失败，请重试！");
    }
}

void PrintApplicationPage::loadDevices()
{
    m_deviceCombo->clear();

    // 获取打印机列表（设备类型为 "printer"）
    QList<Device> printers = DeviceManagementService::getInstance().getDevicesByType("printer");

    for (const Device &device : printers) {
        m_deviceCombo->addItem(device.getName(), device.getId());
    }
}

void PrintApplicationPage::loadSecurityLevels()
{
    m_securityLevelCombo->clear();

    // 获取激活的密级列表
    QList<SecurityLevel> levels = SecurityLevelService::getInstance().getActiveLevels();

    for (const SecurityLevel &level : levels) {
        m_securityLevelCombo->addItem(level.getLevelName(), level.getId());
    }
}

void PrintApplicationPage::loadApprovers()
{
    m_approverCombo->clear();

    // 获取打印任务类型（task_type = 0）的审批员配置
    QList<ApproverConfig> configs = ApproverConfigService::getInstance().getConfigsByTaskType(0);

    // 获取审批角色列表用于显示名称
    QList<QPair<int, QString>> approverRoles = ApproverConfigService::getInstance().getApproverRoles();

    for (const ApproverConfig &config : configs) {
        if (!config.isActive()) continue;

        // 查找审批角色名称
        QString roleName;
        for (const auto &role : approverRoles) {
            if (role.first == config.getApproverRoleId()) {
                roleName = role.second;
                break;
            }
        }

        QString displayText = roleName.isEmpty() ?
            QString("审批角色 #%1").arg(config.getApproverRoleId()) : roleName;
        m_approverCombo->addItem(displayText, config.getId());
    }
}