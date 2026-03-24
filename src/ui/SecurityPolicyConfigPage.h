#ifndef SECURITYPOLICYCONFIGPAGE_H
#define SECURITYPOLICYCONFIGPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>

// Security Policy Configuration Page
class SecurityPolicyConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit SecurityPolicyConfigPage(QWidget* parent = nullptr);

private slots:
    void onSaveClicked();
    void onResetClicked();

private:
    void setupUI();
    void setupAuthGroup();
    void setupPasswordGroup();
    void setupSessionGroup();
    void loadSettings();
    void saveSettings();
    bool validateInput();

    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;

    // Authentication Settings
    QGroupBox* m_authGroup;
    QSpinBox* m_authAttemptsSpin;
    QComboBox* m_lockDurationCombo;

    // Password Policy Settings
    QGroupBox* m_passwordGroup;
    QCheckBox* m_requireUpperCase;
    QCheckBox* m_requireLowerCase;
    QCheckBox* m_requireDigit;
    QCheckBox* m_requireSpecial;
    QSpinBox* m_passwordLengthSpin;
    QSpinBox* m_passwordExpirySpin;

    // Session Settings
    QGroupBox* m_sessionGroup;
    QSpinBox* m_sessionTimeoutSpin;

    // Buttons
    QPushButton* m_saveButton;
    QPushButton* m_resetButton;
};

#endif // SECURITYPOLICYCONFIGPAGE_H
