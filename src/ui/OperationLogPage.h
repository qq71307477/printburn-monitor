#ifndef OPERATIONLOGPAGE_H
#define OPERATIONLOGPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QDialog>
#include <QTextEdit>
#include <QSpinBox>
#include "services/LogAuditService.h"

// Log Detail Dialog
class LogDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogDetailDialog(QWidget* parent = nullptr, const AuditLogEntry& log = AuditLogEntry());

private slots:
    void onCloseClicked();

private:
    void setupUI();
    QString formatLogDetails(const AuditLogEntry& log) const;

    AuditLogEntry m_log;
    QTextEdit* m_detailText;
};

// Main Operation Log Page
class OperationLogPage : public QWidget
{
    Q_OBJECT

public:
    explicit OperationLogPage(QWidget* parent = nullptr);

private slots:
    void onSearchClicked();
    void onResetClicked();
    void onExportClicked();
    void onRefreshClicked();
    void onPrevPageClicked();
    void onNextPageClicked();
    void onFirstPageClicked();
    void onLastPageClicked();
    void onPageSizeChanged(int index);
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupSearchArea();
    void setupTable();
    void setupToolbar();
    void setupPagination();
    void loadData();
    void refreshTable(const QList<AuditLogEntry>& logs);
    void updatePaginationInfo();
    AuditLogEntry getSelectedLog() const;
    QString getOperationModule(const QString& eventType) const;
    QString getOperationResult(const QString& severity) const;
    QString getDepartmentName(int userId) const;
    QString getRoleNames(int userId) const;
    QString parseOSFromUserAgent(const QString& userAgent) const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;

    // Search Area
    QGroupBox* m_searchGroup;
    QComboBox* m_moduleFilterCombo;
    QLineEdit* m_usernameEdit;
    QComboBox* m_resultFilterCombo;
    QDateTimeEdit* m_startTimeEdit;
    QDateTimeEdit* m_endTimeEdit;

    // Table
    QTableWidget* m_logTable;

    // Toolbar
    QPushButton* m_exportButton;
    QPushButton* m_refreshButton;

    // Pagination
    QWidget* m_paginationWidget;
    QPushButton* m_firstPageBtn;
    QPushButton* m_prevPageBtn;
    QPushButton* m_nextPageBtn;
    QPushButton* m_lastPageBtn;
    QLabel* m_pageInfoLabel;
    QComboBox* m_pageSizeCombo;
    QSpinBox* m_pageSpinBox;

    // Data
    QList<AuditLogEntry> m_allLogs;
    int m_currentPage;
    int m_pageSize;
    int m_totalCount;
};

#endif // OPERATIONLOGPAGE_H