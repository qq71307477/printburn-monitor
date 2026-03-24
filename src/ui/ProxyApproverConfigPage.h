#ifndef PROXY_APPROVER_CONFIG_PAGE_H
#define PROXY_APPROVER_CONFIG_PAGE_H

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
#include <QDateEdit>
#include "models/proxy_approver_model.h"

class ProxyApproverDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProxyApproverDialog(QWidget *parent = nullptr, int proxyId = -1);
    ProxyApprover getProxy() const;
    void setProxy(const ProxyApprover& proxy);
    void setUsers(const QList<QPair<int, QString>>& users);
    void setSecurityLevels(const QList<QPair<int, QString>>& levels);
    void setCurrentUserId(int userId);

private:
    void setupUI();

    int m_proxyId;
    int m_currentUserId;
    QComboBox *m_proxyUserCombo;
    QSpinBox *m_minSecurityLevelSpin;
    QSpinBox *m_maxSecurityLevelSpin;
    QComboBox *m_taskTypeCombo;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QCheckBox *m_enabledCheck;
    QDialogButtonBox *m_buttonBox;

    QList<QPair<int, QString>> m_users;
    QList<QPair<int, QString>> m_securityLevels;
};

class ProxyApproverConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProxyApproverConfigPage(QWidget *parent = nullptr);
    void setCurrentUserId(int userId);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onToggleEnabledClicked();
    void onRefreshClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupTable();
    void loadData();
    void updateTable();
    QList<QPair<int, QString>> loadUsers();
    QList<QPair<int, QString>> loadSecurityLevels();
    QString getUserName(int userId) const;
    QString getProxyStatus(const ProxyApprover& proxy) const;

    // UI Components
    QVBoxLayout *m_layout;

    // Header
    QHBoxLayout *m_headerLayout;
    QLabel *m_pageTitle;
    QPushButton *m_refreshButton;

    // Table
    QTableWidget *m_proxyTable;

    // Button area
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_toggleEnabledButton;

    // Data
    QList<ProxyApprover> m_proxies;
    QList<QPair<int, QString>> m_users;
    QList<QPair<int, QString>> m_securityLevels;
    int m_currentUserId;
};

#endif // PROXY_APPROVER_CONFIG_PAGE_H
