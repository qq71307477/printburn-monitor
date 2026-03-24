/**
 * @file main.cpp
 * @brief 打印刻录安全监控系统主入口
 *
 * 系统启动流程：
 * 1. 初始化 QApplication
 * 2. 初始化数据库连接
 * 3. 显示登录窗口
 * 4. 登录成功后显示主窗口
 * 5. 延迟加载插件（后台加载）
 */

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <memory>

#include "ui/MainWindow.h"
#include "ui/LoginWindow.h"
#include "services/AuthService.h"
#include "common/repository/database_manager.h"
#include "common/PluginManager.h"

// 数据库文件名
static const QString DB_NAME = "printburn_monitor.db";

/**
 * @brief 获取数据库文件路径
 *
 * 优先使用应用数据目录，确保数据持久化
 */
QString getDatabasePath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);

    // 确保目录存在
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return dataPath + "/" + DB_NAME;
}

/**
 * @brief 初始化数据库
 * @return 数据库管理器实例，失败返回 nullptr
 */
std::unique_ptr<DatabaseManager> initializeDatabase()
{
    QString dbPath = getDatabasePath();
    auto dbManager = std::make_unique<DatabaseManager>(dbPath);

    if (!dbManager->initialize()) {
        QMessageBox::critical(nullptr, "数据库错误",
            "无法初始化数据库。\n请检查文件权限或磁盘空间。");
        return nullptr;
    }

    return dbManager;
}

/**
 * @brief 后台加载插件（不阻塞UI）
 * @param pluginManager 插件管理器
 */
void loadPluginsAsync(PluginManager *pluginManager)
{
    if (!pluginManager) {
        return;
    }

    // 使用 QtConcurrent 或 QTimer 延迟加载插件
    QTimer::singleShot(100, [pluginManager]() {
        // 获取插件目录路径
        QString pluginsPath = QCoreApplication::applicationDirPath() + "/plugins";
        QDir pluginsDir(pluginsPath);

        if (!pluginsDir.exists()) {
            qWarning("Plugins directory not found: %s", qPrintable(pluginsPath));
            return;
        }

        // 遍历插件目录，加载所有插件
        QStringList pluginFiles = pluginsDir.entryList(QDir::Files);
        for (const QString &fileName : pluginFiles) {
            QString pluginPath = pluginsDir.absoluteFilePath(fileName);

            // 根据平台过滤插件文件
#ifdef Q_OS_WIN
            if (!fileName.endsWith(".dll", Qt::CaseInsensitive)) continue;
#else
            if (!fileName.endsWith(".so") && !fileName.endsWith(".dylib")) continue;
#endif

            if (pluginManager->loadPlugin(pluginPath)) {
                qDebug("Loaded plugin: %s", qPrintable(fileName));
            } else {
                qWarning("Failed to load plugin: %s", qPrintable(fileName));
            }
        }

        // 初始化所有已加载的插件
        pluginManager->initializeAllPlugins();
    });
}

int main(int argc, char *argv[])
{
    // 创建应用程序实例
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("PrintBurnMonitor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PrintBurn");

    // 1. 初始化数据库
    auto dbManager = initializeDatabase();
    if (!dbManager) {
        return 1;
    }

    // 2. 显示登录窗口
    LoginWindow loginWindow;
    int loginResult = loginWindow.exec();

    if (loginResult != QDialog::Accepted) {
        // 用户取消登录或关闭窗口
        return 0;
    }

    // 3. 创建并显示主窗口
    MainWindow mainWindow;

    // 获取当前登录用户信息
    User currentUser = AuthService::getInstance().getCurrentUser();
    QString welcomeMessage = QString("欢迎, %1!")
        .arg(currentUser.getUsername().isEmpty() ? "用户" : currentUser.getUsername());

    mainWindow.show();
    mainWindow.statusBar()->showMessage(welcomeMessage, 5000);

    // 4. 延迟加载插件（不阻塞UI显示）
    loadPluginsAsync(&PluginManager::getInstance());

    // 5. 进入事件循环
    int result = app.exec();

    // 6. 清理资源
    AuthService::getInstance().logout();

    return result;
}