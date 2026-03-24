#!/usr/bin/env powershell
# ============================================
# PrintBurn Monitor Windows Installer Builder
# PowerShell Version
# ============================================

param(
    [string]$QtDir = "C:\Qt\5.15.2\msvc2019_64",
    [string]$NsisPath = "C:\Program Files (x86)\NSIS",
    [string]$Version = "1.0.0",
    [switch]$Clean,
    [switch]$Help
)

# 配置
$ErrorActionPreference = "Stop"
$ProjectName = "PrintBurnMonitor"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$BuildDir = Join-Path $ProjectDir "build-release"
$DeployDir = Join-Path $ProjectDir "deploy\windows"
$OutputDir = Join-Path $ProjectDir "dist"

# 解析版本号
$VersionParts = $Version.Split(".")
$VersionMajor = if ($VersionParts.Length -gt 0) { $VersionParts[0] } else { "1" }
$VersionMinor = if ($VersionParts.Length -gt 1) { $VersionParts[1] } else { "0" }
$VersionBuild = if ($VersionParts.Length -gt 2) { $VersionParts[2] } else { "0" }

function Show-Help {
    Write-Host @"
用法: .\build_installer.ps1 [选项]

选项:
    -QtDir PATH        Qt安装目录 (默认: C:\Qt\5.15.2\msvc2019_64)
    -NsisPath PATH     NSIS安装目录 (默认: C:\Program Files (x86)\NSIS)
    -Version VERSION   版本号 (默认: 1.0.0)
    -Clean             清理后重新构建
    -Help              显示帮助信息

示例:
    .\build_installer.ps1 -QtDir "C:\Qt\5.15.2\msvc2019_64" -Clean
    .\build_installer.ps1 -Version 1.2.3
"@
}

if ($Help) {
    Show-Help
    exit 0
}

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "PrintBurn Monitor Installer Builder (PowerShell)" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# 显示配置信息
Write-Host "[配置信息]" -ForegroundColor Yellow
Write-Host "项目目录: $ProjectDir"
Write-Host "构建目录: $BuildDir"
Write-Host "Qt目录: $QtDir"
Write-Host "NSIS目录: $NsisPath"
Write-Host "版本: $Version"
Write-Host ""

# 检查必要工具
Write-Host "[检查环境]" -ForegroundColor Yellow

if (-not (Test-Path $QtDir)) {
    Write-Error "错误: Qt目录不存在: $QtDir"
    Write-Host "请使用 -QtDir 参数指定正确的Qt安装路径"
    exit 1
}

$MakeNsis = Join-Path $NsisPath "makensis.exe"
if (-not (Test-Path $MakeNsis)) {
    Write-Error "错误: NSIS未找到: $NsisPath"
    Write-Host "请使用 -NsisPath 参数指定正确的NSIS安装路径"
    exit 1
}

Write-Host "环境检查通过" -ForegroundColor Green
Write-Host ""

# 清理构建目录
if ($Clean) {
    Write-Host "[清理构建目录]" -ForegroundColor Yellow
    if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
    if (Test-Path $OutputDir) { Remove-Item -Recurse -Force $OutputDir }
    Write-Host "清理完成"
    Write-Host ""
}

# 创建目录
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

# 步骤1: CMake配置
Write-Host "[步骤1/4] CMake配置" -ForegroundColor Yellow
Push-Location $BuildDir

$Qt5Dir = Join-Path $QtDir "lib\cmake\Qt5"
cmake -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX="$BuildDir\install" `
    -DQt5_DIR="$Qt5Dir" `
    "$ProjectDir"

if ($LASTEXITCODE -ne 0) {
    Write-Error "错误: CMake配置失败"
    Pop-Location
    exit 1
}
Write-Host "CMake配置完成" -ForegroundColor Green
Write-Host ""

# 步骤2: 编译项目
Write-Host "[步骤2/4] 编译项目" -ForegroundColor Yellow
cmake --build . --config Release --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Error "错误: 编译失败"
    Pop-Location
    exit 1
}
Write-Host "编译完成" -ForegroundColor Green
Write-Host ""

# 步骤3: 安装项目文件
Write-Host "[步骤3/4] 安装项目文件" -ForegroundColor Yellow
cmake --install . --config Release

# 使用windeployqt部署Qt依赖
Write-Host "部署Qt依赖..."
$QtBin = Join-Path $QtDir "bin"
$Env:Path = "$QtBin;$Env:Path"

$InstallDir = Join-Path $BuildDir "install"
$ExePath = Join-Path $InstallDir "$ProjectName.exe"

& "$QtBin\windeployqt.exe" `
    --release `
    --no-translations `
    --no-system-d3d-compiler `
    --no-opengl-sw `
    --no-angle `
    $ExePath

# 复制SQLite驱动
$SqlDriversDir = Join-Path $InstallDir "sqldrivers"
New-Item -ItemType Directory -Force -Path $SqlDriversDir | Out-Null
Copy-Item -Force (Join-Path $QtDir "plugins\sqldrivers\qsqlite.dll") $SqlDriversDir

# 创建资源目录
$ResourcesDir = Join-Path $InstallDir "resources"
$DataDir = Join-Path $InstallDir "data"
$LogsDir = Join-Path $InstallDir "logs"
New-Item -ItemType Directory -Force -Path $ResourcesDir | Out-Null
New-Item -ItemType Directory -Force -Path $DataDir | Out-Null
New-Item -ItemType Directory -Force -Path $LogsDir | Out-Null

# 复制资源文件
$ProjectResources = Join-Path $ProjectDir "resources"
if (Test-Path $ProjectResources) {
    Copy-Item -Recurse -Force "$ProjectResources\*" $ResourcesDir
}

# 创建README
$ReadmePath = Join-Path $InstallDir "README.txt"
@"
打印刻录安全监控系统 v$Version

安装目录: $Env:ProgramFiles\PrintBurn Security\$ProjectName
数据目录: [安装目录]\data
日志目录: [安装目录]\logs
"@ | Out-File -FilePath $ReadmePath -Encoding UTF8

# 复制LICENSE
$LicensePath = Join-Path $InstallDir "LICENSE.txt"
$ProjectLicense = Join-Path $ProjectDir "LICENSE"
if (Test-Path $ProjectLicense) {
    Copy-Item -Force $ProjectLicense $LicensePath
} else {
    "本软件受版权保护，未经授权不得复制或分发。" | Out-File -FilePath $LicensePath -Encoding UTF8
}

Write-Host "项目文件安装完成" -ForegroundColor Green
Write-Host ""

# 步骤4: 构建NSIS安装包
Write-Host "[步骤4/4] 构建NSIS安装包" -ForegroundColor Yellow

Push-Location $DeployDir

$NsisScript = Join-Path $DeployDir "installer.nsi"

& $MakeNsis `
    /DBUILD_DIR="$InstallDir" `
    /DQT_DIR="$QtDir" `
    /DRESOURCES_DIR="$ProjectResources" `
    /DVERSIONMAJOR=$VersionMajor `
    /DVERSIONMINOR=$VersionMinor `
    /DVERSIONBUILD=$VersionBuild `
    $NsisScript

if ($LASTEXITCODE -ne 0) {
    Write-Error "错误: NSIS编译失败"
    Pop-Location
    Pop-Location
    exit 1
}

# 移动安装包到输出目录
$SetupFile = Join-Path $DeployDir "PrintBurnMonitor-Setup-$Version.exe"
if (Test-Path $SetupFile) {
    Move-Item -Force $SetupFile $OutputDir
    Write-Host "安装包已生成: $OutputDir\PrintBurnMonitor-Setup-$Version.exe" -ForegroundColor Green
} else {
    $DefaultSetup = Join-Path $DeployDir "PrintBurnMonitor-Setup.exe"
    if (Test-Path $DefaultSetup) {
        Move-Item -Force $DefaultSetup (Join-Path $OutputDir "PrintBurnMonitor-Setup-$Version.exe")
        Write-Host "安装包已生成: $OutputDir\PrintBurnMonitor-Setup-$Version.exe" -ForegroundColor Green
    }
}

Pop-Location
Pop-Location

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "构建完成!" -ForegroundColor Green
Write-Host "安装包位置: $OutputDir\PrintBurnMonitor-Setup-$Version.exe"
Write-Host "============================================" -ForegroundColor Cyan