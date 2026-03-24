#include "SerialNumberConfigPage.h"
#include <QDebug>

SerialNumberConfigPage::SerialNumberConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_service(SerialNumberService::getInstance())
    , m_printConfigId(0)
    , m_burnConfigId(0)
{
    setupUI();
    loadConfigs();
}

void SerialNumberConfigPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel *titleLabel = new QLabel("流水号设置", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 说明文字
    QLabel *descLabel = new QLabel("配置打印和刻录任务的流水号生成规则。格式：前缀 + 日期 + 序列号", this);
    descLabel->setStyleSheet("color: gray; margin-bottom: 10px;");
    mainLayout->addWidget(descLabel);

    // 打印配置区域
    setupPrintConfigGroup();
    mainLayout->addWidget(m_printConfigGroup);

    // 刻录配置区域
    setupBurnConfigGroup();
    mainLayout->addWidget(m_burnConfigGroup);

    // 预览区域
    setupPreviewArea();
    mainLayout->addWidget(new QLabel("预览", this));
    mainLayout->addWidget(m_printPreviewLabel);
    mainLayout->addWidget(m_burnPreviewLabel);

    mainLayout->addStretch();

    // 刷新按钮
    QPushButton *refreshButton = new QPushButton("刷新", this);
    connect(refreshButton, &QPushButton::clicked, this, &SerialNumberConfigPage::refreshDisplay);
    mainLayout->addWidget(refreshButton);
}

void SerialNumberConfigPage::setupPrintConfigGroup()
{
    m_printConfigGroup = new QGroupBox("打印流水号配置", this);
    QFormLayout *formLayout = new QFormLayout(m_printConfigGroup);

    // 前缀
    m_printPrefixEdit = new QLineEdit(this);
    m_printPrefixEdit->setPlaceholderText("如：PRT-");
    formLayout->addRow("前缀:", m_printPrefixEdit);

    // 日期格式
    m_printDateFormatCombo = new QComboBox(this);
    m_printDateFormatCombo->addItem("yyyyMMdd (20240324)", "yyyyMMdd");
    m_printDateFormatCombo->addItem("yyyy-MM-dd (2024-03-24)", "yyyy-MM-dd");
    m_printDateFormatCombo->addItem("yyyy/MM/dd (2024/03/24)", "yyyy/MM/dd");
    m_printDateFormatCombo->addItem("yyMMdd (240324)", "yyMMdd");
    formLayout->addRow("日期格式:", m_printDateFormatCombo);

    // 序列号长度
    m_printSequenceLengthSpin = new QSpinBox(this);
    m_printSequenceLengthSpin->setRange(1, 10);
    m_printSequenceLengthSpin->setValue(4);
    formLayout->addRow("序列号长度:", m_printSequenceLengthSpin);

    // 当前序列号
    m_printCurrentSequenceSpin = new QSpinBox(this);
    m_printCurrentSequenceSpin->setRange(0, 999999999);
    m_printCurrentSequenceSpin->setValue(0);
    formLayout->addRow("当前序列号:", m_printCurrentSequenceSpin);

    // 重置周期
    m_printResetPeriodCombo = new QComboBox(this);
    m_printResetPeriodCombo->addItem("每日", ResetPeriod::DAILY);
    m_printResetPeriodCombo->addItem("每月", ResetPeriod::MONTHLY);
    m_printResetPeriodCombo->addItem("每年", ResetPeriod::YEARLY);
    formLayout->addRow("重置周期:", m_printResetPeriodCombo);

    // 上次重置日期
    m_printLastResetDateEdit = new QDateEdit(this);
    m_printLastResetDateEdit->setCalendarPopup(true);
    m_printLastResetDateEdit->setDate(QDate::currentDate());
    formLayout->addRow("上次重置日期:", m_printLastResetDateEdit);

    // 是否启用
    m_printIsActiveCheck = new QCheckBox("启用流水号", this);
    m_printIsActiveCheck->setChecked(true);
    formLayout->addRow("状态:", m_printIsActiveCheck);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_printSaveButton = new QPushButton("保存配置", this);
    m_printResetButton = new QPushButton("重置序列号", this);
    m_printPreviewButton = new QPushButton("预览", this);

    connect(m_printSaveButton, &QPushButton::clicked, this, &SerialNumberConfigPage::savePrintConfig);
    connect(m_printResetButton, &QPushButton::clicked, this, &SerialNumberConfigPage::resetPrintSequence);
    connect(m_printPreviewButton, &QPushButton::clicked, this, &SerialNumberConfigPage::previewPrintSerialNumber);

    buttonLayout->addWidget(m_printSaveButton);
    buttonLayout->addWidget(m_printResetButton);
    buttonLayout->addWidget(m_printPreviewButton);
    buttonLayout->addStretch();

    formLayout->addRow(buttonLayout);
}

void SerialNumberConfigPage::setupBurnConfigGroup()
{
    m_burnConfigGroup = new QGroupBox("刻录流水号配置", this);
    QFormLayout *formLayout = new QFormLayout(m_burnConfigGroup);

    // 前缀
    m_burnPrefixEdit = new QLineEdit(this);
    m_burnPrefixEdit->setPlaceholderText("如：BRN-");
    formLayout->addRow("前缀:", m_burnPrefixEdit);

    // 日期格式
    m_burnDateFormatCombo = new QComboBox(this);
    m_burnDateFormatCombo->addItem("yyyyMMdd (20240324)", "yyyyMMdd");
    m_burnDateFormatCombo->addItem("yyyy-MM-dd (2024-03-24)", "yyyy-MM-dd");
    m_burnDateFormatCombo->addItem("yyyy/MM/dd (2024/03/24)", "yyyy/MM/dd");
    m_burnDateFormatCombo->addItem("yyMMdd (240324)", "yyMMdd");
    formLayout->addRow("日期格式:", m_burnDateFormatCombo);

    // 序列号长度
    m_burnSequenceLengthSpin = new QSpinBox(this);
    m_burnSequenceLengthSpin->setRange(1, 10);
    m_burnSequenceLengthSpin->setValue(4);
    formLayout->addRow("序列号长度:", m_burnSequenceLengthSpin);

    // 当前序列号
    m_burnCurrentSequenceSpin = new QSpinBox(this);
    m_burnCurrentSequenceSpin->setRange(0, 999999999);
    m_burnCurrentSequenceSpin->setValue(0);
    formLayout->addRow("当前序列号:", m_burnCurrentSequenceSpin);

    // 重置周期
    m_burnResetPeriodCombo = new QComboBox(this);
    m_burnResetPeriodCombo->addItem("每日", ResetPeriod::DAILY);
    m_burnResetPeriodCombo->addItem("每月", ResetPeriod::MONTHLY);
    m_burnResetPeriodCombo->addItem("每年", ResetPeriod::YEARLY);
    formLayout->addRow("重置周期:", m_burnResetPeriodCombo);

    // 上次重置日期
    m_burnLastResetDateEdit = new QDateEdit(this);
    m_burnLastResetDateEdit->setCalendarPopup(true);
    m_burnLastResetDateEdit->setDate(QDate::currentDate());
    formLayout->addRow("上次重置日期:", m_burnLastResetDateEdit);

    // 是否启用
    m_burnIsActiveCheck = new QCheckBox("启用流水号", this);
    m_burnIsActiveCheck->setChecked(true);
    formLayout->addRow("状态:", m_burnIsActiveCheck);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_burnSaveButton = new QPushButton("保存配置", this);
    m_burnResetButton = new QPushButton("重置序列号", this);
    m_burnPreviewButton = new QPushButton("预览", this);

    connect(m_burnSaveButton, &QPushButton::clicked, this, &SerialNumberConfigPage::saveBurnConfig);
    connect(m_burnResetButton, &QPushButton::clicked, this, &SerialNumberConfigPage::resetBurnSequence);
    connect(m_burnPreviewButton, &QPushButton::clicked, this, &SerialNumberConfigPage::previewBurnSerialNumber);

    buttonLayout->addWidget(m_burnSaveButton);
    buttonLayout->addWidget(m_burnResetButton);
    buttonLayout->addWidget(m_burnPreviewButton);
    buttonLayout->addStretch();

    formLayout->addRow(buttonLayout);
}

void SerialNumberConfigPage::setupPreviewArea()
{
    m_printPreviewLabel = new QLabel(this);
    m_printPreviewLabel->setStyleSheet("font-size: 14px; color: #2196F3; padding: 5px;");
    m_printPreviewLabel->setText("打印流水号预览: -");

    m_burnPreviewLabel = new QLabel(this);
    m_burnPreviewLabel->setStyleSheet("font-size: 14px; color: #2196F3; padding: 5px;");
    m_burnPreviewLabel->setText("刻录流水号预览: -");
}

void SerialNumberConfigPage::loadConfigs()
{
    // 加载打印配置
    SerialNumberConfig printConfig = m_service.getConfigByType(SerialNumberConfigType::PRINT);
    if (printConfig.getId() > 0) {
        m_printConfigId = printConfig.getId();
        m_printPrefixEdit->setText(printConfig.getPrefix());

        // 设置日期格式
        int dateFormatIndex = m_printDateFormatCombo->findData(printConfig.getDateFormat());
        if (dateFormatIndex >= 0) {
            m_printDateFormatCombo->setCurrentIndex(dateFormatIndex);
        }

        m_printSequenceLengthSpin->setValue(printConfig.getSequenceLength());
        m_printCurrentSequenceSpin->setValue(printConfig.getCurrentSequence());
        m_printResetPeriodCombo->setCurrentIndex(printConfig.getResetPeriod());
        m_printLastResetDateEdit->setDate(printConfig.getLastResetDate());
        m_printIsActiveCheck->setChecked(printConfig.getIsActive());
    }

    // 加载刻录配置
    SerialNumberConfig burnConfig = m_service.getConfigByType(SerialNumberConfigType::BURN);
    if (burnConfig.getId() > 0) {
        m_burnConfigId = burnConfig.getId();
        m_burnPrefixEdit->setText(burnConfig.getPrefix());

        // 设置日期格式
        int dateFormatIndex = m_burnDateFormatCombo->findData(burnConfig.getDateFormat());
        if (dateFormatIndex >= 0) {
            m_burnDateFormatCombo->setCurrentIndex(dateFormatIndex);
        }

        m_burnSequenceLengthSpin->setValue(burnConfig.getSequenceLength());
        m_burnCurrentSequenceSpin->setValue(burnConfig.getCurrentSequence());
        m_burnResetPeriodCombo->setCurrentIndex(burnConfig.getResetPeriod());
        m_burnLastResetDateEdit->setDate(burnConfig.getLastResetDate());
        m_burnIsActiveCheck->setChecked(burnConfig.getIsActive());
    }

    refreshDisplay();
}

void SerialNumberConfigPage::savePrintConfig()
{
    SerialNumberConfig config;
    if (m_printConfigId > 0) {
        config.setId(m_printConfigId);
    }
    config.setConfigType(SerialNumberConfigType::PRINT);
    config.setPrefix(m_printPrefixEdit->text());
    config.setDateFormat(m_printDateFormatCombo->currentData().toString());
    config.setSequenceLength(m_printSequenceLengthSpin->value());
    config.setCurrentSequence(m_printCurrentSequenceSpin->value());
    config.setResetPeriod(m_printResetPeriodCombo->currentData().toInt());
    config.setLastResetDate(m_printLastResetDateEdit->date());
    config.setIsActive(m_printIsActiveCheck->isChecked());

    bool success = false;
    if (m_printConfigId > 0) {
        success = m_service.updateConfig(config, 1); // TODO: use real operator ID
    } else {
        success = m_service.createConfig(config, 1); // TODO: use real operator ID
        if (success) {
            m_printConfigId = config.getId();
        }
    }

    if (success) {
        QMessageBox::information(this, "成功", "打印流水号配置已保存");
        refreshDisplay();
    } else {
        QMessageBox::warning(this, "失败", "保存打印流水号配置失败");
    }
}

void SerialNumberConfigPage::saveBurnConfig()
{
    SerialNumberConfig config;
    if (m_burnConfigId > 0) {
        config.setId(m_burnConfigId);
    }
    config.setConfigType(SerialNumberConfigType::BURN);
    config.setPrefix(m_burnPrefixEdit->text());
    config.setDateFormat(m_burnDateFormatCombo->currentData().toString());
    config.setSequenceLength(m_burnSequenceLengthSpin->value());
    config.setCurrentSequence(m_burnCurrentSequenceSpin->value());
    config.setResetPeriod(m_burnResetPeriodCombo->currentData().toInt());
    config.setLastResetDate(m_burnLastResetDateEdit->date());
    config.setIsActive(m_burnIsActiveCheck->isChecked());

    bool success = false;
    if (m_burnConfigId > 0) {
        success = m_service.updateConfig(config, 1); // TODO: use real operator ID
    } else {
        success = m_service.createConfig(config, 1); // TODO: use real operator ID
        if (success) {
            m_burnConfigId = config.getId();
        }
    }

    if (success) {
        QMessageBox::information(this, "成功", "刻录流水号配置已保存");
        refreshDisplay();
    } else {
        QMessageBox::warning(this, "失败", "保存刻录流水号配置失败");
    }
}

void SerialNumberConfigPage::resetPrintSequence()
{
    if (m_printConfigId == 0) {
        QMessageBox::warning(this, "警告", "请先保存配置");
        return;
    }

    int result = QMessageBox::question(this, "确认", "确定要重置打印序列号吗？",
                                       QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes) {
        if (m_service.resetSequence(m_printConfigId, 1)) { // TODO: use real operator ID
            m_printCurrentSequenceSpin->setValue(0);
            m_printLastResetDateEdit->setDate(QDate::currentDate());
            QMessageBox::information(this, "成功", "打印序列号已重置");
            refreshDisplay();
        } else {
            QMessageBox::warning(this, "失败", "重置打印序列号失败");
        }
    }
}

void SerialNumberConfigPage::resetBurnSequence()
{
    if (m_burnConfigId == 0) {
        QMessageBox::warning(this, "警告", "请先保存配置");
        return;
    }

    int result = QMessageBox::question(this, "确认", "确定要重置刻录序列号吗？",
                                       QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes) {
        if (m_service.resetSequence(m_burnConfigId, 1)) { // TODO: use real operator ID
            m_burnCurrentSequenceSpin->setValue(0);
            m_burnLastResetDateEdit->setDate(QDate::currentDate());
            QMessageBox::information(this, "成功", "刻录序列号已重置");
            refreshDisplay();
        } else {
            QMessageBox::warning(this, "失败", "重置刻录序列号失败");
        }
    }
}

void SerialNumberConfigPage::previewPrintSerialNumber()
{
    SerialNumberConfig config;
    config.setPrefix(m_printPrefixEdit->text());
    config.setDateFormat(m_printDateFormatCombo->currentData().toString());
    config.setSequenceLength(m_printSequenceLengthSpin->value());
    config.setCurrentSequence(m_printCurrentSequenceSpin->value());

    QString preview = m_service.previewSerialNumber(config);
    m_printPreviewLabel->setText("打印流水号预览: " + preview);
}

void SerialNumberConfigPage::previewBurnSerialNumber()
{
    SerialNumberConfig config;
    config.setPrefix(m_burnPrefixEdit->text());
    config.setDateFormat(m_burnDateFormatCombo->currentData().toString());
    config.setSequenceLength(m_burnSequenceLengthSpin->value());
    config.setCurrentSequence(m_burnCurrentSequenceSpin->value());

    QString preview = m_service.previewSerialNumber(config);
    m_burnPreviewLabel->setText("刻录流水号预览: " + preview);
}

void SerialNumberConfigPage::refreshDisplay()
{
    // 更新预览
    previewPrintSerialNumber();
    previewBurnSerialNumber();
}
