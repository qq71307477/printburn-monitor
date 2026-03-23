#ifndef PRINTAPPLICATIONPAGE_H
#define PRINTAPPLICATIONPAGE_H

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

class PrintApplicationPage : public QWidget
{
    Q_OBJECT

public:
    explicit PrintApplicationPage(QWidget *parent = nullptr);

private slots:
    void browseFile();
    void submitApplication();

private:
    void setupUI();
    void setupApplicationForm();
    void setupDocumentPreview();
    void setupApplicationHistory();

    // UI Components
    QVBoxLayout *m_layout;

    // Application Form
    QGroupBox *m_applicationFormGroup;
    QFormLayout *m_formLayout;
    QLineEdit *m_documentTitleEdit;
    QTextEdit *m_purposeEdit;
    QComboBox *m_priorityCombo;
    QSpinBox *m_copiesSpin;
    QDateEdit *m_deadlineEdit;
    QLineEdit *m_filePathEdit;
    QPushButton *m_browseButton;

    // Document Preview
    QGroupBox *m_documentPreviewGroup;
    QLabel *m_previewLabel;

    // Application History
    QGroupBox *m_historyGroup;
    QTableWidget *m_historyTable;

    QPushButton *m_submitButton;
    QPushButton *m_clearButton;
};

#endif // PRINTAPPLICATIONPAGE_H