#ifndef PRINTEXECUTOR_H
#define PRINTEXECUTOR_H

#include <QString>
#include <QStringList>
#include <QPrinter>
#include <QPrinterInfo>
#include <QMutex>

/**
 * @brief 打印执行服务类
 *
 * 实现跨平台的打印功能，支持打印 PDF、图片等文件类型。
 * 使用 Qt 的 QPrinter 和 QPrintDialog 实现打印功能。
 */
class PrintExecutor
{
public:
    // 单例获取方法
    static PrintExecutor& getInstance();

    /**
     * @brief 打印指定文件
     * @param filePath 文件路径
     * @param copies 打印份数，默认为1
     * @return 打印成功返回 true，失败返回 false
     */
    bool printFile(const QString& filePath, int copies = 1);

    /**
     * @brief 打印 PDF 文件
     * @param filePath PDF 文件路径
     * @param copies 打印份数，默认为1
     * @return 打印成功返回 true，失败返回 false
     */
    bool printPdf(const QString& filePath, int copies = 1);

    /**
     * @brief 打印图片文件
     * @param filePath 图片文件路径
     * @param copies 打印份数，默认为1
     * @return 打印成功返回 true，失败返回 false
     */
    bool printImage(const QString& filePath, int copies = 1);

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    QString getLastError() const;

    /**
     * @brief 获取可用打印机列表
     * @return 打印机名称列表
     */
    QStringList getAvailablePrinters();

    /**
     * @brief 选择指定打印机
     * @param printerName 打印机名称
     * @return 选择成功返回 true，失败返回 false
     */
    bool selectPrinter(const QString& printerName);

    /**
     * @brief 获取当前选中的打印机名称
     * @return 当前打印机名称，如果使用默认打印机则返回空字符串
     */
    QString getCurrentPrinterName() const;

    /**
     * @brief 检查文件是否可打印
     * @param filePath 文件路径
     * @return 可打印返回 true，否则返回 false
     */
    bool isPrintable(const QString& filePath) const;

    /**
     * @brief 获取支持的文件类型
     * @return 支持的文件扩展名列表
     */
    QStringList getSupportedFileTypes() const;

private:
    PrintExecutor();
    ~PrintExecutor();
    PrintExecutor(const PrintExecutor&) = delete;
    PrintExecutor& operator=(const PrintExecutor&) = delete;

    // 初始化打印机
    void initializePrinter();

    // 设置打印份数
    void setCopies(int copies);

    // 记录打印日志
    void logPrintOperation(const QString& filePath, int copies, bool success);

    // 根据文件扩展名获取文件类型
    QString getFileType(const QString& filePath) const;

    // 打印文本文件
    bool printText(const QString& filePath, int copies);

private:
    QPrinter* m_printer;            // 打印机对象
    QString m_lastError;            // 最后的错误信息
    QString m_currentPrinterName;   // 当前选中的打印机名称
    mutable QMutex m_mutex;          // 线程安全互斥锁
    bool m_initialized;              // 是否已初始化
};

#endif // PRINTEXECUTOR_H