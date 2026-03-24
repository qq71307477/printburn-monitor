#ifndef ROLEMANAGEMENTPAGE_H
#define ROLEMANAGEMENTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QTreeWidget>
#include <QCheckBox>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QTextEdit>
#include "models/role_model.h"
#include "services/RoleManagementService.h"

// Role Edit Dialog
class RoleEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoleEditDialog(QWidget* parent = nullptr, const Role& role = Role());
    Role getRole() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onSelectAllPermissions();
    void onClearAllPermissions();

private:
    void setupUI();
    void setupPermissionTree();
    void checkPermissionRecursive(QTreeWidgetItem* item, const QStringList& perms);
    void collectPermissionsRecursive(QTreeWidgetItem* item, QStringList& perms);
    void setCheckStateRecursive(QTreeWidgetItem* item, Qt::CheckState state);

    Role m_role;
    bool m_isEdit;

    // UI Components
    QLineEdit* m_nameEdit;
    QSpinBox* m_displayOrderSpin;
    QCheckBox* m_activeCheck;
    QTreeWidget* m_permissionTree;
    QTextEdit* m_descriptionEdit;
};

// Main Role Management Page
class RoleManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit RoleManagementPage(QWidget* parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onSearchClicked();
    void onResetClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupSearchArea();
    void setupTable();
    void setupToolbar();
    void loadData();
    void refreshTable(const QList<Role>& roles);
    Role getSelectedRole() const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;

    // Search Area
    QLineEdit* m_keywordEdit;
    QComboBox* m_statusFilterCombo;
    QDateTimeEdit* m_startTimeEdit;
    QDateTimeEdit* m_endTimeEdit;

    // Table
    QTableWidget* m_roleTable;

    // Toolbar
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_refreshButton;
};

#endif // ROLEMANAGEMENTPAGE_H
