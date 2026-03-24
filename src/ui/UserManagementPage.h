#ifndef USERMANAGEMENTPAGE_H
#define USERMANAGEMENTPAGE_H

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
#include <QFormLayout>
#include <QSpinBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QListWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include "models/user_model.h"
#include "models/role_model.h"
#include "models/department_model.h"
#include "services/UserManagementService.h"

// Forward declarations
class QCheckBox;

// User Edit Dialog
class UserEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserEditDialog(QWidget* parent = nullptr, const User& user = User());
    User getUser() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void loadDepartments();
    void loadRoles();

    User m_user;
    bool m_isEdit;

    // UI Components
    QLineEdit* m_usernameEdit;
    QLineEdit* m_firstNameEdit;
    QLineEdit* m_lastNameEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_phoneEdit;
    QLineEdit* m_cardNumberEdit;
    QLineEdit* m_passwordEdit;
    QComboBox* m_departmentCombo;
    QComboBox* m_securityLevelCombo;
    QCheckBox* m_activeCheck;
    QListWidget* m_rolesList;
    QSpinBox* m_displayOrderSpin;
};

// Import Users Dialog
class ImportUsersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportUsersDialog(QWidget* parent = nullptr);
    QString getFilePath() const;
    QList<User> getParsedUsers() const;
    QString getImportSummary() const;

private slots:
    void onBrowseClicked();
    void onImportClicked();
    void onCancelClicked();

private:
    void setupUI();
    bool parseCSVFile(const QString& filePath);
    int getDepartmentIdByName(const QString& name) const;
    int getRoleIdByName(const QString& name) const;

    QString m_filePath;
    QLineEdit* m_filePathEdit;
    QList<User> m_parsedUsers;
    QString m_importSummary;
};

// Reset Password Dialog
class ResetPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetPasswordDialog(QWidget* parent = nullptr, const QString& username = "");
    QString getNewPassword() const;

private slots:
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUI();

    QLineEdit* m_newPasswordEdit;
    QLineEdit* m_confirmPasswordEdit;
    QString m_username;
};

// Main User Management Page
class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget* parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onSearchClicked();
    void onResetClicked();
    void onResetPasswordClicked();
    void onBatchImportClicked();
    void onBatchExportClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupSearchArea();
    void setupTable();
    void setupToolbar();
    void loadData();
    void refreshTable(const QList<User>& users);
    User getSelectedUser() const;
    QString getDepartmentName(int departmentId) const;
    QString getRoleNames(int userId) const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;

    // Search Area
    QGroupBox* m_searchGroup;
    QLineEdit* m_keywordEdit;
    QComboBox* m_departmentFilterCombo;
    QComboBox* m_statusFilterCombo;
    QComboBox* m_roleFilterCombo;
    QDateTimeEdit* m_startTimeEdit;
    QDateTimeEdit* m_endTimeEdit;

    // Table
    QTableWidget* m_userTable;

    // Toolbar
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_refreshButton;
    QPushButton* m_resetPasswordButton;
    QPushButton* m_batchImportButton;
    QPushButton* m_batchExportButton;
};

#endif // USERMANAGEMENTPAGE_H
