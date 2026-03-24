@echo off
REM ============================================
REM PrintBurn Monitor Windows Installer Builder
REM 打印刻录安全监控系统 - Windows安装包构建脚本
REM ============================================

setlocal EnableDelayedExpansion

REM 配置参数
set PROJECT_NAME=PrintBurnMonitor
set PROJECT_DIR=%~dp0..\..
set BUILD_DIR=%PROJECT_DIR%\build-release
set DEPLOY_DIR=%PROJECT_DIR%\deploy\windows
set OUTPUT_DIR=%PROJECT_DIR%\dist

REM Qt配置 (请根据实际安装路径修改)
set QT_DIR=C:\Qt\5.15.2\msvc2019_64
set QT_BIN=%QT_DIR%\bin
set CMAKE_GENERATOR="Visual Studio 17 2022"
set CMAKE_ARCH=x64

REM NSIS配置
set NSIS_PATH=C:\Program Files (x86)\NSIS
set MAKENSIS="%NSIS_PATH%\makensis.exe"

REM 版本号 (可通过参数覆盖)
set VERSION_MAJOR=1
set VERSION_MINOR=0
set VERSION_BUILD=0
set VERSION=%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_BUILD%

echo ============================================
echo PrintBurn Monitor Installer Builder
echo Version: %VERSION%
echo ============================================
echo.

REM 解析命令行参数
:parse_args
if "%1"=="" goto end_parse
if /i "%1"=="--qt-dir" set QT_DIR=%2& shift & shift & goto parse_args
if /i "%1"=="--nsis-path" set NSIS_PATH=%2& shift & shift & goto parse_args
if /i "%1"=="--version" set VERSION=%2& shift & shift & goto parse_args
if /i "%1"=="--clean" set CLEAN_BUILD=1& shift & goto parse_args
if /i "%1"=="--help" goto show_help
shift
goto parse_args
:end_parse

REM 更新Qt路径
set QT_BIN=%QT_DIR%\bin

echo [配置信息]
echo 项目目录: %PROJECT_DIR%
echo 构建目录: %BUILD_DIR%
echo Qt目录: %QT_DIR%
echo NSIS目录: %NSIS_PATH%
echo 版本: %VERSION%
echo.

REM 检查必要工具
echo [检查环境]
if not exist "%QT_DIR%" (
    echo 错误: Qt目录不存在: %QT_DIR%
    echo 请使用 --qt-dir 参数指定正确的Qt安装路径
    exit /b 1
)

if not exist "%NSIS_PATH%\makensis.exe" (
    echo 错误: NSIS未找到: %NSIS_PATH%
    echo 请使用 --nsis-path 参数指定正确的NSIS安装路径
    exit /b 1
)

echo 环境检查通过
echo.

REM 清理构建目录
if defined CLEAN_BUILD (
    echo [清理构建目录]
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    if exist "%OUTPUT_DIR%" rmdir /s /q "%OUTPUT_DIR%"
    echo 清理完成
    echo.
)

REM 创建构建目录
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM 步骤1: CMake配置
echo [步骤1/4] CMake配置
cd /d "%BUILD_DIR%"
cmake -G %CMAKE_GENERATOR% -A %CMAKE_ARCH% ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX="%BUILD_DIR%\install" ^
    -DQt5_DIR="%QT_DIR%\lib\cmake\Qt5" ^
    "%PROJECT_DIR%"

if errorlevel 1 (
    echo 错误: CMake配置失败
    exit /b 1
)
echo CMake配置完成
echo.

REM 步骤2: 编译项目
echo [步骤2/4] 编译项目
cmake --build . --config Release --parallel

if errorlevel 1 (
    echo 错误: 编译失败
    exit /b 1
)
echo 编译完成
echo.

REM 步骤3: 安装项目文件
echo [步骤3/4] 安装项目文件
cmake --install . --config Release

if errorlevel 1 (
    echo 警告: cmake install 未完全成功，尝试手动部署...
)

REM 使用windeployqt部署Qt依赖
echo 部署Qt依赖...
set PATH=%QT_BIN%;%PATH%

cd /d "%BUILD_DIR%\install"
"%QT_BIN%\windeployqt.exe" ^
    --release ^
    --no-translations ^
    --no-system-d3d-compiler ^
    --no-opengl-sw ^
    --no-angle ^
    "%BUILD_DIR%\install\%PROJECT_NAME%.exe"

if errorlevel 1 (
    echo 警告: windeployqt 执行有警告，继续...
)

REM 复制SQLite驱动
if not exist "%BUILD_DIR%\install\sqldrivers" mkdir "%BUILD_DIR%\install\sqldrivers"
copy "%QT_DIR%\plugins\sqldrivers\qsqlite.dll" "%BUILD_DIR%\install\sqldrivers\" >nul

REM 创建资源和数据目录
if not exist "%BUILD_DIR%\install\resources" mkdir "%BUILD_DIR%\install\resources"
if not exist "%BUILD_DIR%\install\data" mkdir "%BUILD_DIR%\install\data"
if not exist "%BUILD_DIR%\install\logs" mkdir "%BUILD_DIR%\install\logs"

REM 复制资源文件
if exist "%PROJECT_DIR%\resources" (
    xcopy "%PROJECT_DIR%\resources\*" "%BUILD_DIR%\install\resources\" /E /Y /Q
)

REM 创建README文件
echo 打印刻录安全监控系统 v%VERSION% > "%BUILD_DIR%\install\README.txt"
echo. >> "%BUILD_DIR%\install\README.txt"
echo 安装目录: %ProgramFiles%\PrintBurn Security\%PROJECT_NAME% >> "%BUILD_DIR%\install\README.txt"
echo 数据目录: [安装目录]\data >> "%BUILD_DIR%\install\README.txt"
echo 日志目录: [安装目录]\logs >> "%BUILD_DIR%\install\README.txt"

REM 复制LICENSE
if exist "%PROJECT_DIR%\LICENSE" (
    copy "%PROJECT_DIR%\LICENSE" "%BUILD_DIR%\install\LICENSE.txt" >nul
) else (
    echo 本软件受版权保护，未经授权不得复制或分发。 > "%BUILD_DIR%\install\LICENSE.txt"
)

echo 项目文件安装完成
echo.

REM 步骤4: 构建NSIS安装包
echo [步骤4/4] 构建NSIS安装包

REM 准备NSIS编译环境
cd /d "%DEPLOY_DIR%"

REM 创建临时配置文件
set NSIS_SCRIPT=temp_installer_%VERSION%.nsi

REM 复制模板并替换变量
copy installer.nsi %NSIS_SCRIPT% >nul

REM 使用NSIS命令行定义变量
%MAKENSIS% ^
    /DBUILD_DIR="%BUILD_DIR%\install" ^
    /DQT_DIR="%QT_DIR%" ^
    /DRESOURCES_DIR="%PROJECT_DIR%\resources" ^
    /DVERSIONMAJOR=%VERSION_MAJOR% ^
    /DVERSIONMINOR=%VERSION_MINOR% ^
    /DVERSIONBUILD=%VERSION_BUILD% ^
    %NSIS_SCRIPT%

if errorlevel 1 (
    echo 错误: NSIS编译失败
    del %NSIS_SCRIPT%
    exit /b 1
)

REM 清理临时文件
del %NSIS_SCRIPT%

REM 移动生成的安装包到输出目录
if exist "%DEPLOY_DIR%\PrintBurnMonitor-Setup-%VERSION%.exe" (
    move "%DEPLOY_DIR%\PrintBurnMonitor-Setup-%VERSION%.exe" "%OUTPUT_DIR%\" >nul
    echo 安装包已生成: %OUTPUT_DIR%\PrintBurnMonitor-Setup-%VERSION%.exe
) else if exist "%DEPLOY_DIR%\PrintBurnMonitor-Setup.exe" (
    move "%DEPLOY_DIR%\PrintBurnMonitor-Setup.exe" "%OUTPUT_DIR%\PrintBurnMonitor-Setup-%VERSION%.exe" >nul
    echo 安装包已生成: %OUTPUT_DIR%\PrintBurnMonitor-Setup-%VERSION%.exe
)

echo.
echo ============================================
echo 构建完成!
echo 安装包位置: %OUTPUT_DIR%\PrintBurnMonitor-Setup-%VERSION%.exe
echo ============================================

cd /d "%PROJECT_DIR%"
endlocal
exit /b 0

:show_help
echo 用法: %~nx0 [选项]
echo.
echo 选项:
echo   --qt-dir PATH      Qt安装目录 (默认: C:\Qt\5.15.2\msvc2019_64)
echo   --nsis-path PATH   NSIS安装目录 (默认: C:\Program Files (x86)\NSIS)
echo   --version VERSION  版本号 (默认: 1.0.0)
echo   --clean            清理后重新构建
echo   --help             显示帮助信息
echo.
echo 示例:
echo   %~nx0 --qt-dir "C:\Qt\5.15.2\msvc2019_64" --clean
echo   %~nx0 --version 1.2.3
exit /b 0