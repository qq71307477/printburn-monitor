# 打印刻录安全监控系统（单机版）

## 项目概述

打印刻录安全监控系统是一款专为企业环境设计的安全监控软件，用于管理和监控打印和刻录操作，确保数据安全和合规性。系统采用插件化架构，支持任务申请、审批、设备管理等功能。

## 架构特点

- **插件化架构**：系统采用插件化设计，包含4个核心插件：
  - ClientPlugin：客户端界面插件
  - TaskPlugin：任务管理插件
  - ApprovalPlugin：审批流程插件
  - SecurityPlugin：安全策略插件

- **模块化服务**：系统包含多种业务逻辑服务：
  - AuthService：用户认证服务
  - TaskService：任务管理服务
  - SensitiveWordsService：敏感词检查服务
  - DeviceManagementService：设备管理服务
  - UserManagementService：用户管理服务
  - RoleManagementService：角色管理服务
  - LogAuditService：日志审计服务

- **数据库设计**：使用SQLite数据库存储用户、任务、设备等信息

## 功能模块

### 1. 用户认证
- 登录/登出
- 密码修改
- 权限验证

### 2. 任务管理
- 打印任务申请
- 刻录任务申请
- 任务审批流程
- 任务状态追踪

### 3. 设备管理
- 打印设备管理
- 刻录设备管理
- 设备分配与回收

### 4. 安全策略
- 敏感词检查
- 审批流程控制
- 操作审计日志

### 5. 系统管理
- 用户管理
- 角色管理
- 权限分配
- 组织架构管理

## 界面功能

- 登录界面
- 个人主页
- 打印申请
- 刻录申请
- 打印输出
- 刻录输出
- 任务审批

## 技术栈

- **编程语言**: C++ with Qt Framework
- **UI框架**: Qt Widgets
- **数据库**: SQLite
- **构建系统**: CMake
- **测试框架**: Qt Test

## 项目结构

```
├── src/                    # 源代码
│   ├── main/              # 主程序入口
│   ├── ui/                # 用户界面组件
│   ├── common/            # 公共组件（插件管理器等）
│   ├── plugins/           # 插件模块
│   └── services/          # 业务逻辑服务
├── models/                # 数据模型
├── include/               # 头文件
├── tests/                 # 测试代码
├── installer/             # 安装包配置
│   ├── windows/           # Windows安装包配置
│   ├── linux/             # Linux安装包脚本
│   └── debian/            # Debian包控制文件
├── .github/workflows/     # CI/CD配置
└── docs/                  # 文档
```

## 构建与运行

### 构建要求

- CMake 3.16+
- Qt 6.x
- C++17 编译器

### 构建步骤

```bash
# 克隆项目
git clone <repository-url>
cd <project-directory>

# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译项目
cmake --build . --config Release

# 运行程序
./PrintBurnMonitor
```

### 运行测试

```bash
# 运行单元测试
cd build
make test
# 或
ctest -V
```

## 部署

项目支持跨平台部署：

- **Windows**: NSIS安装包
- **Linux**: DEB/RPM包
- **macOS**: DMG安装包

CI/CD流程会自动构建各平台的安装包。

## 许可证

此项目仅供学习和参考使用。