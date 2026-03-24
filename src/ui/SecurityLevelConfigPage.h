#ifndef SECURITYLEVELCONFIGPAGE_H
#define SECURITYLEVELCONFIGPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include "models/security_level_model.h"
#include "services/SecurityLevelService.h"

class SecurityLevelEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SecurityLevelEditDialog(QWidget* parent = nullptr, const SecurityLevel& level = SecurityLevel());
    SecurityLevel getSecurityLevel() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();

    SecurityLevel m_level;

    // UI Components
    QLineEdit* m_levelCodeEdit;
    QLineEdit* m_levelNameEdit;
    QSpinBox* m_retentionDaysSpin;
    QSpinBox* m_timeoutMinutesSpin;
    QTextEdit* m_descriptionEdit;
    QCheckBox* m_activeCheck;

    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
};

class SecurityLevelConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit SecurityLevelConfigPage(QWidget* parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefreshClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void loadData();
    SecurityLevel getSelectedLevel() const;
    void refreshTable(const QList<SecurityLevel>& levels);

    // UI Components
    QVBoxLayout* m_layout;
    QLabel* m_titleLabel;
    QTableWidget* m_levelTable;

    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_refreshButton;
};

#endif // SECURITYLEVELCONFIGPAGE_H
