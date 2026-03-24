#include "BarcodeConfigPage.h"

BarcodeConfigPage::BarcodeConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_service(BarcodeService::getInstance())
    , m_printConfigId(0)
    , m_burnConfigId(0)
{
    setupUI();
    loadConfigs();
}

void BarcodeConfigPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *titleLabel = new QLabel("条码设置", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);

    // Description
    QLabel *descLabel = new QLabel("配置打印和刻录任务的条码/二维码生成规则", this);
    descLabel->setStyleSheet("color: #666; margin-bottom: 10px;");
    mainLayout->addWidget(descLabel);

    // Setup config groups
    setupPrintConfigGroup();
    setupBurnConfigGroup();

    mainLayout->addWidget(m_printConfigGroup);
    mainLayout->addWidget(m_burnConfigGroup);
    mainLayout->addStretch();
}

void BarcodeConfigPage::setupPrintConfigGroup()
{
    m_printConfigGroup = new QGroupBox("打印条码配置", this);
    QVBoxLayout *layout = new QVBoxLayout(m_printConfigGroup);

    // Barcode type selection
    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("条码类型:"));
    m_printBarcodeTypeCombo = new QComboBox();
    m_printBarcodeTypeCombo->addItem("条形码 (Code128)", BarcodeType::BARCODE);
    m_printBarcodeTypeCombo->addItem("二维码", BarcodeType::QRCODE);
    connect(m_printBarcodeTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BarcodeConfigPage::onPrintBarcodeTypeChanged);
    typeLayout->addWidget(m_printBarcodeTypeCombo);
    typeLayout->addStretch();
    layout->addLayout(typeLayout);

    // Barcode size settings
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    m_printBarcodeSizeLabel = new QLabel("条码尺寸:");
    sizeLayout->addWidget(m_printBarcodeSizeLabel);
    m_printBarcodeWidthSpin = new QSpinBox();
    m_printBarcodeWidthSpin->setRange(100, 500);
    m_printBarcodeWidthSpin->setValue(200);
    m_printBarcodeWidthSpin->setSuffix(" px");
    sizeLayout->addWidget(m_printBarcodeWidthSpin);
    sizeLayout->addWidget(new QLabel("x"));
    m_printBarcodeHeightSpin = new QSpinBox();
    m_printBarcodeHeightSpin->setRange(30, 150);
    m_printBarcodeHeightSpin->setValue(50);
    m_printBarcodeHeightSpin->setSuffix(" px");
    sizeLayout->addWidget(m_printBarcodeHeightSpin);
    sizeLayout->addStretch();
    layout->addLayout(sizeLayout);

    // QR code size
    QHBoxLayout *qrLayout = new QHBoxLayout();
    m_printQrcodeSizeLabel = new QLabel("二维码尺寸:");
    qrLayout->addWidget(m_printQrcodeSizeLabel);
    m_printQrcodeSizeSpin = new QSpinBox();
    m_printQrcodeSizeSpin->setRange(50, 300);
    m_printQrcodeSizeSpin->setValue(100);
    m_printQrcodeSizeSpin->setSuffix(" px");
    qrLayout->addWidget(m_printQrcodeSizeSpin);
    qrLayout->addStretch();
    layout->addLayout(qrLayout);

    // Fields selection
    QLabel *fieldsLabel = new QLabel("条码内容字段 (流水号为必选，最多再选4个):");
    layout->addWidget(fieldsLabel);

    m_printFieldsList = new QListWidget();
    m_printFieldsList->setSelectionMode(QAbstractItemView::MultiSelection);
    m_printFieldsList->addItem("流水号 (必选)");
    m_printFieldsList->item(0)->setFlags(Qt::ItemIsEnabled); // 不可选择
    m_printFieldsList->addItem("自定义文本");
    m_printFieldsList->addItem("提交人部门");
    m_printFieldsList->addItem("提交人");
    m_printFieldsList->addItem("密级");
    m_printFieldsList->addItem("份数");
    m_printFieldsList->addItem("文件名");
    m_printFieldsList->setMaximumHeight(120);
    connect(m_printFieldsList, &QListWidget::itemSelectionChanged,
            this, &BarcodeConfigPage::updatePrintFieldSelection);
    layout->addWidget(m_printFieldsList);

    // Custom text
    QHBoxLayout *customLayout = new QHBoxLayout();
    customLayout->addWidget(new QLabel("自定义文本:"));
    m_printCustomTextEdit = new QLineEdit();
    m_printCustomTextEdit->setPlaceholderText("输入自定义文本内容");
    customLayout->addWidget(m_printCustomTextEdit);
    layout->addLayout(customLayout);

    // Active check
    m_printIsActiveCheck = new QCheckBox("启用打印条码");
    m_printIsActiveCheck->setChecked(true);
    layout->addWidget(m_printIsActiveCheck);

    // Save button
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_printSaveButton = new QPushButton("保存打印配置");
    m_printSaveButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(m_printSaveButton, &QPushButton::clicked, this, &BarcodeConfigPage::savePrintConfig);
    btnLayout->addWidget(m_printSaveButton);
    layout->addLayout(btnLayout);
}

void BarcodeConfigPage::setupBurnConfigGroup()
{
    m_burnConfigGroup = new QGroupBox("刻录条码配置", this);
    QVBoxLayout *layout = new QVBoxLayout(m_burnConfigGroup);

    // Barcode type selection
    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("条码类型:"));
    m_burnBarcodeTypeCombo = new QComboBox();
    m_burnBarcodeTypeCombo->addItem("条形码 (Code128)", BarcodeType::BARCODE);
    m_burnBarcodeTypeCombo->addItem("二维码", BarcodeType::QRCODE);
    connect(m_burnBarcodeTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BarcodeConfigPage::onBurnBarcodeTypeChanged);
    typeLayout->addWidget(m_burnBarcodeTypeCombo);
    typeLayout->addStretch();
    layout->addLayout(typeLayout);

    // Barcode size settings
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    m_burnBarcodeSizeLabel = new QLabel("条码尺寸:");
    sizeLayout->addWidget(m_burnBarcodeSizeLabel);
    m_burnBarcodeWidthSpin = new QSpinBox();
    m_burnBarcodeWidthSpin->setRange(100, 500);
    m_burnBarcodeWidthSpin->setValue(200);
    m_burnBarcodeWidthSpin->setSuffix(" px");
    sizeLayout->addWidget(m_burnBarcodeWidthSpin);
    sizeLayout->addWidget(new QLabel("x"));
    m_burnBarcodeHeightSpin = new QSpinBox();
    m_burnBarcodeHeightSpin->setRange(30, 150);
    m_burnBarcodeHeightSpin->setValue(50);
    m_burnBarcodeHeightSpin->setSuffix(" px");
    sizeLayout->addWidget(m_burnBarcodeHeightSpin);
    sizeLayout->addStretch();
    layout->addLayout(sizeLayout);

    // QR code size
    QHBoxLayout *qrLayout = new QHBoxLayout();
    m_burnQrcodeSizeLabel = new QLabel("二维码尺寸:");
    qrLayout->addWidget(m_burnQrcodeSizeLabel);
    m_burnQrcodeSizeSpin = new QSpinBox();
    m_burnQrcodeSizeSpin->setRange(50, 300);
    m_burnQrcodeSizeSpin->setValue(100);
    m_burnQrcodeSizeSpin->setSuffix(" px");
    qrLayout->addWidget(m_burnQrcodeSizeSpin);
    qrLayout->addStretch();
    layout->addLayout(qrLayout);

    // Fields selection
    QLabel *fieldsLabel = new QLabel("条码内容字段 (流水号为必选，最多再选4个):");
    layout->addWidget(fieldsLabel);

    m_burnFieldsList = new QListWidget();
    m_burnFieldsList->setSelectionMode(QAbstractItemView::MultiSelection);
    m_burnFieldsList->addItem("流水号 (必选)");
    m_burnFieldsList->item(0)->setFlags(Qt::ItemIsEnabled); // 不可选择
    m_burnFieldsList->addItem("自定义文本");
    m_burnFieldsList->addItem("提交人部门");
    m_burnFieldsList->addItem("提交人");
    m_burnFieldsList->addItem("密级");
    m_burnFieldsList->addItem("份数");
    m_burnFieldsList->addItem("文件名");
    m_burnFieldsList->setMaximumHeight(120);
    connect(m_burnFieldsList, &QListWidget::itemSelectionChanged,
            this, &BarcodeConfigPage::updateBurnFieldSelection);
    layout->addWidget(m_burnFieldsList);

    // Custom text
    QHBoxLayout *customLayout = new QHBoxLayout();
    customLayout->addWidget(new QLabel("自定义文本:"));
    m_burnCustomTextEdit = new QLineEdit();
    m_burnCustomTextEdit->setPlaceholderText("输入自定义文本内容");
    customLayout->addWidget(m_burnCustomTextEdit);
    layout->addLayout(customLayout);

    // Active check
    m_burnIsActiveCheck = new QCheckBox("启用刻录条码");
    m_burnIsActiveCheck->setChecked(true);
    layout->addWidget(m_burnIsActiveCheck);

    // Save button
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_burnSaveButton = new QPushButton("保存刻录配置");
    m_burnSaveButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(m_burnSaveButton, &QPushButton::clicked, this, &BarcodeConfigPage::saveBurnConfig);
    btnLayout->addWidget(m_burnSaveButton);
    layout->addLayout(btnLayout);
}

void BarcodeConfigPage::loadConfigs()
{
    // Load print config (config_type = 0)
    BarcodeConfig printConfig = m_service.getConfigByType(0);
    if (printConfig.getId() > 0) {
        m_printConfigId = printConfig.getId();
        m_printBarcodeTypeCombo->setCurrentIndex(printConfig.getBarcodeType());
        m_printBarcodeWidthSpin->setValue(printConfig.getBarcodeWidth());
        m_printBarcodeHeightSpin->setValue(printConfig.getBarcodeHeight());
        m_printQrcodeSizeSpin->setValue(printConfig.getQrcodeSize());
        m_printCustomTextEdit->setText(printConfig.getCustomText());
        m_printIsActiveCheck->setChecked(printConfig.getIsActive());

        // Set selected fields
        QList<int> fields = printConfig.getSelectedFields();
        for (int i = 0; i < m_printFieldsList->count(); ++i) {
            if (fields.contains(i)) {
                m_printFieldsList->item(i)->setSelected(true);
            }
        }
    }

    // Load burn config (config_type = 1)
    BarcodeConfig burnConfig = m_service.getConfigByType(1);
    if (burnConfig.getId() > 0) {
        m_burnConfigId = burnConfig.getId();
        m_burnBarcodeTypeCombo->setCurrentIndex(burnConfig.getBarcodeType());
        m_burnBarcodeWidthSpin->setValue(burnConfig.getBarcodeWidth());
        m_burnBarcodeHeightSpin->setValue(burnConfig.getBarcodeHeight());
        m_burnQrcodeSizeSpin->setValue(burnConfig.getQrcodeSize());
        m_burnCustomTextEdit->setText(burnConfig.getCustomText());
        m_burnIsActiveCheck->setChecked(burnConfig.getIsActive());

        // Set selected fields
        QList<int> fields = burnConfig.getSelectedFields();
        for (int i = 0; i < m_burnFieldsList->count(); ++i) {
            if (fields.contains(i)) {
                m_burnFieldsList->item(i)->setSelected(true);
            }
        }
    }

    refreshDisplay();
}

void BarcodeConfigPage::savePrintConfig()
{
    BarcodeConfig config;
    if (m_printConfigId > 0) {
        config.setId(m_printConfigId);
    }
    config.setConfigType(0); // Print
    config.setBarcodeType(m_printBarcodeTypeCombo->currentIndex());
    config.setBarcodeWidth(m_printBarcodeWidthSpin->value());
    config.setBarcodeHeight(m_printBarcodeHeightSpin->value());
    config.setQrcodeSize(m_printQrcodeSizeSpin->value());
    config.setCustomText(m_printCustomTextEdit->text());
    config.setIsActive(m_printIsActiveCheck->isChecked());

    // Get selected fields
    QList<int> selectedFields;
    selectedFields.append(BarcodeField::SERIAL_NUMBER); // Always include serial number
    for (int i = 1; i < m_printFieldsList->count(); ++i) {
        if (m_printFieldsList->item(i)->isSelected()) {
            selectedFields.append(i);
        }
    }
    config.setSelectedFields(selectedFields);

    bool success;
    if (m_printConfigId > 0) {
        success = m_service.updateConfig(config, 1); // operatorId = 1 (system)
    } else {
        success = m_service.createConfig(config, 1);
        if (success) {
            m_printConfigId = config.getId();
        }
    }

    if (success) {
        QMessageBox::information(this, "成功", "打印条码配置已保存");
    } else {
        QMessageBox::warning(this, "错误", "保存打印条码配置失败");
    }
}

void BarcodeConfigPage::saveBurnConfig()
{
    BarcodeConfig config;
    if (m_burnConfigId > 0) {
        config.setId(m_burnConfigId);
    }
    config.setConfigType(1); // Burn
    config.setBarcodeType(m_burnBarcodeTypeCombo->currentIndex());
    config.setBarcodeWidth(m_burnBarcodeWidthSpin->value());
    config.setBarcodeHeight(m_burnBarcodeHeightSpin->value());
    config.setQrcodeSize(m_burnQrcodeSizeSpin->value());
    config.setCustomText(m_burnCustomTextEdit->text());
    config.setIsActive(m_burnIsActiveCheck->isChecked());

    // Get selected fields
    QList<int> selectedFields;
    selectedFields.append(BarcodeField::SERIAL_NUMBER); // Always include serial number
    for (int i = 1; i < m_burnFieldsList->count(); ++i) {
        if (m_burnFieldsList->item(i)->isSelected()) {
            selectedFields.append(i);
        }
    }
    config.setSelectedFields(selectedFields);

    bool success;
    if (m_burnConfigId > 0) {
        success = m_service.updateConfig(config, 1); // operatorId = 1 (system)
    } else {
        success = m_service.createConfig(config, 1);
        if (success) {
            m_burnConfigId = config.getId();
        }
    }

    if (success) {
        QMessageBox::information(this, "成功", "刻录条码配置已保存");
    } else {
        QMessageBox::warning(this, "错误", "保存刻录条码配置失败");
    }
}

void BarcodeConfigPage::refreshDisplay()
{
    onPrintBarcodeTypeChanged(m_printBarcodeTypeCombo->currentIndex());
    onBurnBarcodeTypeChanged(m_burnBarcodeTypeCombo->currentIndex());
}

void BarcodeConfigPage::updatePrintFieldSelection()
{
    // Count non-serial number selected fields
    int count = 0;
    for (int i = 1; i < m_printFieldsList->count(); ++i) {
        if (m_printFieldsList->item(i)->isSelected()) {
            count++;
        }
    }

    // If exceeded limit (4), deselect the last one
    if (count > 4) {
        QMessageBox::warning(this, "提示", "除流水号外最多只能选择4个字段");
        // Deselect the last selected item (excluding serial number)
        for (int i = m_printFieldsList->count() - 1; i > 0; --i) {
            if (m_printFieldsList->item(i)->isSelected()) {
                m_printFieldsList->item(i)->setSelected(false);
                break;
            }
        }
    }
}

void BarcodeConfigPage::updateBurnFieldSelection()
{
    // Count non-serial number selected fields
    int count = 0;
    for (int i = 1; i < m_burnFieldsList->count(); ++i) {
        if (m_burnFieldsList->item(i)->isSelected()) {
            count++;
        }
    }

    // If exceeded limit (4), deselect the last one
    if (count > 4) {
        QMessageBox::warning(this, "提示", "除流水号外最多只能选择4个字段");
        // Deselect the last selected item (excluding serial number)
        for (int i = m_burnFieldsList->count() - 1; i > 0; --i) {
            if (m_burnFieldsList->item(i)->isSelected()) {
                m_burnFieldsList->item(i)->setSelected(false);
                break;
            }
        }
    }
}

void BarcodeConfigPage::onPrintBarcodeTypeChanged(int index)
{
    bool isBarcode = (index == BarcodeType::BARCODE);
    m_printBarcodeSizeLabel->setVisible(isBarcode);
    m_printBarcodeWidthSpin->setVisible(isBarcode);
    m_printBarcodeHeightSpin->setVisible(isBarcode);
    m_printQrcodeSizeLabel->setVisible(!isBarcode);
    m_printQrcodeSizeSpin->setVisible(!isBarcode);
}

void BarcodeConfigPage::onBurnBarcodeTypeChanged(int index)
{
    bool isBarcode = (index == BarcodeType::BARCODE);
    m_burnBarcodeSizeLabel->setVisible(isBarcode);
    m_burnBarcodeWidthSpin->setVisible(isBarcode);
    m_burnBarcodeHeightSpin->setVisible(isBarcode);
    m_burnQrcodeSizeLabel->setVisible(!isBarcode);
    m_burnQrcodeSizeSpin->setVisible(!isBarcode);
}
