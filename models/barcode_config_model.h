#ifndef BARCODE_CONFIG_MODEL_H
#define BARCODE_CONFIG_MODEL_H

#include <QString>
#include <QDateTime>
#include <QList>

// 条码内容字段枚举
namespace BarcodeField {
    constexpr int SERIAL_NUMBER = 0;    // 流水号 (必选)
    constexpr int CUSTOM_TEXT = 1;      // 自定义文本
    constexpr int DEPARTMENT = 2;       // 提交人部门
    constexpr int SUBMITTER = 3;        // 提交人
    constexpr int SECURITY_LEVEL = 4;   // 密级
    constexpr int COPIES = 5;           // 份数
    constexpr int FILENAME = 6;         // 文件名
}

// 条码类型枚举
namespace BarcodeType {
    constexpr int BARCODE = 0;   // 条形码 (Code128)
    constexpr int QRCODE = 1;    // 二维码
}

// 条码配置类
class BarcodeConfig {
public:
    BarcodeConfig() :
        id(0),
        config_type(0),
        barcode_type(0),
        barcode_width(200),
        barcode_height(50),
        qrcode_size(100),
        is_active(true) {}

    // Getters
    int getId() const { return id; }
    int getConfigType() const { return config_type; }  // 0=打印, 1=刻录
    int getBarcodeType() const { return barcode_type; } // 0=条形码, 1=二维码
    int getBarcodeWidth() const { return barcode_width; }
    int getBarcodeHeight() const { return barcode_height; }
    int getQrcodeSize() const { return qrcode_size; }
    QString getCustomText() const { return custom_text; }
    QList<int> getSelectedFields() const { return selected_fields; }
    bool getIsActive() const { return is_active; }
    QDateTime getCreatedAt() const { return created_at; }
    QDateTime getUpdatedAt() const { return updated_at; }

    // Setters
    void setId(int value) { id = value; }
    void setConfigType(int value) { config_type = value; }
    void setBarcodeType(int value) { barcode_type = value; }
    void setBarcodeWidth(int value) { barcode_width = value; }
    void setBarcodeHeight(int value) { barcode_height = value; }
    void setQrcodeSize(int value) { qrcode_size = value; }
    void setCustomText(const QString& value) { custom_text = value; }
    void setSelectedFields(const QList<int>& value) { selected_fields = value; }
    void setIsActive(bool value) { is_active = value; }
    void setCreatedAt(const QDateTime& value) { created_at = value; }
    void setUpdatedAt(const QDateTime& value) { updated_at = value; }

    // Helper methods
    QString getConfigTypeName() const {
        switch (config_type) {
            case 0: return "打印";
            case 1: return "刻录";
            default: return "未知";
        }
    }

    QString getBarcodeTypeName() const {
        switch (barcode_type) {
            case BarcodeType::BARCODE: return "条形码";
            case BarcodeType::QRCODE: return "二维码";
            default: return "未知";
        }
    }

    QString getFieldName(int field) const {
        switch (field) {
            case BarcodeField::SERIAL_NUMBER: return "流水号";
            case BarcodeField::CUSTOM_TEXT: return "自定义文本";
            case BarcodeField::DEPARTMENT: return "提交人部门";
            case BarcodeField::SUBMITTER: return "提交人";
            case BarcodeField::SECURITY_LEVEL: return "密级";
            case BarcodeField::COPIES: return "份数";
            case BarcodeField::FILENAME: return "文件名";
            default: return "未知";
        }
    }

    // 检查字段是否已选择
    bool isFieldSelected(int field) const {
        return selected_fields.contains(field);
    }

    // 添加字段（检查最大数量限制：除流水号外最多4个）
    bool addField(int field) {
        if (field == BarcodeField::SERIAL_NUMBER) {
            // 流水号始终包含，无需添加
            return true;
        }
        if (selected_fields.contains(field)) {
            return true;  // 已存在
        }
        // 统计非流水号的字段数量
        int count = selected_fields.count();
        if (field != BarcodeField::SERIAL_NUMBER && count >= 4) {
            return false;  // 超过最大限制
        }
        selected_fields.append(field);
        return true;
    }

    // 移除字段
    void removeField(int field) {
        if (field == BarcodeField::SERIAL_NUMBER) {
            return;  // 流水号不可移除
        }
        selected_fields.removeAll(field);
    }

    // Legacy struct members for backward compatibility
    int id;
    int config_type;
    int barcode_type;
    int barcode_width;
    int barcode_height;
    int qrcode_size;
    QString custom_text;
    QList<int> selected_fields;
    bool is_active;
    QDateTime created_at;
    QDateTime updated_at;
};

#endif // BARCODE_CONFIG_MODEL_H