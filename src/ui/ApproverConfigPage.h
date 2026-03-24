#ifndef APPROVER_CONFIG_PAGE_H
#define APPROVER_CONFIG_PAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include "models/approver_config_model.h"

class ApproverConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ApproverConfigDialog(QWidget *parent = nullptr, int configId = -1);
    ApproverConfig getConfig() const;
    void setConfig(const ApproverConfig& config);
    void setApproverRoles(const QList<QPair<int, QString>>& roles);

private:
    void setupUI();

    int m_configId;
    QComboBox *m_taskTypeCombo;
    QComboBox *m_approverRoleCombo;
    QSpinBox *m_minSecurityLevelSpin;
    QSpinBox *m_maxSecurityLevelSpin;
    QCheckBox *m_activeCheck;
    QDialogButtonBox *m_buttonBox;
};

class ApproverConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApproverConfigPage(QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupTable();
    void loadData();
    void updateTable();
    QList<QPair<int, QString>> loadApproverRoles();
    QString getRoleName(int roleId) const;

    // UI Components
    QVBoxLayout *m_layout;

    // Header
    QHBoxLayout *m_headerLayout;
    QLabel *m_pageTitle;
    QPushButton *m_refreshButton;

    // Table
    QTableWidget *m_configTable;

    // Button area
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;

    // Data
    QList<ApproverConfig> m_configs;
    QList<QPair<int, QString>> m_approverRoles;
};

#endif // APPROVER_CONFIG_PAGE_H
