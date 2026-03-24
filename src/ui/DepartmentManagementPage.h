#ifndef DEPARTMENTMANAGEMENTPAGE_H
#define DEPARTMENTMANAGEMENTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QMessageBox>
#include "models/department_model.h"

// Department Edit Dialog
class DepartmentEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DepartmentEditDialog(QWidget* parent = nullptr, const Department& dept = Department());
    Department getDepartment() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();

    Department m_dept;
    bool m_isEdit;

    // UI Components
    QLineEdit* m_nameEdit;
    QLineEdit* m_descriptionEdit;
    QComboBox* m_parentCombo;
};

// Main Department Management Page
class DepartmentManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit DepartmentManagementPage(QWidget* parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onTreeItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    void setupUI();
    void setupToolbar();
    void setupTree();
    void loadData();
    void refreshTree(const std::vector<std::unique_ptr<Department>>& depts);
    Department getSelectedDepartment() const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;

    // Tree
    QTreeWidget* m_deptTree;

    // Toolbar
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_refreshButton;
};

#endif // DEPARTMENTMANAGEMENTPAGE_H
