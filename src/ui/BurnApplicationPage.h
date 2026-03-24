#ifndef BURNAPPLICATIONPAGE_H
#define BURNAPPLICATIONPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QFileDialog>
#include <QTableWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStringList>

class BurnApplicationPage : public QWidget
{
    Q_OBJECT

public:
    explicit BurnApplicationPage(QWidget *parent = nullptr);

private slots:
    void browseFiles();
    void submitApplication();

private:
    void setupUI();
    void setupApplicationForm();
    void setupDocumentPreview();
    void setupApplicationHistory();
    void loadDevices();
    void loadSecurityLevels();
    void loadApprovers();

    // UI Components
    QVBoxLayout *m_layout;

    // Application Form
    QGroupBox *m_applicationFormGroup;
    QFormLayout *m_formLayout;
    QLineEdit *m_documentTitleEdit;
    QTextEdit *m_purposeEdit;
    QComboBox *m_deviceCombo;
    QComboBox *m_securityLevelCombo;
    QComboBox *m_approverCombo;
    QComboBox *m_mediaTypeCombo;
    QRadioButton *m_singleSessionRadio;
    QRadioButton *m_multiSessionRadio;
    QButtonGroup *m_sessionButtonGroup;
    QSpinBox *m_copiesSpin;
    QDateEdit *m_deadlineEdit;
    QLineEdit *m_filePathsEdit;
    QPushButton *m_browseButton;
    QStringList m_selectedFiles;

    // Document Preview
    QGroupBox *m_documentPreviewGroup;
    QTableWidget *m_previewTable;

    // Application History
    QGroupBox *m_historyGroup;
    QTableWidget *m_historyTable;

    QPushButton *m_submitButton;
    QPushButton *m_clearButton;
};

#endif // BURNAPPLICATIONPAGE_H