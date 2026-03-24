#ifndef BARCODECONFIGPAGE_H
#define BARCODECONFIGPAGE_H

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
#include <QCheckBox>
#include <QListWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include "../models/barcode_config_model.h"
#include "../services/BarcodeService.h"

class BarcodeConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit BarcodeConfigPage(QWidget *parent = nullptr);

private slots:
    void loadConfigs();
    void savePrintConfig();
    void saveBurnConfig();
    void refreshDisplay();
    void updatePrintFieldSelection();
    void updateBurnFieldSelection();
    void onPrintBarcodeTypeChanged(int index);
    void onBurnBarcodeTypeChanged(int index);

private:
    void setupUI();
    void setupPrintConfigGroup();
    void setupBurnConfigGroup();

    // UI Components - Print Config
    QGroupBox *m_printConfigGroup;
    QComboBox *m_printBarcodeTypeCombo;
    QSpinBox *m_printBarcodeWidthSpin;
    QSpinBox *m_printBarcodeHeightSpin;
    QSpinBox *m_printQrcodeSizeSpin;
    QLineEdit *m_printCustomTextEdit;
    QListWidget *m_printFieldsList;
    QCheckBox *m_printIsActiveCheck;
    QPushButton *m_printSaveButton;
    QLabel *m_printBarcodeSizeLabel;
    QLabel *m_printQrcodeSizeLabel;

    // UI Components - Burn Config
    QGroupBox *m_burnConfigGroup;
    QComboBox *m_burnBarcodeTypeCombo;
    QSpinBox *m_burnBarcodeWidthSpin;
    QSpinBox *m_burnBarcodeHeightSpin;
    QSpinBox *m_burnQrcodeSizeSpin;
    QLineEdit *m_burnCustomTextEdit;
    QListWidget *m_burnFieldsList;
    QCheckBox *m_burnIsActiveCheck;
    QPushButton *m_burnSaveButton;
    QLabel *m_burnBarcodeSizeLabel;
    QLabel *m_burnQrcodeSizeLabel;

    // Service
    BarcodeService &m_service;

    // Config IDs
    int m_printConfigId;
    int m_burnConfigId;
};

#endif // BARCODECONFIGPAGE_H
