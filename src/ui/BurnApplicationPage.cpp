#include "BurnApplicationPage.h"
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
#include <QRadioButton>
#include <QButtonGroup>
#include <QFileInfo>

BurnApplicationPage::BurnApplicationPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void BurnApplicationPage::setupUI()
{
    m_layout = new QVBoxLayout(this);

    QLabel *pageTitle = new QLabel("刻录申请", this);
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

void BurnApplicationPage::setupApplicationForm()
{
    m_applicationFormGroup = new QGroupBox("刻录申请表单", this);
    m_formLayout = new QFormLayout(m_applicationFormGroup);

    // 文档标题
    m_documentTitleEdit = new QLineEdit(this);
    m_documentTitleEdit->setPlaceholderText("请输入文档标题");
    m_formLayout->addRow("文档标题:", m_documentTitleEdit);

    // 刻录目的
    m_purposeEdit = new QTextEdit(this);
    m_purposeEdit->setFixedHeight(60);
    m_purposeEdit->setPlaceholderText("请简要说明刻录目的");
    m_formLayout->addRow("刻录目的:", m_purposeEdit);

    // 介质类型
    m_mediaTypeCombo = new QComboBox(this);
    m_mediaTypeCombo->addItem("CD-R", 0);
    m_mediaTypeCombo->addItem("DVD-R", 1);
    m_mediaTypeCombo->addItem("BD-R", 2);
    m_mediaTypeCombo->addItem("CD-RW", 3);
    m_mediaTypeCombo->addItem("DVD-RW", 4);
    m_formLayout->addRow("介质类型:", m_mediaTypeCombo);

    // 会话模式
    QHBoxLayout *sessionLayout = new QHBoxLayout();
    m_singleSessionRadio = new QRadioButton("单会话", this);
    m_multiSessionRadio = new QRadioButton("多会话", this);
    m_singleSessionRadio->setChecked(true);

    m_sessionButtonGroup = new QButtonGroup(this);
    m_sessionButtonGroup->addButton(m_singleSessionRadio, 0);
    m_sessionButtonGroup->addButton(m_multiSessionRadio, 1);

    sessionLayout->addWidget(m_singleSessionRadio);
    sessionLayout->addWidget(m_multiSessionRadio);
    sessionLayout->addStretch();
    m_formLayout->addRow("会话模式:", sessionLayout);

    // 刻录份数
    m_copiesSpin = new QSpinBox(this);
    m_copiesSpin->setRange(1, 99);
    m_copiesSpin->setValue(1);
    m_formLayout->addRow("刻录份数:", m_copiesSpin);

    // 截止日期
    m_deadlineEdit = new QDateEdit(this);
    m_deadlineEdit->setDate(QDate::currentDate().addDays(7));
    m_deadlineEdit->setDisplayFormat("yyyy-MM-dd");
    m_deadlineEdit->setCalendarPopup(true);
    m_formLayout->addRow("截止日期:", m_deadlineEdit);

    // 文件路径
    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_filePathsEdit = new QLineEdit(this);
    m_filePathsEdit->setReadOnly(true);
    m_filePathsEdit->setPlaceholderText("请选择要刻录的文件/文件夹");
    m_browseButton = new QPushButton("浏览", this);
    connect(m_browseButton, &QPushButton::clicked, this, &BurnApplicationPage::browseFiles);

    fileLayout->addWidget(m_filePathsEdit);
    fileLayout->addWidget(m_browseButton);
    m_formLayout->addRow("选择文件:", fileLayout);

    // 提交按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_submitButton = new QPushButton("提交申请", this);
    m_clearButton = new QPushButton("清空", this);

    connect(m_submitButton, &QPushButton::clicked, this, &BurnApplicationPage::submitApplication);
    connect(m_clearButton, &QPushButton::clicked, [this]() {
        m_documentTitleEdit->clear();
        m_purposeEdit->clear();
        m_mediaTypeCombo->setCurrentIndex(0);
        m_singleSessionRadio->setChecked(true);
        m_copiesSpin->setValue(1);
        m_deadlineEdit->setDate(QDate::currentDate().addDays(7));
        m_filePathsEdit->clear();
    });

    buttonLayout->addWidget(m_submitButton);
    buttonLayout->addWidget(m_clearButton);
    m_formLayout->addRow(buttonLayout);

    m_layout->addWidget(m_applicationFormGroup);
}

void BurnApplicationPage::setupDocumentPreview()
{
    m_documentPreviewGroup = new QGroupBox("文件预览", this);
    QVBoxLayout *previewLayout = new QVBoxLayout(m_documentPreviewGroup);

    m_previewTable = new QTableWidget(0, 3, this);
    m_previewTable->setHorizontalHeaderLabels({"文件名", "大小", "类型"});
    m_previewTable->horizontalHeader()->setStretchLastSection(true);
    m_previewTable->verticalHeader()->setVisible(false);
    m_previewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_previewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    previewLayout->addWidget(m_previewTable);
    m_layout->addWidget(m_documentPreviewGroup);
}

void BurnApplicationPage::setupApplicationHistory()
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
        m_historyTable->setItem(i, 1, new QTableWidgetItem("光盘内容 " + QString::number(i+1)));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(i == 0 ? "已批准" : (i == 1 ? "待审批" : "已拒绝")));
        m_historyTable->setItem(i, 3, new QTableWidgetItem(QString::number(i+1)));
        m_historyTable->setItem(i, 4, new QTableWidgetItem("申请人 " + QString::number(i+1)));
    }

    historyLayout->addWidget(m_historyTable);
    m_layout->addWidget(m_historyGroup);
}

void BurnApplicationPage::browseFiles()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "选择要刻录的文件",
        "",
        "所有文件 (*.*);;文档文件 (*.pdf *.doc *.docx *.txt);;图片文件 (*.jpg *.png *.gif);;视频文件 (*.mp4 *.avi *.mkv)"
    );

    if (!fileNames.isEmpty()) {
        m_filePathsEdit->setText(QString("%1 个文件").arg(fileNames.size()));

        // 更新预览表
        m_previewTable->setRowCount(0); // 清空现有行
        for (const QString &fileName : fileNames) {
            QFileInfo fileInfo(fileName);
            int row = m_previewTable->rowCount();
            m_previewTable->insertRow(row);
            m_previewTable->setItem(row, 0, new QTableWidgetItem(fileInfo.fileName()));
            m_previewTable->setItem(row, 1, new QTableWidgetItem(QString::number(fileInfo.size())));
            m_previewTable->setItem(row, 2, new QTableWidgetItem(fileInfo.suffix()));
        }
    }
}

void BurnApplicationPage::submitApplication()
{
    if (m_documentTitleEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入文档标题！");
        return;
    }

    if (m_filePathsEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要刻录的文件！");
        return;
    }

    // 这里应该是实际的提交逻辑
    // 在真实环境中，应该调用任务插件创建刻录任务
    QMessageBox::information(this, "成功", "刻录申请已提交，请等待审批！");

    // 清空表单
    m_documentTitleEdit->clear();
    m_purposeEdit->clear();
    m_mediaTypeCombo->setCurrentIndex(0);
    m_singleSessionRadio->setChecked(true);
    m_copiesSpin->setValue(1);
    m_deadlineEdit->setDate(QDate::currentDate().addDays(7));
    m_filePathsEdit->clear();
    m_previewTable->setRowCount(0);
}