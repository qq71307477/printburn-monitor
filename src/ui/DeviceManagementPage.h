#ifndef DEVICEMANAGEMENTPAGE_H
#define DEVICEMANAGEMENTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QDialog>
#include <QFormLayout>
#include <QTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QListWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include <QSpinBox>
#include "models/device_model.h"
#include "services/DeviceManagementService.h"

// Device Edit Dialog
class DeviceEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceEditDialog(QWidget* parent = nullptr, const Device& device = Device());
    Device getDevice() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();

    Device m_device;
    bool m_isEdit;

    // UI Components
    QLineEdit* m_nameEdit;
    QLineEdit* m_serialNumberEdit;
    QComboBox* m_deviceTypeCombo;
    QLineEdit* m_ipAddressEdit;
    QLineEdit* m_locationEdit;
    QTextEdit* m_specificationsEdit;
    QComboBox* m_statusCombo;
    QCheckBox* m_monitoredCheck;
};

// Device Authorization Dialog
class DeviceAuthorizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceAuthorizationDialog(QWidget* parent = nullptr, int deviceId = 0, const QString& deviceName = QString());
    QList<int> getAuthorizedUserIds() const;
    QList<int> getAuthorizedDepartmentIds() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onTabChanged(int index);

private:
    void setupUI();
    void loadUsers();
    void loadDepartments();
    void loadCurrentAuthorizations();

    int m_deviceId;
    QString m_deviceName;

    // UI Components
    QTabWidget* m_tabWidget;
    QListWidget* m_usersList;
    QTreeWidget* m_departmentsTree;
    QList<int> m_authorizedUserIds;
    QList<int> m_authorizedDepartmentIds;
};

// Main Device Management Page
class DeviceManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceManagementPage(QWidget* parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onSearchClicked();
    void onResetClicked();
    void onAuthorizeClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupSearchArea();
    void setupTable();
    void setupToolbar();
    void loadData();
    void refreshTable(const QList<Device>& devices);
    Device getSelectedDevice() const;
    QString getDeviceTypeDisplay(const QString& deviceType) const;
    QString getStatusDisplay(const QString& status) const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;

    // Search Area
    QGroupBox* m_searchGroup;
    QLineEdit* m_keywordEdit;
    QComboBox* m_deviceTypeFilterCombo;
    QComboBox* m_statusFilterCombo;

    // Table
    QTableWidget* m_deviceTable;

    // Toolbar
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_authorizeButton;
    QPushButton* m_refreshButton;
};

#endif // DEVICEMANAGEMENTPAGE_H