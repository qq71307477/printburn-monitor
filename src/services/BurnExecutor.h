#ifndef BURNEXECUTOR_H
#define BURNEXECUTOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QTimer>
#include <QMutex>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

/**
 * @brief 刻录执行器服务类
 *
 * 提供跨平台的光盘刻录功能：
 * - Windows: 使用 IMAPI (Image Mastering API)
 * - Linux: 使用 cdrecord/wodim 或 growisofs 命令行工具
 */
class BurnExecutor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 介质类型枚举
     */
    enum class MediaType {
        CD_R,       // CD-R
        CD_RW,      // CD-RW
        DVD_R,      // DVD-R
        DVD_RW,     // DVD-RW
        DVD_RAM,    // DVD-RAM
        DVD_PLUS_R, // DVD+R
        DVD_PLUS_RW,// DVD+RW
        BD_R,       // BD-R (蓝光)
        BD_RE,      // BD-RE (蓝光可擦写)
        Unknown     // 未知类型
    };
    Q_ENUM(MediaType)

    /**
     * @brief 刻录状态枚举
     */
    enum class BurnStatus {
        Idle,           // 空闲
        Preparing,      // 准备中
        Burning,        // 刻录中
        Finalizing,     // 结束化中
        Completed,      // 已完成
        Failed,         // 失败
        Cancelled       // 已取消
    };
    Q_ENUM(BurnStatus)

    /**
     * @brief 获取单例实例
     */
    static BurnExecutor& getInstance();

    // 禁止拷贝和赋值
    BurnExecutor(const BurnExecutor&) = delete;
    BurnExecutor& operator=(const BurnExecutor&) = delete;

    /**
     * @brief 刻录文件列表到光盘
     * @param filePaths 要刻录的文件路径列表
     * @param volumeLabel 光盘卷标
     * @param mediaType 介质类型
     * @return 刻录是否成功启动
     */
    bool burnFiles(const QStringList& filePaths, const QString& volumeLabel = "DATA", MediaType mediaType = MediaType::CD_R);

    /**
     * @brief 刻录目录到光盘
     * @param directory 要刻录的目录路径
     * @param volumeLabel 光盘卷标
     * @param mediaType 介质类型
     * @return 刻录是否成功启动
     */
    bool burnDirectory(const QString& directory, const QString& volumeLabel = "DATA", MediaType mediaType = MediaType::CD_R);

    /**
     * @brief 检查是否有可用刻录介质
     * @param devicePath 设备路径（Linux）或驱动器号（Windows），为空则检测默认设备
     * @return 是否有可用介质
     */
    bool isMediaAvailable(const QString& devicePath = QString());

    /**
     * @brief 获取可用刻录机列表
     * @return 刻录机设备路径列表
     */
    QStringList getAvailableBurners();

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    QString getLastError() const;

    /**
     * @brief 获取刻录进度
     * @return 进度百分比 (0-100)
     */
    int getBurnProgress() const;

    /**
     * @brief 获取当前刻录状态
     * @return 刻录状态
     */
    BurnStatus getBurnStatus() const;

    /**
     * @brief 取消当前刻录任务
     * @return 是否成功取消
     */
    bool cancelBurn();

    /**
     * @brief 获取刻录机信息
     * @param devicePath 设备路径
     * @return 包含设备信息的 QVariantMap
     */
    QVariantMap getBurnerInfo(const QString& devicePath);

    /**
     * @brief 检测介质类型
     * @param devicePath 设备路径
     * @return 介质类型
     */
    MediaType detectMediaType(const QString& devicePath = QString());

    /**
     * @brief 弹出光盘
     * @param devicePath 设备路径
     * @return 是否成功
     */
    bool ejectDisc(const QString& devicePath = QString());

    /**
     * @brief 擦除可擦写介质
     * @param devicePath 设备路径
     * @param quick 是否快速擦除
     * @return 擦除是否成功启动
     */
    bool eraseDisc(const QString& devicePath = QString(), bool quick = true);

    /**
     * @brief 将介质类型转换为显示字符串
     * @param mediaType 介质类型
     * @return 显示字符串
     */
    static QString mediaTypeToString(MediaType mediaType);

    /**
     * @brief 将字符串转换为介质类型
     * @param mediaTypeStr 介质类型字符串
     * @return 介质类型
     */
    static MediaType stringToMediaType(const QString& mediaTypeStr);

signals:
    /**
     * @brief 刻录进度变化信号
     * @param progress 进度百分比 (0-100)
     */
    void progressChanged(int progress);

    /**
     * @brief 刻录状态变化信号
     * @param status 新状态
     */
    void statusChanged(BurnStatus status);

    /**
     * @brief 刻录完成信号
     * @param success 是否成功
     * @param message 结果消息
     */
    void burnCompleted(bool success, const QString& message);

    /**
     * @brief 刻录错误信号
     * @param error 错误消息
     */
    void burnError(const QString& error);

    /**
     * @brief 日志消息信号
     * @param message 日志消息
     * @param level 日志级别 (info, warning, error)
     */
    void logMessage(const QString& message, const QString& level = "info");

private slots:
    /**
     * @brief 处理进程输出
     */
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void updateProgress();

private:
    BurnExecutor();
    ~BurnExecutor();

    /**
     * @brief 初始化平台相关的刻录组件
     */
    void initializePlatformSpecific();

    /**
     * @brief Windows 平台：使用 IMAPI 刻录
     */
#ifdef Q_OS_WIN
    bool burnWithIMAPI(const QStringList& filePaths, const QString& volumeLabel, MediaType mediaType);
    bool detectMediaWithIMAPI(const QString& driveLetter);
    QStringList getBurnersWithIMAPI();
    HRESULT initializeIMAPI();
    void releaseIMAPI();
#endif

    /**
     * @brief Linux 平台：使用命令行工具刻录
     */
#ifdef Q_OS_LINUX
    bool burnWithCdrecord(const QStringList& filePaths, const QString& volumeLabel, MediaType mediaType);
    bool burnWithGrowisofs(const QStringList& filePaths, const QString& volumeLabel, MediaType mediaType);
    bool detectMediaLinux(const QString& devicePath);
    QStringList getBurnersLinux();
    QString findCdrecordPath();
    QString findGrowisofsPath();
    bool createISOImage(const QString& outputPath, const QStringList& filePaths, const QString& volumeLabel);
#endif

    /**
     * @brief 记录日志
     * @param message 日志消息
     * @param level 日志级别
     */
    void log(const QString& message, const QString& level = "info");

    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const QString& error);

    /**
     * @brief 设置刻录状态
     * @param status 新状态
     */
    void setBurnStatus(BurnStatus status);

    /**
     * @brief 验证文件路径
     * @param filePaths 文件路径列表
     * @return 是否全部有效
     */
    bool validateFilePaths(const QStringList& filePaths);

    /**
     * @brief 验证目录路径
     * @param directory 目录路径
     * @return 是否有效
     */
    bool validateDirectory(const QString& directory);

    /**
     * @brief 计算文件/目录总大小
     * @param paths 路径列表
     * @return 总大小（字节）
     */
    qint64 calculateTotalSize(const QStringList& paths);

    /**
     * @brief 获取介质容量
     * @param mediaType 介质类型
     * @return 容量（字节）
     */
    qint64 getMediaCapacity(MediaType mediaType);

    // 成员变量
    mutable QMutex m_mutex;
    QString m_lastError;
    BurnStatus m_status;
    int m_progress;
    QString m_currentDevice;
    QStringList m_currentFiles;
    QProcess* m_currentProcess;
    QTimer* m_progressTimer;
    MediaType m_currentMediaType;
    bool m_isCancelled;

#ifdef Q_OS_WIN
    // Windows IMAPI 相关句柄
    void* m_imapiDiscMaster;    // IDiscMaster*
    void* m_imapiDiscRecorder;  // IDiscRecorder*
    void* m_imapiJolietDiscMaster; // IJolietDiscMaster*
#endif
};

#endif // BURNEXECUTOR_H