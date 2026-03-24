# 打印刻录安全监控系统（单机版）部署指南

**版本**：V1.0
**适用系统版本**：V5.3
**文档更新日期**：2026-03-24

---

## 目录

1. [环境要求](#1-环境要求)
2. [安装步骤](#2-安装步骤)
3. [配置说明](#3-配置说明)
4. [启动与停止](#4-启动与停止)
5. [数据备份与恢复](#5-数据备份与恢复)
6. [常见问题排查](#6-常见问题排查)
7. [技术支持](#7-技术支持)

---

## 1. 环境要求

### 1.1 硬件要求

| 配置项 | 最低要求 | 推荐配置 |
|--------|----------|----------|
| CPU | 双核 2.0GHz | 四核 2.5GHz 及以上 |
| 内存 | 4GB | 8GB 及以上 |
| 硬盘 | 10GB 可用空间 | 50GB 及以上（考虑数据增长） |
| 显示器 | 1366x768 分辨率 | 1920x1080 及以上 |
| 打印设备 | 支持标准打印驱动 | - |
| 刻录设备 | DVD/CD 刻录机 | - |

### 1.2 操作系统要求

| 操作系统 | 版本要求 | 架构 |
|----------|----------|------|
| Windows | Windows 10 1903+ / Windows 11 | x64 |
| 统信 UOS | V20+ | x64 / 龙芯 / 飞腾 |
| 麒麟 | V10+ | x64 / 龙芯 / 飞腾 |

### 1.3 软件依赖

| 依赖项 | 版本要求 | 说明 |
|--------|----------|------|
| Qt 运行时 | 6.5+ | 已包含在安装包中 |
| SQLite | 3.40+ | 已包含在安装包中 |
| OpenSSL | 1.1.1+ | 用于密码加密和授权验证 |
| Visual C++ 运行时 | 2019+ | Windows 平台需要 |
| libarchive | 3.5+ | 用于文件压缩存储（Linux） |

### 1.4 网络要求

- 本系统为单机版，无需网络连接即可正常运行
- 如需导入授权文件，需要可移动存储设备（U盘/光盘）

---

## 2. 安装步骤

### 2.1 Windows 安装

#### 2.1.1 安装前准备

1. 确认操作系统版本满足要求
2. 使用管理员账户登录系统
3. 关闭杀毒软件的实时防护（安装完成后恢复）
4. 准备安装包：`PrintBurnMonitor_Setup.exe`

#### 2.1.2 安装过程

1. **启动安装程序**
   - 双击 `PrintBurnMonitor_Setup.exe`
   - 如出现用户账户控制提示，点击"是"

2. **欢迎界面**
   - 阅读欢迎信息，点击"下一步"

3. **许可协议**
   - 阅读软件许可协议
   - 选择"我接受协议"，点击"下一步"

4. **选择组件**
   - 主要程序（必选）
   - 文档（可选）
   - 点击"下一步"

5. **选择安装位置**
   - 默认路径：`C:\Program Files\Security Solutions Inc.\打印刻录安全监控系统`
   - 可点击"浏览"自定义安装路径
   - 点击"下一步"

6. **选择开始菜单文件夹**
   - 默认创建"打印刻录安全监控系统"文件夹
   - 点击"下一步"

7. **开始安装**
   - 确认安装信息，点击"安装"
   - 等待安装进度完成

8. **安装完成**
   - 勾选"运行 打印刻录安全监控系统"
   - 点击"完成"

#### 2.1.3 安装后验证

安装完成后，系统会自动创建：
- 桌面快捷方式
- 开始菜单快捷方式
- 卸载程序入口（控制面板 → 程序和功能）

### 2.2 Linux 安装

#### 2.2.1 DEB 包安装（统信 UOS / Ubuntu / Debian）

```bash
# 安装依赖
sudo apt-get update
sudo apt-get install -y qt6-qmake qt6-base-dev-tools libqt6core6t64 libqt6gui6t64 \
    libqt6widgets6t64 libqt6sql6t64 libqt6network6t64 libsqlite3-0

# 安装软件包
sudo dpkg -i printburn-monitor_1.0.0_amd64.deb

# 如遇到依赖问题，执行以下命令修复
sudo apt-get install -f

# 更新桌面数据库
sudo update-desktop-database
```

#### 2.2.2 RPM 包安装（麒麟 / CentOS / openEuler）

```bash
# 安装依赖
sudo yum install -y qt6-qtbase-devel qt6-qtbase-private-devel sqlite

# 安装软件包
sudo rpm -ivh printburn-monitor-1.0.0-1.x86_64.rpm

# 更新桌面数据库
sudo update-desktop-database
```

#### 2.2.3 安装后验证

```bash
# 检查安装状态
dpkg -l | grep printburn-monitor    # DEB
rpm -qa | grep printburn-monitor    # RPM

# 检查可执行文件
which printburn-monitor
# 输出应为：/opt/usr/local/bin/printburn-monitor

# 检查桌面文件
ls /usr/share/applications/printburn-monitor.desktop
```

### 2.3 安装验证

安装完成后，请进行以下验证：

1. **程序启动验证**
   - Windows：双击桌面快捷方式
   - Linux：在应用菜单中找到"打印刻录安全监控系统"
   - 预期：显示登录界面

2. **数据库初始化验证**
   - 首次启动时，系统自动创建数据库文件
   - 数据库位置：
     - Windows：`%APPDATA%\PrintBurnMonitor\data\printburn.db`
     - Linux：`~/.local/share/PrintBurnMonitor/data/printburn.db`

3. **插件加载验证**
   - 启动程序后，检查日志输出
   - 确认 4 个插件加载成功：
     - ClientPlugin
     - TaskPlugin
     - ApprovalPlugin
     - SecurityPlugin

---

## 3. 配置说明

### 3.1 数据库配置

#### 3.1.1 数据库位置

系统使用 SQLite 嵌入式数据库，无需额外配置数据库服务。

| 平台 | 数据库路径 |
|------|------------|
| Windows | `%APPDATA%\PrintBurnMonitor\data\printburn.db` |
| Linux | `~/.local/share/PrintBurnMonitor/data/printburn.db` |

#### 3.1.2 数据库初始化

首次启动时，系统自动执行以下初始化操作：

1. 创建数据库文件
2. 创建数据表结构
3. 插入默认数据

如需手动初始化数据库：

```bash
# Linux
sqlite3 ~/.local/share/PrintBurnMonitor/data/printburn.db < /opt/printburn-monitor/scripts/init_db.sql

# Windows (使用 SQLite 命令行工具)
sqlite3 "%APPDATA%\PrintBurnMonitor\data\printburn.db" < "安装目录\scripts\init_db.sql"
```

#### 3.1.3 数据库配置文件

数据库连接配置存储在：

- Windows：`%APPDATA%\PrintBurnMonitor\config\database.ini`
- Linux：`~/.config/PrintBurnMonitor/database.ini`

配置文件格式：

```ini
[database]
driver=QSQLITE
name=printburn.db
path=
```

### 3.2 安全策略配置

安全策略用于控制系统的安全行为，由**安全保密员**进行配置。

#### 3.2.1 访问安全策略配置

1. 使用安全保密员账号登录系统
2. 进入【系统管理】→【安全策略】菜单

#### 3.2.2 配置项说明

| 配置项 | 默认值 | 可选值 | 说明 |
|--------|--------|--------|------|
| 最大登录尝试次数 | 5 | 1-5 | 连续失败后锁定账户 |
| 账户锁定时长 | 10分钟 | 10/15/30/60/120 分钟 | 锁定后解锁等待时间 |
| 密码最小长度 | 10 | 8-20 | 密码最少字符数 |
| 密码复杂度 | 中等 | 低/中/高 | 密码复杂度要求 |
| 密码有效期 | 7天 | 1-7 天 | 密码过期周期 |
| 会话超时时间 | 5分钟 | 5-15 分钟 | 无操作自动退出时间 |

#### 3.2.3 密码复杂度规则

| 级别 | 规则说明 |
|------|----------|
| 低 | 至少包含数字或字母 |
| 中 | 大写字母 + 小写字母 + 数字 |
| 高 | 大写字母 + 小写字母 + 数字 + 特殊字符 |

#### 3.2.4 配置示例

```json
{
  "maxLoginAttempts": 5,
  "lockDurationMinutes": 10,
  "passwordComplexity": {
    "requireUppercase": true,
    "requireLowercase": true,
    "requireDigit": true,
    "requireSpecial": false,
    "description": "大写字母 + 小写字母 + 数字"
  },
  "passwordMinLength": 10,
  "passwordExpiryDays": 7,
  "sessionTimeoutMinutes": 5
}
```

### 3.3 三员账号初始化

系统采用**三员分立**原则，默认创建三类管理员账号。

#### 3.3.1 三员账号说明

| 角色 | 用户名 | 默认密码 | 职责 |
|------|--------|----------|------|
| 系统管理员 | sysadmin | Admin@123 | 用户管理、角色管理、设备管理、基础配置 |
| 安全保密员 | secadmin | Secure@123 | 密级设置、安全策略、审批员配置、条码设置 |
| 安全审计员 | audadmin | Audit@123 | 日志查询、审计报告、无修改权限 |

#### 3.3.2 首次登录流程

1. **首次登录**
   - 使用任一三员账号登录
   - 系统提示修改初始密码

2. **修改密码**
   - 输入旧密码
   - 输入新密码（需符合安全策略要求）
   - 确认新密码
   - 点击"确定"保存

3. **创建普通用户**
   - 使用系统管理员账号登录
   - 进入【系统管理】→【用户管理】
   - 添加普通用户账号

#### 3.3.3 安全建议

1. **首次部署后立即修改三员账号密码**
2. **三员账号密码应复杂且互不相同**
3. **定期更换密码（建议每90天）**
4. **三员账号由不同人员分别管理**

#### 3.3.4 密码重置

如忘记管理员密码，可通过以下方式重置：

```bash
# Linux
cd ~/.local/share/PrintBurnMonitor/data
sqlite3 printburn.db "UPDATE users SET password_hash='新密码哈希值' WHERE username='sysadmin';"

# Windows
cd "%APPDATA%\PrintBurnMonitor\data"
sqlite3 printburn.db "UPDATE users SET password_hash='新密码哈希值' WHERE username='sysadmin';"
```

> **注意**：密码哈希值需要使用 bcrypt 算法生成。

---

## 4. 启动与停止

### 4.1 启动程序

#### 4.1.1 Windows 启动方式

**方式一：桌面快捷方式**
- 双击桌面"打印刻录安全监控系统"图标

**方式二：开始菜单**
- 点击【开始】→【所有程序】→【打印刻录安全监控系统】

**方式三：命令行启动**
```cmd
cd "C:\Program Files\Security Solutions Inc.\打印刻录安全监控系统"
PrintBurnMonitor.exe
```

#### 4.1.2 Linux 启动方式

**方式一：桌面菜单**
- 应用菜单 → 办公 → 打印刻录安全监控系统

**方式二：命令行启动**
```bash
/opt/usr/local/bin/printburn-monitor
```

**方式三：后台启动**
```bash
nohup /opt/usr/local/bin/printburn-monitor > /var/log/printburn.log 2>&1 &
```

### 4.2 停止程序

#### 4.2.1 正常退出

1. 点击主窗口右上角关闭按钮
2. 或使用菜单【文件】→【退出】
3. 系统提示确认退出，点击"确定"

#### 4.2.2 强制退出

**Windows：**
- 任务管理器 → 找到 `PrintBurnMonitor.exe` → 结束任务

**Linux：**
```bash
# 查找进程
ps aux | grep printburn-monitor

# 强制终止
kill -9 <PID>
```

### 4.3 开机自启动

#### 4.3.1 Windows 设置

**方式一：安装时自动配置**
- 安装程序会自动创建开机启动项

**方式二：手动配置**
1. 按 `Win + R`，输入 `shell:startup`
2. 将程序快捷方式复制到打开的文件夹

**方式三：注册表配置**
```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
新建字符串值：PrintBurnMonitor
值数据："C:\Program Files\...\PrintBurnMonitor.exe"
```

#### 4.3.2 Linux 设置

**systemd 服务方式（推荐）：**

创建服务文件 `/etc/systemd/system/printburn-monitor.service`：

```ini
[Unit]
Description=PrintBurn Monitor Service
After=network.target

[Service]
Type=simple
User=<用户名>
ExecStart=/opt/usr/local/bin/printburn-monitor
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

启用服务：

```bash
sudo systemctl daemon-reload
sudo systemctl enable printburn-monitor
sudo systemctl start printburn-monitor
```

**桌面自动启动方式：**

```bash
mkdir -p ~/.config/autostart
cat > ~/.config/autostart/printburn-monitor.desktop << EOF
[Desktop Entry]
Type=Application
Name=PrintBurn Monitor
Exec=/opt/usr/local/bin/printburn-monitor
Terminal=false
EOF
```

---

## 5. 数据备份与恢复

### 5.1 数据库备份

#### 5.1.1 手动备份

**Windows：**
```cmd
cd %APPDATA%\PrintBurnMonitor\data
copy printburn.db printburn_backup_%date:~0,10%.db
```

**Linux：**
```bash
cd ~/.local/share/PrintBurnMonitor/data
cp printburn.db printburn_backup_$(date +%Y%m%d).db
```

#### 5.1.2 定时备份脚本

**Windows（计划任务）：**

创建备份脚本 `backup_db.bat`：

```batch
@echo off
set BACKUP_DIR=D:\Backup\PrintBurnMonitor
set DATA_DIR=%APPDATA%\PrintBurnMonitor\data
set DATE=%date:~0,4%%date:~5,2%%date:~8,2%

if not exist %BACKUP_DIR% mkdir %BACKUP_DIR%
copy "%DATA_DIR%\printburn.db" "%BACKUP_DIR%\printburn_%DATE%.db"

:: 保留最近30天的备份
forfiles /p %BACKUP_DIR% /m *.db /d -30 /c "cmd /c del @path"
```

通过 Windows 任务计划程序设置每日执行。

**Linux（cron 定时任务）：**

创建备份脚本 `/opt/printburn-monitor/scripts/backup_db.sh`：

```bash
#!/bin/bash
BACKUP_DIR="/var/backup/printburn-monitor"
DATA_DIR="$HOME/.local/share/PrintBurnMonitor/data"
DATE=$(date +%Y%m%d)

mkdir -p $BACKUP_DIR
cp "$DATA_DIR/printburn.db" "$BACKUP_DIR/printburn_$DATE.db"

# 保留最近30天的备份
find $BACKUP_DIR -name "*.db" -mtime +30 -delete
```

设置 crontab：

```bash
# 每天凌晨2点执行备份
0 2 * * * /opt/printburn-monitor/scripts/backup_db.sh
```

#### 5.1.3 SQLite 在线备份

使用 SQLite 的 `.backup` 命令进行在线备份，不影响系统运行：

```bash
sqlite3 ~/.local/share/PrintBurnMonitor/data/printburn.db
sqlite> .backup backup_printburn.db
sqlite> .quit
```

### 5.2 数据恢复

#### 5.2.1 完整恢复

**步骤：**

1. 停止打印刻录安全监控系统

2. 备份当前数据库（如有必要）
   ```bash
   mv printburn.db printburn_corrupted.db
   ```

3. 恢复备份数据
   ```bash
   cp printburn_backup_20260324.db printburn.db
   ```

4. 启动系统并验证数据

#### 5.2.2 部分数据恢复

如只需恢复部分数据，可使用 SQL 导入导出：

```bash
# 从备份导出特定表
sqlite3 backup_printburn.db
sqlite> .output users_export.sql
sqlite> .dump users
sqlite> .quit

# 导入到当前数据库
sqlite3 printburn.db < users_export.sql
```

#### 5.2.3 数据完整性检查

恢复后执行完整性检查：

```bash
sqlite3 printburn.db "PRAGMA integrity_check;"
# 正常输出：ok

sqlite3 printburn.db "PRAGMA foreign_key_check;"
# 正常输出：空（无外键违规）
```

---

## 6. 常见问题排查

### 6.1 安装失败

#### 6.1.1 Windows 安装失败

| 错误现象 | 可能原因 | 解决方案 |
|----------|----------|----------|
| 权限不足 | 未以管理员身份运行 | 右键安装程序 → 以管理员身份运行 |
| 安装包损坏 | 下载不完整 | 重新下载安装包 |
| 杀毒软件拦截 | 安全软件拦截 | 临时禁用杀毒软件，安装完成后恢复 |
| 缺少 VC++ 运行时 | 系统缺少依赖 | 安装 Visual C++ 2019 Redistributable |
| 磁盘空间不足 | 存储空间不够 | 清理磁盘空间至10GB以上 |

**缺少 VC++ 运行时解决方案：**

从 Microsoft 官网下载安装：
- [Visual C++ 2015-2022 Redistributable (x64)](https://aka.ms/vs/17/release/vc_redist.x64.exe)

#### 6.1.2 Linux 安装失败

| 错误现象 | 可能原因 | 解决方案 |
|----------|----------|----------|
| 依赖缺失 | 系统缺少 Qt 库 | 执行 `sudo apt-get install -f` |
| 权限不足 | 未使用 sudo | 使用 sudo 执行安装命令 |
| 架构不匹配 | 软件包与系统架构不同 | 下载对应架构的安装包 |
| 包冲突 | 已存在旧版本 | 先卸载旧版本再安装 |

**手动安装依赖：**

```bash
# Ubuntu/Debian
sudo apt-get install -y qt6-base-dev libsqlite3-dev libssl-dev

# CentOS/RHEL/麒麟
sudo yum install -y qt6-qtbase-devel sqlite-devel openssl-devel
```

### 6.2 启动失败

#### 6.2.1 程序无法启动

| 错误现象 | 可能原因 | 解决方案 |
|----------|----------|----------|
| 双击无反应 | 插件加载失败 | 检查 plugins 目录是否完整 |
| 提示缺少 DLL/so | 运行库缺失 | 重新安装或修复依赖 |
| 数据库锁定 | 上次异常退出 | 删除 .lock 文件 |
| 配置文件损坏 | 配置文件格式错误 | 删除配置文件重新生成 |

**检查日志文件：**

- Windows：`%APPDATA%\PrintBurnMonitor\logs\printburn.log`
- Linux：`~/.local/share/PrintBurnMonitor/logs/printburn.log`

**插件加载问题排查：**

```bash
# Linux 检查插件文件
ls -la /opt/printburn-monitor/plugins/
# 应包含：libClientPlugin.so libTaskPlugin.so libApprovalPlugin.so libSecurityPlugin.so

# 检查插件依赖
ldd /opt/printburn-monitor/plugins/libClientPlugin.so
```

#### 6.2.2 登录失败

| 错误现象 | 可能原因 | 解决方案 |
|----------|----------|----------|
| 用户名或密码错误 | 输入错误 | 确认账号密码正确性 |
| 账户已锁定 | 登录失败次数过多 | 等待锁定时间后重试或联系管理员 |
| 数据库连接失败 | 数据库文件损坏 | 恢复数据库备份 |
| 用户不存在 | 用户未创建 | 使用管理员账号创建用户 |

### 6.3 数据库错误

#### 6.3.1 数据库文件损坏

**症状：**
- 启动时提示"数据库打开失败"
- 数据查询异常

**解决方案：**

1. 检查数据库完整性
   ```bash
   sqlite3 printburn.db "PRAGMA integrity_check;"
   ```

2. 如有错误，尝试修复
   ```bash
   # 导出数据
   sqlite3 printburn.db ".recover" > recover.sql

   # 创建新数据库
   rm printburn.db
   sqlite3 printburn.db < recover.sql
   ```

3. 如无法修复，使用备份恢复

#### 6.3.2 数据库性能问题

**症状：**
- 列表加载缓慢
- 查询超时

**解决方案：**

1. 执行 VACUUM 清理
   ```bash
   sqlite3 printburn.db "VACUUM;"
   ```

2. 重建索引
   ```bash
   sqlite3 printburn.db "REINDEX;"
   ```

3. 检查数据库大小
   ```bash
   ls -lh printburn.db
   ```
   如果超过 2GB，考虑归档历史数据或迁移到网络版。

#### 6.3.3 数据库锁定

**症状：**
- 操作时提示"database is locked"

**解决方案：**

1. 确认没有其他进程访问数据库
   ```bash
   # Linux
   lsof | grep printburn.db

   # Windows
   # 使用资源监视器查看文件句柄
   ```

2. 删除锁文件
   ```bash
   rm printburn.db-shm printburn.db-wal
   ```

3. 重启应用程序

---

## 7. 技术支持

### 7.1 获取帮助

如果您在使用过程中遇到问题，可通过以下方式获取帮助：

#### 7.1.1 日志收集

提交问题前，请收集以下信息：

1. **系统日志**
   - Windows：`%APPDATA%\PrintBurnMonitor\logs\`
   - Linux：`~/.local/share/PrintBurnMonitor/logs/`

2. **系统信息**
   ```bash
   # Linux
   uname -a
   lsb_release -a
   qt6-config --version

   # Windows
   systeminfo
   ```

3. **错误截图**
   - 包含完整的错误信息

#### 7.1.2 联系方式

| 支持类型 | 联系方式 |
|----------|----------|
| 技术支持邮箱 | support@example.com |
| 技术支持电话 | 400-XXX-XXXX |
| 工作时间 | 周一至周五 9:00-18:00 |

### 7.2 常用命令参考

#### 7.2.1 Windows 命令

```cmd
# 查看程序版本
PrintBurnMonitor.exe --version

# 指定配置文件路径启动
PrintBurnMonitor.exe --config "D:\config\app.ini"

# 查看帮助
PrintBurnMonitor.exe --help
```

#### 7.2.2 Linux 命令

```bash
# 查看程序版本
printburn-monitor --version

# 指定配置文件路径启动
printburn-monitor --config /path/to/config.ini

# 查看帮助
printburn-monitor --help

# 查看运行状态
systemctl status printburn-monitor

# 重启服务
sudo systemctl restart printburn-monitor
```

### 7.3 版本更新

#### 7.3.1 检查更新

系统内置更新检查功能：
1. 进入【帮助】→【检查更新】
2. 如有新版本，按提示下载更新

#### 7.3.2 手动更新

1. 备份数据库和配置文件
2. 下载新版本安装包
3. 运行安装程序覆盖安装
4. 启动程序验证数据完整性

> **重要**：更新前务必备份数据，以防数据丢失。

---

## 附录

### A. 文件目录结构

**Windows 安装目录：**
```
C:\Program Files\Security Solutions Inc.\打印刻录安全监控系统\
├── PrintBurnMonitor.exe          # 主程序
├── Qt6Core.dll                   # Qt 核心库
├── Qt6Gui.dll                    # Qt GUI 库
├── Qt6Widgets.dll                # Qt 界面库
├── Qt6Sql.dll                    # Qt SQL 库
├── Qt6Network.dll                # Qt 网络库
├── platforms\                    # 平台插件
│   └── qwindows.dll
├── styles\                       # 样式插件
│   └── qwindowsvistastyle.dll
├── plugins\                      # 业务插件
│   ├── ClientPlugin.dll
│   ├── TaskPlugin.dll
│   ├── ApprovalPlugin.dll
│   └── SecurityPlugin.dll
├── resources\                    # 资源文件
│   ├── icons\
│   └── qss\
├── scripts\                      # 脚本文件
│   └── init_db.sql
└── docs\                         # 文档文件
    ├── readme.txt
    ├── license.txt
    └── changelog.txt
```

**Windows 数据目录：**
```
%APPDATA%\PrintBurnMonitor\
├── data\
│   ├── printburn.db              # 数据库文件
│   ├── printburn.db-shm          # 共享内存文件
│   └── printburn.db-wal          # 预写日志文件
├── config\
│   ├── database.ini              # 数据库配置
│   └── app.ini                   # 应用配置
├── logs\
│   └── printburn.log             # 日志文件
└── backup\                       # 备份目录
```

**Linux 安装目录：**
```
/opt/printburn-monitor/
├── bin\
│   └── printburn-monitor         # 主程序
├── plugins\
│   ├── libClientPlugin.so
│   ├── libTaskPlugin.so
│   ├── libApprovalPlugin.so
│   └── libSecurityPlugin.so
├── resources\
│   ├── icons\
│   └── qss\
└── scripts\
    └── init_db.sql
```

### B. 端口和网络

单机版无需网络端口，系统完全在本地运行。

### C. 数据库表概览

| 表名 | 说明 |
|------|------|
| users | 用户信息表 |
| roles | 角色定义表 |
| user_roles | 用户角色关联表 |
| departments | 部门组织表 |
| security_levels | 密级定义表 |
| devices | 设备信息表 |
| print_tasks | 打印任务表 |
| burn_tasks | 刻录任务表 |
| burn_task_files | 刻录文件关联表 |
| login_logs | 登录日志表 |
| operation_logs | 操作日志表 |
| security_policies | 安全策略表 |
| approver_configs | 审批员配置表 |
| proxy_approvers | 代理审批表 |
| barcode_configs | 条码配置表 |
| serial_configs | 流水号配置表 |

---

**文档结束**

*本文档最后更新于 2026-03-24*