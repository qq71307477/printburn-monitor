#include "SecurityLevelConfigPage.h"
#include <QHeaderView>
#include <QDateTime>

// SecurityLevelEditDialog Implementation

SecurityLevelEditDialog::SecurityLevelEditDialog(QWidget* parent, const SecurityLevel& level)
    : QDialog(parent)
    , m_level(level)
    , m_levelCodeEdit(nullptr)
    , m_levelNameEdit(nullptr)
    , m_retentionDaysSpin(nullptr)
    , m_timeoutMinutesSpin(nullptr)
    , m_descriptionEdit(nullptr)
    , m_activeCheck(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle(level.getId() > 0 ? "编辑密级" : "添加密级");
    setModal(true);
    resize(400, 350);
    setupUI();
}

void SecurityLevelEditDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

    // Level Code
    m_levelCodeEdit = new QLineEdit(this);
    m_levelCodeEdit->setText(m_level.getLevelCode());
    m_levelCodeEdit->setPlaceholderText("如: 1, 2, 3");
    formLayout->addRow("密级编号:", m_levelCodeEdit);

    // Level Name
    m_levelNameEdit = new QLineEdit(this);
    m_levelNameEdit->setText(m_level.getLevelName());
    m_levelNameEdit->setPlaceholderText("如: 绝密, 机密, 秘密");
    formLayout->addRow("密级名称:", m_levelNameEdit);

    // Retention Days
    m_retentionDaysSpin = new QSpinBox(this);
    m_retentionDaysSpin->setRange(1, 9999);
    m_retentionDaysSpin->setValue(m_level.getRetentionDays() > 0 ? m_level.getRetentionDays() : 30);
    formLayout->addRow("保存天数:", m_retentionDaysSpin);

    // Timeout Minutes
    m_timeoutMinutesSpin = new QSpinBox(this);
    m_timeoutMinutesSpin->setRange(1, 9999);
    m_timeoutMinutesSpin->setValue(m_level.getTimeoutMinutes() > 0 ? m_level.getTimeoutMinutes() : 30);
    formLayout->addRow("超时分钟数:", m_timeoutMinutesSpin);

    // Description
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setText(m_level.getDescription());
    m_descriptionEdit->setMaximumHeight(80);
    formLayout->addRow("描述:", m_descriptionEdit);

    // Is Active
    m_activeCheck = new QCheckBox("启用", this);
    m_activeCheck->setChecked(m_level.isActive());
    formLayout->addRow("状态:", m_activeCheck);

    mainLayout->addLayout(formLayout);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_saveButton = new QPushButton("保存", this);
    m_cancelButton = new QPushButton("取消", this);

    connect(m_saveButton, &QPushButton::clicked, this, &SecurityLevelEditDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SecurityLevelEditDialog::onCancelClicked);

    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Disable level_code editing for existing records
    if (m_level.getId() > 0) {
        m_levelCodeEdit->setReadOnly(true);
        m_levelCodeEdit->setStyleSheet("background-color: #f0f0f0;");
    }
}

void SecurityLevelEditDialog::onSaveClicked()
{
    // Validate input
    if (m_levelCodeEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "密级编号不能为空！");
        m_levelCodeEdit->setFocus();
        return;
    }

    if (m_levelNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "密级名称不能为空！");
        m_levelNameEdit->setFocus();
        return;
    }

    m_level.setLevelCode(m_levelCodeEdit->text().trimmed());
    m_level.setLevelName(m_levelNameEdit->text().trimmed());
    m_level.setRetentionDays(m_retentionDaysSpin->value());
    m_level.setTimeoutMinutes(m_timeoutMinutesSpin->value());
    m_level.setDescription(m_descriptionEdit->toPlainText().trimmed());
    m_level.setActive(m_activeCheck->isChecked());

    accept();
}

void SecurityLevelEditDialog::onCancelClicked()
{
    reject();
}

SecurityLevel SecurityLevelEditDialog::getSecurityLevel() const
{
    return m_level;
}

// SecurityLevelConfigPage Implementation

SecurityLevelConfigPage::SecurityLevelConfigPage(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_titleLabel(nullptr)
    , m_levelTable(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
{
    setupUI();
    loadData();
}

void SecurityLevelConfigPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("密级设置", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_layout->addWidget(m_titleLabel);

    // Description label
    QLabel* descLabel = new QLabel("管理系统密级配置，用于定义不同密级文档的保存期限和超时时间。", this);
    descLabel->setStyleSheet("color: #666; margin-bottom: 10px;");
    m_layout->addWidget(descLabel);

    // Toolbar
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->addStretch();

    m_addButton = new QPushButton("添加", this);
    m_editButton = new QPushButton("编辑", this);
    m_deleteButton = new QPushButton("删除", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_addButton, &QPushButton::clicked, this, &SecurityLevelConfigPage::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &SecurityLevelConfigPage::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &SecurityLevelConfigPage::onDeleteClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &SecurityLevelConfigPage::onRefreshClicked);

    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addWidget(m_refreshButton);

    m_layout->addLayout(toolbarLayout);

    // Table
    m_levelTable = new QTableWidget(0, 6, this);
    m_levelTable->setHorizontalHeaderLabels({"密级编号", "密级名称", "保存天数", "超时分钟数", "状态", "描述"});
    m_levelTable->horizontalHeader()->setStretchLastSection(true);
    m_levelTable->verticalHeader()->setVisible(false);
    m_levelTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_levelTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_levelTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Set column widths
    m_levelTable->setColumnWidth(0, 80);
    m_levelTable->setColumnWidth(1, 100);
    m_levelTable->setColumnWidth(2, 80);
    m_levelTable->setColumnWidth(3, 100);
    m_levelTable->setColumnWidth(4, 60);

    connect(m_levelTable, &QTableWidget::cellDoubleClicked, this, &SecurityLevelConfigPage::onTableDoubleClicked);

    m_layout->addWidget(m_levelTable);

    // Status label
    QLabel* statusLabel = new QLabel("提示: 双击表格行可直接编辑，删除操作将禁用密级而非物理删除。", this);
    statusLabel->setStyleSheet("color: #999; font-size: 12px;");
    m_layout->addWidget(statusLabel);

    m_layout->addStretch();
}

void SecurityLevelConfigPage::loadData()
{
    SecurityLevelService& service = SecurityLevelService::getInstance();
    QList<SecurityLevel> levels = service.getAllLevels();
    refreshTable(levels);
}

void SecurityLevelConfigPage::refreshTable(const QList<SecurityLevel>& levels)
{
    m_levelTable->setRowCount(0);

    for (int i = 0; i < levels.count(); ++i) {
        const SecurityLevel& level = levels[i];

        m_levelTable->insertRow(i);

        m_levelTable->setItem(i, 0, new QTableWidgetItem(level.getLevelCode()));
        m_levelTable->setItem(i, 1, new QTableWidgetItem(level.getLevelName()));
        m_levelTable->setItem(i, 2, new QTableWidgetItem(QString::number(level.getRetentionDays())));
        m_levelTable->setItem(i, 3, new QTableWidgetItem(QString::number(level.getTimeoutMinutes())));
        m_levelTable->setItem(i, 4, new QTableWidgetItem(level.isActive() ? "启用" : "禁用"));
        m_levelTable->setItem(i, 5, new QTableWidgetItem(level.getDescription()));

        // Store level ID in the row
        m_levelTable->item(i, 0)->setData(Qt::UserRole, level.getId());

        // Gray out inactive rows
        if (!level.isActive()) {
            for (int col = 0; col < m_levelTable->columnCount(); ++col) {
                QTableWidgetItem* item = m_levelTable->item(i, col);
                if (item) {
                    item->setForeground(QBrush(Qt::gray));
                }
            }
        }
    }
}

SecurityLevel SecurityLevelConfigPage::getSelectedLevel() const
{
    SecurityLevel level;

    QList<QTableWidgetItem*> selectedItems = m_levelTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return level;
    }

    int row = selectedItems.first()->row();
    int levelId = m_levelTable->item(row, 0)->data(Qt::UserRole).toInt();

    SecurityLevelService& service = SecurityLevelService::getInstance();
    return service.getLevel(levelId);
}

void SecurityLevelConfigPage::onAddClicked()
{
    SecurityLevelEditDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        SecurityLevel newLevel = dialog.getSecurityLevel();

        SecurityLevelService& service = SecurityLevelService::getInstance();
        QString validationError = service.validateLevel(newLevel);

        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, "验证错误", validationError);
            return;
        }

        if (service.createLevel(newLevel)) {
            QMessageBox::information(this, "成功", "密级添加成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "密级添加失败，请检查密级编号是否已存在。");
        }
    }
}

void SecurityLevelConfigPage::onEditClicked()
{
    SecurityLevel level = getSelectedLevel();
    if (level.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的密级！");
        return;
    }

    SecurityLevelEditDialog dialog(this, level);

    if (dialog.exec() == QDialog::Accepted) {
        SecurityLevel updatedLevel = dialog.getSecurityLevel();

        SecurityLevelService& service = SecurityLevelService::getInstance();
        QString validationError = service.validateLevel(updatedLevel);

        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, "验证错误", validationError);
            return;
        }

        if (service.updateLevel(updatedLevel)) {
            QMessageBox::information(this, "成功", "密级更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "密级更新失败。");
        }
    }
}

void SecurityLevelConfigPage::onDeleteClicked()
{
    SecurityLevel level = getSelectedLevel();
    if (level.getId() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的密级！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要禁用密级 '%1' (%2) 吗？\n\n注意：此操作将禁用该密级而非物理删除。")
            .arg(level.getLevelName())
            .arg(level.getLevelCode()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        SecurityLevelService& service = SecurityLevelService::getInstance();
        if (service.deactivateLevel(level.getId())) {
            QMessageBox::information(this, "成功", "密级已禁用！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "操作失败。");
        }
    }
}

void SecurityLevelConfigPage::onRefreshClicked()
{
    loadData();
}

void SecurityLevelConfigPage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    int levelId = m_levelTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (levelId <= 0) {
        return;
    }

    SecurityLevelService& service = SecurityLevelService::getInstance();
    SecurityLevel level = service.getLevel(levelId);

    if (level.getId() <= 0) {
        return;
    }

    SecurityLevelEditDialog dialog(this, level);

    if (dialog.exec() == QDialog::Accepted) {
        SecurityLevel updatedLevel = dialog.getSecurityLevel();

        QString validationError = service.validateLevel(updatedLevel);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(this, "验证错误", validationError);
            return;
        }

        if (service.updateLevel(updatedLevel)) {
            QMessageBox::information(this, "成功", "密级更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "密级更新失败。");
        }
    }
}
