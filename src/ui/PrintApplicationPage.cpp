#include "PrintApplicationPage.h"
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
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>

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
        m_previewLabel->setText("已选择文件: " + QFileInfo(fileName).fileName());
    }
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

    // 这里应该是实际的提交逻辑
    // 在真实环境中，应该调用任务插件创建打印任务
    QMessageBox::information(this, "成功", "打印申请已提交，请等待审批！");

    // 清空表单
    m_documentTitleEdit->clear();
    m_purposeEdit->clear();
    m_priorityCombo->setCurrentIndex(0);
    m_copiesSpin->setValue(1);
    m_deadlineEdit->setDate(QDate::currentDate().addDays(7));
    m_filePathEdit->clear();
    m_previewLabel->setText("文档预览将在选择文件后显示");
}