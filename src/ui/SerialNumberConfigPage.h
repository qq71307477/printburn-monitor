#ifndef SERIALNUMBERCONFIGPAGE_H
#define SERIALNUMBERCONFIGPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include "../models/serial_number_config_model.h"
#include "../services/SerialNumberService.h"

class SerialNumberConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit SerialNumberConfigPage(QWidget *parent = nullptr);

private slots:
    void loadConfigs();
    void savePrintConfig();
    void saveBurnConfig();
    void resetPrintSequence();
    void resetBurnSequence();
    void previewPrintSerialNumber();
    void previewBurnSerialNumber();
    void refreshDisplay();

private:
    void setupUI();
    void setupPrintConfigGroup();
    void setupBurnConfigGroup();
    void setupPreviewArea();

    // UI Components - Print Config
    QGroupBox *m_printConfigGroup;
    QLineEdit *m_printPrefixEdit;
    QComboBox *m_printDateFormatCombo;
    QSpinBox *m_printSequenceLengthSpin;
    QSpinBox *m_printCurrentSequenceSpin;
    QComboBox *m_printResetPeriodCombo;
    QDateEdit *m_printLastResetDateEdit;
    QCheckBox *m_printIsActiveCheck;
    QPushButton *m_printSaveButton;
    QPushButton *m_printResetButton;
    QPushButton *m_printPreviewButton;

    // UI Components - Burn Config
    QGroupBox *m_burnConfigGroup;
    QLineEdit *m_burnPrefixEdit;
    QComboBox *m_burnDateFormatCombo;
    QSpinBox *m_burnSequenceLengthSpin;
    QSpinBox *m_burnCurrentSequenceSpin;
    QComboBox *m_burnResetPeriodCombo;
    QDateEdit *m_burnLastResetDateEdit;
    QCheckBox *m_burnIsActiveCheck;
    QPushButton *m_burnSaveButton;
    QPushButton *m_burnResetButton;
    QPushButton *m_burnPreviewButton;

    // UI Components - Preview
    QLabel *m_printPreviewLabel;
    QLabel *m_burnPreviewLabel;

    // Service
    SerialNumberService &m_service;

    // Config IDs
    int m_printConfigId;
    int m_burnConfigId;
};

#endif // SERIALNUMBERCONFIGPAGE_H
