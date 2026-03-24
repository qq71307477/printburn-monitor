#include "PrintExecutor.h"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <mutex>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// 静态实例
static std::once_flag onceFlag;
static PrintExecutor* instance = nullptr;

PrintExecutor& PrintExecutor::getInstance()
{
    std::call_once(onceFlag, []() {
        instance = new PrintExecutor();
    });
    return *instance;
}

PrintExecutor::PrintExecutor()
    : m_printer(nullptr)
    , m_initialized(false)
{
    initializePrinter();
}

PrintExecutor::~PrintExecutor()
{
    QMutexLocker locker(&m_mutex);
    if (m_printer) {
        delete m_printer;
        m_printer = nullptr;
    }
}

void PrintExecutor::initializePrinter()
{
    QMutexLocker locker(&m_mutex);

    if (m_initialized) {
        return;
    }

    // 创建打印机对象，使用默认打印机
    m_printer = new QPrinter(QPrinter::HighResolution);
    m_printer->setPageSize(QPrinter::A4);

    // 检查是否有可用打印机
    QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
    if (printers.isEmpty()) {
        m_lastError = QObject::tr("未找到可用的打印机");
        qWarning() << "PrintExecutor: No available printers found";
    } else {
        // 获取默认打印机名称
        QPrinterInfo defaultPrinter = QPrinterInfo::defaultPrinter();
        if (!defaultPrinter.isNull()) {
            m_currentPrinterName = defaultPrinter.printerName();
        }
    }

    m_initialized = true;
    qDebug() << "PrintExecutor: Initialized with" << printers.size() << "printer(s)";
}

bool PrintExecutor::printFile(const QString& filePath, int copies)
{
    QMutexLocker locker(&m_mutex);

    m_lastError.clear();

    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_lastError = QObject::tr("文件不存在: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 检查文件是否可读
    if (!fileInfo.isReadable()) {
        m_lastError = QObject::tr("文件不可读: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 根据文件类型选择打印方法
    QString fileType = getFileType(filePath).toLower();

    bool success = false;
    if (fileType == "pdf") {
        success = printPdf(filePath, copies);
    } else if (fileType == "png" || fileType == "jpg" || fileType == "jpeg" ||
               fileType == "bmp" || fileType == "gif" || fileType == "tiff") {
        success = printImage(filePath, copies);
    } else if (fileType == "txt" || fileType == "text" || fileType == "log") {
        success = printText(filePath, copies);
    } else {
        m_lastError = QObject::tr("不支持的文件类型: %1").arg(fileType);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 记录打印日志
    logPrintOperation(filePath, copies, success);

    return success;
}

bool PrintExecutor::printPdf(const QString& filePath, int copies)
{
    // 注意：此方法已在调用处加锁

    m_lastError.clear();

    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_lastError = QObject::tr("PDF文件不存在: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 设置打印份数
    setCopies(copies);

    // 设置输出格式为打印机
    m_printer->setOutputFormat(QPrinter::NativeFormat);
    m_printer->setOutputFileName(QString());  // 清空输出文件名，使用打印机

    // Qt 5.12 中打印 PDF 需要使用 QPainter 绘制
    // 由于 QPdfDocument 在 Qt 5.12 中可能不可用，我们使用 QPrinter 的 PDF 打印功能
    // 这里使用一种简化的方法：将 PDF 文件作为原始数据发送到打印机

#ifdef Q_OS_WIN
    // Windows 平台：使用 ShellExecute 打印
    // 这种方法依赖于系统默认的 PDF 查看器
    QString nativeFilePath = QDir::toNativeSeparators(filePath);
    HINSTANCE result = ShellExecuteW(nullptr, L"print", nativeFilePath.toStdWString().c_str(),
                                     nullptr, nullptr, SW_HIDE);
    if (reinterpret_cast<qintptr>(result) <= 32) {
        m_lastError = QObject::tr("Windows 打印失败，错误代码: %1").arg(reinterpret_cast<qintptr>(result));
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }
    return true;
#elif defined(Q_OS_LINUX)
    // Linux 平台：尝试使用 lpr 命令打印
    QString command = QString("lpr -P \"%1\" -# %2 \"%3\"")
                          .arg(m_currentPrinterName)
                          .arg(copies)
                          .arg(filePath);

    int result = system(command.toUtf8().constData());
    if (result != 0) {
        m_lastError = QObject::tr("Linux 打印命令执行失败，返回码: %1").arg(result);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }
    return true;
#else
    // 其他平台：使用 QPainter 尝试绘制
    // 这种方法有限，只能打印第一页
    QPainter painter(m_printer);
    if (!painter.isActive()) {
        m_lastError = QObject::tr("无法启动打印作业");
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 加载 PDF 作为图像（简化方案）
    QImage image(filePath);
    if (image.isNull()) {
        m_lastError = QObject::tr("无法加载PDF文件: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 缩放图像以适应页面
    QRect pageRect = m_printer->pageRect();
    QSize scaledSize = image.size();
    scaledSize.scale(pageRect.size(), Qt::KeepAspectRatio);

    QRect targetRect;
    targetRect.setSize(scaledSize);
    targetRect.moveCenter(pageRect.center());

    painter.drawImage(targetRect, image);
    painter.end();

    return true;
#endif
}

bool PrintExecutor::printImage(const QString& filePath, int copies)
{
    // 注意：此方法在 printFile 中已加锁，但为了独立调用安全，需要检查

    m_lastError.clear();

    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_lastError = QObject::tr("图片文件不存在: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 加载图片
    QImage image(filePath);
    if (image.isNull()) {
        m_lastError = QObject::tr("无法加载图片文件: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 设置打印份数
    setCopies(copies);

    // 设置输出格式为打印机
    m_printer->setOutputFormat(QPrinter::NativeFormat);
    m_printer->setOutputFileName(QString());

    // 创建绘图器
    QPainter painter(m_printer);
    if (!painter.isActive()) {
        m_lastError = QObject::tr("无法启动打印作业");
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 计算缩放比例以适应页面
    QRect pageRect = m_printer->pageRect();
    QSize scaledSize = image.size();
    scaledSize.scale(pageRect.size(), Qt::KeepAspectRatio);

    // 居中放置图片
    QRect targetRect;
    targetRect.setSize(scaledSize);
    targetRect.moveCenter(pageRect.center());

    // 绘制图片
    painter.drawImage(targetRect, image);
    painter.end();

    return true;
}

bool PrintExecutor::printText(const QString& filePath, int copies)
{
    m_lastError.clear();

    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_lastError = QObject::tr("文本文件不存在: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 读取文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QObject::tr("无法打开文件: %1").arg(filePath);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();

    // 设置打印份数
    setCopies(copies);

    // 设置输出格式为打印机
    m_printer->setOutputFormat(QPrinter::NativeFormat);
    m_printer->setOutputFileName(QString());

    // 创建绘图器
    QPainter painter(m_printer);
    if (!painter.isActive()) {
        m_lastError = QObject::tr("无法启动打印作业");
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    // 设置字体
    QFont font("SimSun", 10);  // 使用宋体，10磅
    painter.setFont(font);

    // 获取页面参数
    QRect pageRect = m_printer->pageRect();
    int lineHeight = painter.fontMetrics().height();
    int y = pageRect.top();

    // 按行分割内容
    QStringList lines = content.split('\n');
    for (const QString& line : lines) {
        if (y + lineHeight > pageRect.bottom()) {
            // 需要新页面
            m_printer->newPage();
            y = pageRect.top();
        }

        // 绘制文本行
        painter.drawText(pageRect.left(), y, line);
        y += lineHeight;
    }

    painter.end();
    return true;
}

QString PrintExecutor::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

QStringList PrintExecutor::getAvailablePrinters()
{
    QMutexLocker locker(&m_mutex);

    QStringList printerNames;
    QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();

    for (const QPrinterInfo& printer : printers) {
        printerNames.append(printer.printerName());
    }

    qDebug() << "PrintExecutor: Found" << printerNames.size() << "available printers";
    return printerNames;
}

bool PrintExecutor::selectPrinter(const QString& printerName)
{
    QMutexLocker locker(&m_mutex);

    m_lastError.clear();

    // 查找指定打印机
    QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
    bool found = false;

    for (const QPrinterInfo& printer : printers) {
        if (printer.printerName() == printerName) {
            // 设置打印机
            m_printer->setPrinterName(printerName);
            m_currentPrinterName = printerName;
            found = true;
            break;
        }
    }

    if (!found) {
        m_lastError = QObject::tr("未找到指定的打印机: %1").arg(printerName);
        qWarning() << "PrintExecutor:" << m_lastError;
        return false;
    }

    qDebug() << "PrintExecutor: Selected printer:" << printerName;
    return true;
}

QString PrintExecutor::getCurrentPrinterName() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentPrinterName;
}

bool PrintExecutor::isPrintable(const QString& filePath) const
{
    QString fileType = getFileType(filePath).toLower();

    // 支持的文件类型列表
    static const QStringList supportedTypes = {
        "pdf", "png", "jpg", "jpeg", "bmp", "gif", "tiff", "txt", "text", "log"
    };

    return supportedTypes.contains(fileType);
}

QStringList PrintExecutor::getSupportedFileTypes() const
{
    return QStringList()
           << "pdf"
           << "png"
           << "jpg"
           << "jpeg"
           << "bmp"
           << "gif"
           << "tiff"
           << "txt"
           << "text"
           << "log";
}

void PrintExecutor::setCopies(int copies)
{
    // 注意：此方法在调用处已加锁

    if (copies < 1) {
        copies = 1;
    }
    if (copies > 999) {
        copies = 999;  // 限制最大打印份数
    }

    m_printer->setCopyCount(copies);
    qDebug() << "PrintExecutor: Set copies to" << copies;
}

void PrintExecutor::logPrintOperation(const QString& filePath, int copies, bool success)
{
    // 记录打印操作日志
    QString logMessage = QString("[%1] Print operation: %2, copies: %3, result: %4")
                             .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                             .arg(filePath)
                             .arg(copies)
                             .arg(success ? "SUCCESS" : "FAILED");

    if (success) {
        qDebug() << "PrintExecutor:" << logMessage;
    } else {
        qWarning() << "PrintExecutor:" << logMessage << "Error:" << m_lastError;
    }

    // TODO: 集成到审计日志系统
    // 可以调用 LogAuditService 记录到数据库
}

QString PrintExecutor::getFileType(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower();
}