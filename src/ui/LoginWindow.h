#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSettings>

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSuccessful(const QString &username, const QString &role);

private slots:
    void attemptLogin();
    void togglePasswordVisibility();

private:
    void setupUI();
    void loadSavedCredentials();
    void saveCredentialsIfNeeded();

    // UI Elements
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginButton;
    QPushButton *m_cancelButton;
    QCheckBox *m_rememberMeCheckBox;
    QLabel *m_errorLabel;
    QPushButton *m_togglePasswordButton;

    QSettings *m_settings;
};

#endif // LOGINWINDOW_H