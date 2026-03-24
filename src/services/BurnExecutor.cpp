#include "BurnExecutor.h"
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDateTime>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QVariantMap>
#include <mutex>

#ifdef Q_OS_LINUX
#include <mntent.h>
#include <sys/statvfs.h>
#include <unistd.h>
#endif

// 静态实例
static std::once_flag onceFlag;
static BurnExecutor* instance = nullptr;

BurnExecutor& BurnExecutor::getInstance()
{
    std::call_once(onceFlag, []() {
        instance = new BurnExecutor();
    });
    return *instance;
}

BurnExecutor::BurnExecutor()
    : m_status(BurnStatus::Idle)
    , m_progress(0)
    , m_currentProcess(nullptr)
    , m_progressTimer(new QTimer(this))
    , m_currentMediaType(MediaType::Unknown)
    , m_isCancelled(false)
#ifdef Q_OS_WIN
    , m_imapiDiscMaster(nullptr)
    , m_imapiDiscRecorder(nullptr)
    , m_imapiJolietDiscMaster(nullptr)
#endif
{
    initializePlatformSpecific();
    connect(m_progressTimer, &QTimer::timeout, this, &BurnExecutor::updateProgress);
}

BurnExecutor::~BurnExecutor()
{
    cancelBurn();
#ifdef Q_OS_WIN
    releaseIMAPI();
#endif
    delete m_progressTimer;
}

void BurnExecutor::initializePlatformSpecific()
{
#ifdef Q_OS_WIN
    initializeIMAPI();
#endif
    log("BurnExecutor initialized", "info");
}

// ==================== 公共方法 ====================

bool BurnExecutor::burnFiles(const QStringList& filePaths, const QString& volumeLabel, MediaType mediaType)
{
    QMutexLocker locker(&m_mutex);

    if (m_status == BurnStatus::Burning || m_status == BurnStatus::Preparing) {
        setLastError(tr("已有刻录任务正在执行"));
        return false;
    }

    if (!validateFilePaths(filePaths)) {
        setLastError(tr("文件路径验证失败"));
        return false;
    }

    // 检查介质
    if (!isMediaAvailable()) {
        setLastError(tr("没有检测到可用的刻录介质"));
        return false;
    }

    // 检查容量
    qint64 totalSize = calculateTotalSize(filePaths);
    qint64 capacity = getMediaCapacity(mediaType);
    if (totalSize > capacity) {
        setLastError(tr("文件总大小 (%1 MB) 超过介质容量 (%2 MB)")
                     .arg(totalSize / (1024 * 1024))
                     .arg(capacity / (1024 * 1024)));
        return false;
    }

    m_currentFiles = filePaths;
    m_currentMediaType = mediaType;
    m_isCancelled = false;
    setBurnStatus(BurnStatus::Preparing);

    log(QString("开始刻录文件，共 %1 个文件，总大小: %2 MB")
        .arg(filePaths.size())
        .arg(totalSize / (1024 * 1024)));

#ifdef Q_OS_WIN
    return burnWithIMAPI(filePaths, volumeLabel, mediaType);
#elif defined(Q_OS_LINUX)
    // 根据介质类型选择刻录工具
    if (mediaType == MediaType::DVD_R || mediaType == MediaType::DVD_RW ||
        mediaType == MediaType::DVD_PLUS_R || mediaType == MediaType::DVD_PLUS_RW ||
        mediaType == MediaType::BD_R || mediaType == MediaType::BD_RE) {
        return burnWithGrowisofs(filePaths, volumeLabel, mediaType);
    } else {
        return burnWithCdrecord(filePaths, volumeLabel, mediaType);
    }
#else
    setLastError(tr("不支持的操作系统"));
    setBurnStatus(BurnStatus::Failed);
    return false;
#endif
}

bool BurnExecutor::burnDirectory(const QString& directory, const QString& volumeLabel, MediaType mediaType)
{
    if (!validateDirectory(directory)) {
        setLastError(tr("目录路径无效或不存在"));
        return false;
    }

    // 收集目录中的所有文件
    QStringList filePaths;
    QDir dir(directory);
    QDirIterator it(directory, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        filePaths.append(it.filePath());
    }

    if (filePaths.isEmpty()) {
        setLastError(tr("目录为空或无法读取"));
        return false;
    }

    log(QString("刻录目录: %1，包含 %2 个文件").arg(directory).arg(filePaths.size()));
    return burnFiles(filePaths, volumeLabel, mediaType);
}

bool BurnExecutor::isMediaAvailable(const QString& devicePath)
{
#ifdef Q_OS_WIN
    Q_UNUSED(devicePath);
    // Windows: 使用 IMAPI 检测
    QStringList burners = getAvailableBurners();
    return !burners.isEmpty();
#elif defined(Q_OS_LINUX)
    return detectMediaLinux(devicePath);
#else
    return false;
#endif
}

QStringList BurnExecutor::getAvailableBurners()
{
#ifdef Q_OS_WIN
    return getBurnersWithIMAPI();
#elif defined(Q_OS_LINUX)
    return getBurnersLinux();
#else
    return QStringList();
#endif
}

QString BurnExecutor::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

int BurnExecutor::getBurnProgress() const
{
    QMutexLocker locker(&m_mutex);
    return m_progress;
}

BurnExecutor::BurnStatus BurnExecutor::getBurnStatus() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

bool BurnExecutor::cancelBurn()
{
    QMutexLocker locker(&m_mutex);

    if (m_status != BurnStatus::Burning && m_status != BurnStatus::Preparing) {
        return false;
    }

    m_isCancelled = true;

    if (m_currentProcess && m_currentProcess->state() != QProcess::NotRunning) {
        log("正在取消刻录任务...", "warning");
        m_currentProcess->terminate();
        if (!m_currentProcess->waitForFinished(3000)) {
            m_currentProcess->kill();
        }
    }

    setBurnStatus(BurnStatus::Cancelled);
    log("刻录任务已取消", "warning");
    return true;
}

QVariantMap BurnExecutor::getBurnerInfo(const QString& devicePath)
{
    QVariantMap info;
    info["devicePath"] = devicePath;

#ifdef Q_OS_LINUX
    // 使用 cdrecord -atip 获取设备信息
    QString cdrecord = findCdrecordPath();
    if (!cdrecord.isEmpty()) {
        QProcess process;
        process.start(cdrecord, QStringList() << "-atip" << QString("dev=%1").arg(devicePath));
        if (process.waitForFinished(10000)) {
            QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
            // 解析输出获取设备信息
            if (output.contains("CD-R") || output.contains("CD-RW")) {
                info["mediaType"] = "CD";
            } else if (output.contains("DVD") || output.contains("DVD-R") || output.contains("DVD+R")) {
                info["mediaType"] = "DVD";
            } else if (output.contains("BD") || output.contains("BLURAY")) {
                info["mediaType"] = "Blu-ray";
            }
            info["info"] = output;
        }
    }
#elif defined(Q_OS_WIN)
    // Windows 下使用 IMAPI 获取信息
    info["devicePath"] = devicePath;
    info["mediaType"] = "Unknown";
#endif

    return info;
}

BurnExecutor::MediaType BurnExecutor::detectMediaType(const QString& devicePath)
{
#ifdef Q_OS_LINUX
    QString cdrecord = findCdrecordPath();
    if (!cdrecord.isEmpty()) {
        QProcess process;
        QString dev = devicePath.isEmpty() ? "/dev/sr0" : devicePath;
        process.start(cdrecord, QStringList() << "-atip" << QString("dev=%1").arg(dev));
        if (process.waitForFinished(10000)) {
            QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).toUpper();

            if (output.contains("CD-RW")) return MediaType::CD_RW;
            if (output.contains("CD-R")) return MediaType::CD_R;
            if (output.contains("DVD+RW")) return MediaType::DVD_PLUS_RW;
            if (output.contains("DVD+R")) return MediaType::DVD_PLUS_R;
            if (output.contains("DVD-RW")) return MediaType::DVD_RW;
            if (output.contains("DVD-RAM")) return MediaType::DVD_RAM;
            if (output.contains("DVD-R")) return MediaType::DVD_R;
            if (output.contains("BD-RE")) return MediaType::BD_RE;
            if (output.contains("BD-R")) return MediaType::BD_R;
        }
    }
#endif
    return MediaType::Unknown;
}

bool BurnExecutor::ejectDisc(const QString& devicePath)
{
#ifdef Q_OS_LINUX
    QString device = devicePath.isEmpty() ? "/dev/sr0" : devicePath;
    QProcess process;
    process.start("eject", QStringList() << device);
    bool result = process.waitForFinished(5000);
    if (result && process.exitCode() == 0) {
        log("光盘已弹出", "info");
        return true;
    } else {
        // 尝试使用 cdrecord
        QString cdrecord = findCdrecordPath();
        if (!cdrecord.isEmpty()) {
            process.start(cdrecord, QStringList() << "-eject" << QString("dev=%1").arg(device));
            result = process.waitForFinished(5000);
            if (result && process.exitCode() == 0) {
                log("光盘已弹出 (使用 cdrecord)", "info");
                return true;
            }
        }
    }
#elif defined(Q_OS_WIN)
    Q_UNUSED(devicePath);
    // Windows 使用 IMAPI 弹出
    // TODO: 实现 IMAPI 弹出
    m_lastError = "弹出功能尚未实现";
    return false;
#endif

    setLastError(tr("弹出光盘失败"));
    return false;
}

bool BurnExecutor::eraseDisc(const QString& devicePath, bool quick)
{
    QMutexLocker locker(&m_mutex);

    if (m_status == BurnStatus::Burning || m_status == BurnStatus::Preparing) {
        setLastError(tr("已有刻录任务正在执行"));
        return false;
    }

#ifdef Q_OS_LINUX
    QString device = devicePath.isEmpty() ? "/dev/sr0" : devicePath;
    QString cdrecord = findCdrecordPath();

    if (cdrecord.isEmpty()) {
        setLastError(tr("未找到 cdrecord/wodim 工具"));
        return false;
    }

    QStringList args;
    args << "dev=" + device;
    args << "blank=" + (quick ? QString("fast") : QString("all"));

    m_currentProcess = new QProcess(this);
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BurnExecutor::onProcessFinished);
    connect(m_currentProcess, &QProcess::readyReadStandardOutput,
            this, &BurnExecutor::onProcessReadyRead);
    connect(m_currentProcess, &QProcess::errorOccurred,
            this, &BurnExecutor::onProcessError);

    log(QString("开始%1擦除光盘: %2").arg(quick ? "快速" : "完全").arg(device), "info");

    m_currentProcess->start(cdrecord, args);
    m_status = BurnStatus::Burning;
    return true;
#elif defined(Q_OS_WIN)
    Q_UNUSED(devicePath);
    Q_UNUSED(quick);
    setLastError(tr("Windows 平台擦除功能尚未实现"));
    return false;
#else
    return false;
#endif
}

QString BurnExecutor::mediaTypeToString(MediaType mediaType)
{
    switch (mediaType) {
    case MediaType::CD_R:       return "CD-R";
    case MediaType::CD_RW:      return "CD-RW";
    case MediaType::DVD_R:      return "DVD-R";
    case MediaType::DVD_RW:     return "DVD-RW";
    case MediaType::DVD_RAM:    return "DVD-RAM";
    case MediaType::DVD_PLUS_R: return "DVD+R";
    case MediaType::DVD_PLUS_RW:return "DVD+RW";
    case MediaType::BD_R:       return "BD-R";
    case MediaType::BD_RE:      return "BD-RE";
    default:                    return "Unknown";
    }
}

BurnExecutor::MediaType BurnExecutor::stringToMediaType(const QString& mediaTypeStr)
{
    QString upper = mediaTypeStr.toUpper();
    if (upper == "CD-R")       return MediaType::CD_R;
    if (upper == "CD-RW")      return MediaType::CD_RW;
    if (upper == "DVD-R")      return MediaType::DVD_R;
    if (upper == "DVD-RW")     return MediaType::DVD_RW;
    if (upper == "DVD-RAM")    return MediaType::DVD_RAM;
    if (upper == "DVD+R")      return MediaType::DVD_PLUS_R;
    if (upper == "DVD+RW")     return MediaType::DVD_PLUS_RW;
    if (upper == "BD-R")       return MediaType::BD_R;
    if (upper == "BD-RE")      return MediaType::BD_RE;
    return MediaType::Unknown;
}

// ==================== 私有方法 ====================

void BurnExecutor::log(const QString& message, const QString& level)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logEntry = QString("[%1] [%2] %3").arg(timestamp).arg(level.toUpper()).arg(message);
    qDebug() << logEntry;
    emit logMessage(message, level);
}

void BurnExecutor::setLastError(const QString& error)
{
    m_lastError = error;
    if (!error.isEmpty()) {
        log(error, "error");
        emit burnError(error);
    }
}

void BurnExecutor::setBurnStatus(BurnStatus status)
{
    m_status = status;
    emit statusChanged(status);
}

bool BurnExecutor::validateFilePaths(const QStringList& filePaths)
{
    for (const QString& path : filePaths) {
        QFileInfo info(path);
        if (!info.exists()) {
            setLastError(tr("文件不存在: %1").arg(path));
            return false;
        }
        if (!info.isReadable()) {
            setLastError(tr("文件不可读: %1").arg(path));
            return false;
        }
    }
    return true;
}

bool BurnExecutor::validateDirectory(const QString& directory)
{
    QDir dir(directory);
    return dir.exists() && dir.isReadable();
}

qint64 BurnExecutor::calculateTotalSize(const QStringList& paths)
{
    qint64 totalSize = 0;
    for (const QString& path : paths) {
        QFileInfo info(path);
        if (info.isFile()) {
            totalSize += info.size();
        } else if (info.isDir()) {
            QDir dir(path);
            QDirIterator it(path, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                totalSize += it.fileInfo().size();
            }
        }
    }
    return totalSize;
}

qint64 BurnExecutor::getMediaCapacity(MediaType mediaType)
{
    // 返回介质容量（字节）
    switch (mediaType) {
    case MediaType::CD_R:
    case MediaType::CD_RW:
        return 700 * 1024 * 1024;  // 700 MB
    case MediaType::DVD_R:
    case MediaType::DVD_RW:
    case MediaType::DVD_PLUS_R:
    case MediaType::DVD_PLUS_RW:
        return 4700 * 1024 * 1024; // 4.7 GB
    case MediaType::DVD_RAM:
        return 4700 * 1024 * 1024; // 4.7 GB (single sided)
    case MediaType::BD_R:
    case MediaType::BD_RE:
        return 25000 * 1024 * 1024; // 25 GB (single layer)
    default:
        return 700 * 1024 * 1024;  // 默认 CD 容量
    }
}

void BurnExecutor::updateProgress()
{
    if (m_status != BurnStatus::Burning) {
        m_progressTimer->stop();
        return;
    }

    // 进度更新由平台相关代码处理
    emit progressChanged(m_progress);
}

// ==================== 进程处理槽函数 ====================

void BurnExecutor::onProcessReadyRead()
{
    if (!m_currentProcess) return;

    QString output = QString::fromLocal8Bit(m_currentProcess->readAllStandardOutput());
    QString errorOutput = QString::fromLocal8Bit(m_currentProcess->readAllStandardError());

    // 解析刻录进度
    // cdrecord/wodim 输出格式类似: "Track 01:  45 MB written (fifo 100%) [fifo 100%]"
    QRegularExpression progressRx("(\\d+)\\s*(MB|KB)\\s*written");
    QRegularExpressionMatch match = progressRx.match(output);
    if (match.hasMatch()) {
        log(QString("刻录进度: %1").arg(output.trimmed()));
        // 这里可以根据总大小计算实际进度
    }

    // 检查错误
    if (!errorOutput.isEmpty()) {
        log(errorOutput, "warning");
    }
}

void BurnExecutor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_progressTimer->stop();

    if (m_isCancelled) {
        setBurnStatus(BurnStatus::Cancelled);
        emit burnCompleted(false, tr("刻录已取消"));
        return;
    }

    if (exitStatus == QProcess::CrashExit) {
        setLastError(tr("刻录进程崩溃"));
        setBurnStatus(BurnStatus::Failed);
        emit burnCompleted(false, m_lastError);
        return;
    }

    if (exitCode != 0) {
        QString errorMsg = tr("刻录失败，退出码: %1").arg(exitCode);
        if (m_currentProcess) {
            errorMsg += "\n" + QString::fromLocal8Bit(m_currentProcess->readAllStandardError());
        }
        setLastError(errorMsg);
        setBurnStatus(BurnStatus::Failed);
        emit burnCompleted(false, errorMsg);
        return;
    }

    setBurnStatus(BurnStatus::Completed);
    m_progress = 100;
    emit progressChanged(100);
    log("刻录完成", "info");
    emit burnCompleted(true, tr("刻录成功完成"));
}

void BurnExecutor::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = tr("无法启动刻录进程");
        break;
    case QProcess::Crashed:
        errorMsg = tr("刻录进程崩溃");
        break;
    case QProcess::Timedout:
        errorMsg = tr("刻录进程超时");
        break;
    case QProcess::WriteError:
        errorMsg = tr("写入刻录进程失败");
        break;
    case QProcess::ReadError:
        errorMsg = tr("读取刻录进程输出失败");
        break;
    default:
        errorMsg = tr("未知错误");
    }

    setLastError(errorMsg);
    setBurnStatus(BurnStatus::Failed);
    emit burnCompleted(false, errorMsg);
}

// ==================== Windows 平台实现 ====================

#ifdef Q_OS_WIN

bool BurnExecutor::burnWithIMAPI(const QStringList& filePaths, const QString& volumeLabel, MediaType mediaType)
{
    Q_UNUSED(filePaths);
    Q_UNUSED(volumeLabel);
    Q_UNUSED(mediaType);

    // IMAPI 实现需要复杂的 COM 接口调用
    // 这里提供一个基本框架

    setLastError(tr("Windows IMAPI 刻录功能需要完整实现"));
    log("Windows IMAPI 刻录尚未完全实现", "warning");

    // 基本步骤:
    // 1. 初始化 COM
    // 2. 创建 IDiscMaster 对象
    // 3. 枚举并选择 IDiscRecorder
    // 4. 创建 IJolietDiscMaster 接口
    // 5. 添加文件到映像
    // 6. 调用 RecordDisc 方法进行刻录

    // 由于 IMAPI 实现复杂，这里提供替代方案：使用外部工具
    // 可以使用 cdrtools 的 Windows 版本 (cdrecord.exe)

    setBurnStatus(BurnStatus::Failed);
    return false;
}

bool BurnExecutor::detectMediaWithIMAPI(const QString& driveLetter)
{
    Q_UNUSED(driveLetter);

    // TODO: 使用 IMAPI 检测介质
    return false;
}

QStringList BurnExecutor::getBurnersWithIMAPI()
{
    QStringList burners;

    // 使用简单的驱动器枚举来检测光驱
    QStringList drives = QDir::drives();
    for (const QString& drive : drives) {
        QString path = QDir::toNativeSeparators(drive);
        // 简单检查是否是光驱（检查介质类型）
        if (GetDriveTypeW((LPCWSTR)path.utf16()) == DRIVE_CDROM) {
            burners.append(path);
        }
    }

    return burners;
}

HRESULT BurnExecutor::initializeIMAPI()
{
    // 初始化 COM 库
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        log("COM 初始化失败", "error");
        return hr;
    }

    // 创建 IDiscMaster 对象
    // hr = CoCreateInstance(CLSID_MSDiscMasterObj, NULL, CLSCTX_ALL,
    //                       IID_IDiscMaster, (void**)&m_imapiDiscMaster);

    return S_OK;
}

void BurnExecutor::releaseIMAPI()
{
    // 释放 IMAPI 接口
    if (m_imapiJolietDiscMaster) {
        // ((IJolietDiscMaster*)m_imapiJolietDiscMaster)->Release();
        m_imapiJolietDiscMaster = nullptr;
    }
    if (m_imapiDiscRecorder) {
        // ((IDiscRecorder*)m_imapiDiscRecorder)->Release();
        m_imapiDiscRecorder = nullptr;
    }
    if (m_imapiDiscMaster) {
        // ((IDiscMaster*)m_imapiDiscMaster)->Release();
        m_imapiDiscMaster = nullptr;
    }

    CoUninitialize();
}

#endif // Q_OS_WIN

// ==================== Linux 平台实现 ====================

#ifdef Q_OS_LINUX

QString BurnExecutor::findCdrecordPath()
{
    // 优先使用 wodim (cdrecord 的开源替代)
    QStringList paths = {"/usr/bin/wodim", "/usr/bin/cdrecord",
                         "/usr/local/bin/wodim", "/usr/local/bin/cdrecord"};

    for (const QString& path : paths) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    // 尝试从 PATH 中查找
    QProcess process;
    process.start("which", QStringList() << "wodim");
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        return QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
    }

    process.start("which", QStringList() << "cdrecord");
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        return QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
    }

    return QString();
}

QString BurnExecutor::findGrowisofsPath()
{
    QStringList paths = {"/usr/bin/growisofs", "/usr/local/bin/growisofs"};

    for (const QString& path : paths) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    QProcess process;
    process.start("which", QStringList() << "growisofs");
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        return QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
    }

    return QString();
}

bool BurnExecutor::detectMediaLinux(const QString& devicePath)
{
    QString device = devicePath.isEmpty() ? "/dev/sr0" : devicePath;

    // 检查设备是否存在
    if (!QFile::exists(device)) {
        return false;
    }

    // 尝试打开设备来检测介质
    QFile devFile(device);
    if (devFile.open(QIODevice::ReadOnly)) {
        devFile.close();
        return true;
    }

    // 使用 cdrecord 检测
    QString cdrecord = findCdrecordPath();
    if (!cdrecord.isEmpty()) {
        QProcess process;
        process.start(cdrecord, QStringList() << "-atip" << QString("dev=%1").arg(device));
        if (process.waitForFinished(10000)) {
            return process.exitCode() == 0;
        }
    }

    return false;
}

QStringList BurnExecutor::getBurnersLinux()
{
    QStringList burners;

    // 扫描 /dev 目录查找光驱设备
    QDir devDir("/dev");
    QStringList filters;
    filters << "sr*" << "scd*" << "cdrom*";
    devDir.setNameFilters(filters);

    QStringList entries = devDir.entryList(QDir::System | QDir::Readable);
    for (const QString& entry : entries) {
        QString fullPath = "/dev/" + entry;

        // 检查是否是符号链接，如果是则获取真实路径
        QFileInfo info(fullPath);
        if (info.isSymLink()) {
            fullPath = info.symLinkTarget();
        }

        // 避免重复
        if (!burners.contains(fullPath)) {
            burners.append(fullPath);
        }
    }

    // 如果没有找到，添加默认设备
    if (burners.isEmpty() && QFile::exists("/dev/sr0")) {
        burners.append("/dev/sr0");
    }

    // 使用 cdrecord -scanbus 获取更详细的设备列表
    QString cdrecord = findCdrecordPath();
    if (!cdrecord.isEmpty()) {
        QProcess process;
        process.start(cdrecord, QStringList() << "-scanbus");
        if (process.waitForFinished(10000)) {
            QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
            // 解析输出查找 CD/DVD 刻录机
            QRegularExpression rx("(\\d+,\\d+,\\d+)\\s+\\d+\\)\\s+.*'(CD|DVD|BD).*'");
            QRegularExpressionMatchIterator it = rx.globalMatch(output);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QString busId = match.captured(1);
                // 可以使用 busId 作为 cdrecord 的 dev 参数
            }
        }
    }

    return burners;
}

bool BurnExecutor::createISOImage(const QString& outputPath, const QStringList& filePaths, const QString& volumeLabel)
{
    QString mkisofsPath;

    // 查找 mkisofs 或 genisoimage
    QStringList mkisofsPaths = {"/usr/bin/genisoimage", "/usr/bin/mkisofs",
                                "/usr/local/bin/genisoimage", "/usr/local/bin/mkisofs"};

    for (const QString& path : mkisofsPaths) {
        if (QFile::exists(path)) {
            mkisofsPath = path;
            break;
        }
    }

    // 使用 which 查找
    if (mkisofsPath.isEmpty()) {
        QProcess process;
        process.start("which", QStringList() << "genisoimage");
        if (process.waitForFinished(3000) && process.exitCode() == 0) {
            mkisofsPath = QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
        }
    }

    if (mkisofsPath.isEmpty()) {
        QProcess process;
        process.start("which", QStringList() << "mkisofs");
        if (process.waitForFinished(3000) && process.exitCode() == 0) {
            mkisofsPath = QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
        }
    }

    if (mkisofsPath.isEmpty()) {
        setLastError(tr("未找到 genisoimage/mkisofs 工具"));
        return false;
    }

    // 构建参数
    QStringList args;
    args << "-J" << "-R";  // Joliet 和 Rock Ridge 扩展
    args << "-V" << volumeLabel;  // 卷标
    args << "-o" << outputPath;   // 输出文件

    // 添加文件路径
    for (const QString& path : filePaths) {
        args << path;
    }

    log(QString("创建 ISO 映像: %1").arg(outputPath));

    QProcess process;
    process.start(mkisofsPath, args);
    if (!process.waitForFinished(300000)) {  // 5 分钟超时
        setLastError(tr("创建 ISO 映像超时"));
        return false;
    }

    if (process.exitCode() != 0) {
        setLastError(tr("创建 ISO 映像失败: %1")
                     .arg(QString::fromLocal8Bit(process.readAllStandardError())));
        return false;
    }

    return true;
}

bool BurnExecutor::burnWithCdrecord(const QStringList& filePaths, const QString& volumeLabel, MediaType mediaType)
{
    QString cdrecord = findCdrecordPath();
    if (cdrecord.isEmpty()) {
        setLastError(tr("未找到 cdrecord/wodim 工具"));
        setBurnStatus(BurnStatus::Failed);
        return false;
    }

    // 获取刻录设备
    QStringList burners = getAvailableBurners();
    if (burners.isEmpty()) {
        setLastError(tr("未找到可用的刻录设备"));
        setBurnStatus(BurnStatus::Failed);
        return false;
    }

    m_currentDevice = burners.first();

    // 创建临时 ISO 文件
    QString tempIso = QString("/tmp/burn_%1.iso")
                       .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));

    if (!createISOImage(tempIso, filePaths, volumeLabel)) {
        setBurnStatus(BurnStatus::Failed);
        return false;
    }

    // 构建刻录参数
    QStringList args;
    args << QString("dev=%1").arg(m_currentDevice);

    // 根据介质类型设置参数
    switch (mediaType) {
    case MediaType::CD_R:
        args << "-dao" << "-data";  // Disk-at-once 模式
        break;
    case MediaType::CD_RW:
        args << "-dao" << "-data";
        break;
    default:
        args << "-dao";
    }

    // 速度设置（自动检测）
    args << "-eject";  // 完成后弹出

    // 添加 ISO 文件
    args << tempIso;

    log(QString("使用 cdrecord 刻录到设备: %1").arg(m_currentDevice));
    setBurnStatus(BurnStatus::Burning);
    m_progress = 0;
    m_progressTimer->start(1000);

    // 启动刻录进程
    m_currentProcess = new QProcess(this);
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BurnExecutor::onProcessFinished);
    connect(m_currentProcess, &QProcess::readyReadStandardOutput,
            this, &BurnExecutor::onProcessReadyRead);
    connect(m_currentProcess, &QProcess::readyReadStandardError,
            this, &BurnExecutor::onProcessReadyRead);
    connect(m_currentProcess, &QProcess::errorOccurred,
            this, &BurnExecutor::onProcessError);

    m_currentProcess->start(cdrecord, args);

    return true;
}

bool BurnExecutor::burnWithGrowisofs(const QStringList& filePaths, const QString& volumeLabel, MediaType mediaType)
{
    QString growisofs = findGrowisofsPath();
    if (growisofs.isEmpty()) {
        setLastError(tr("未找到 growisofs 工具"));
        setBurnStatus(BurnStatus::Failed);
        return false;
    }

    // 获取刻录设备
    QStringList burners = getAvailableBurners();
    if (burners.isEmpty()) {
        setLastError(tr("未找到可用的刻录设备"));
        setBurnStatus(BurnStatus::Failed);
        return false;
    }

    m_currentDevice = burners.first();

    // 构建刻录参数
    QStringList args;
    args << QString("-Z/%1").arg(m_currentDevice);  // 初始会话

    // 卷标
    args << "-V" << volumeLabel;

    // ISO 扩展
    args << "-J" << "-R";

    // 根据介质类型设置参数
    if (mediaType == MediaType::DVD_RW || mediaType == MediaType::DVD_PLUS_RW ||
        mediaType == MediaType::BD_RE) {
        // 可擦写介质，使用 -Z 覆盖写入
    }

    // 添加文件路径
    for (const QString& path : filePaths) {
        args << path;
    }

    log(QString("使用 growisofs 刻录到设备: %1").arg(m_currentDevice));
    setBurnStatus(BurnStatus::Burning);
    m_progress = 0;
    m_progressTimer->start(1000);

    // 启动刻录进程
    m_currentProcess = new QProcess(this);
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BurnExecutor::onProcessFinished);
    connect(m_currentProcess, &QProcess::readyReadStandardOutput,
            this, &BurnExecutor::onProcessReadyRead);
    connect(m_currentProcess, &QProcess::readyReadStandardError,
            this, &BurnExecutor::onProcessReadyRead);
    connect(m_currentProcess, &QProcess::errorOccurred,
            this, &BurnExecutor::onProcessError);

    m_currentProcess->start(growisofs, args);

    return true;
}

#endif // Q_OS_LINUX