#ifndef LOGINLOGPAGE_H
#define LOGINLOGPAGE_H

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
#include <QSpinBox>
#include "services/LogAuditService.h"

// Main Login Log Page
class LoginLogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginLogPage(QWidget* parent = nullptr);

private slots:
    void onSearchClicked();
    void onResetClicked();
    void onExportClicked();
    void onRefreshClicked();
    void onPrevPageClicked();
    void onNextPageClicked();
    void onPageSizeChanged(int index);

private:
    void setupUI();
    void setupSearchArea();
    void setupTable();
    void setupToolbar();
    void setupPagination();
    void loadData();
    void refreshTable(const QList<AuditLogEntry>& logs);
    void updatePaginationInfo();
    QString getDepartmentName(int userId) const;
    QString getRoleNames(int userId) const;
    QString getOperatingSystem(const QString& userAgent) const;
    QString getOperationType(const QString& eventType) const;
    QString getOperationResult(const QString& severity) const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;

    // Search Area
    QGroupBox* m_searchGroup;
    QLineEdit* m_usernameEdit;
    QDateTimeEdit* m_startTimeEdit;
    QDateTimeEdit* m_endTimeEdit;
    QComboBox* m_operationTypeCombo;
    QComboBox* m_resultCombo;

    // Table
    QTableWidget* m_logTable;

    // Pagination
    int m_currentPage;
    int m_pageSize;
    int m_totalCount;
    QSpinBox* m_pageSpinBox;
    QLabel* m_pageInfoLabel;
    QPushButton* m_prevPageButton;
    QPushButton* m_nextPageButton;
    QComboBox* m_pageSizeCombo;

    // Toolbar
    QPushButton* m_searchButton;
    QPushButton* m_resetButton;
    QPushButton* m_exportButton;
    QPushButton* m_refreshButton;
};

#endif // LOGINLOGPAGE_H