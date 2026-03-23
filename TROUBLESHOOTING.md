# CI 构建问题排查文档

## 项目信息

- **仓库**: https://github.com/qq71307477/printburn-monitor
- **Qt 版本**: 5.12.12
- **构建平台**: Ubuntu (gcc_64), Windows (win64_msvc2017_64), macOS-13 (clang_64)

---

## 当前状态

### CI 构建历史

| 提交 | Ubuntu | Windows | macOS | 说明 |
|------|--------|---------|-------|------|
| `f3d3b02` | ❌ failure | ❌ failure | ⏹ cancelled | TaskRepository 默认构造函数 |
| `3085571` | ❌ failure | ❌ failure | ⏹ cancelled | Repository 接口不匹配 |
| `6e33532` | ❌ failure | ❌ failure | ⏹ cancelled | Windows Qt 架构错误 |

### CI 链接

- Actions: https://github.com/qq71307477/printburn-monitor/actions
- 最新构建: https://github.com/qq71307477/printburn-monitor/actions/runs/23439376688

---

## 已修复问题

### 1. Windows Qt 架构名称错误 ✅

**问题**: Qt 5.12.12 没有 `win64_msvc2019_64` 架构

**修复**: `.github/workflows/build.yml`
```yaml
# 修改前
qt_arch: win64_msvc2019_64

# 修改后
qt_arch: win64_msvc2017_64
```

### 2. QHeaderView 缺失 ✅

**问题**: `PrintApplicationPage.cpp` 和 `BurnApplicationPage.cpp` 编译错误
```
error: invalid use of incomplete type 'class QHeaderView'
```

**修复**: 添加 `#include <QHeaderView>`

### 3. Model 类缺少 Qt 风格方法 ✅

**问题**: Services 代码使用 `user.getUsername()`, `device.getSerialNumber()` 等方法，但 Model 是简单 struct

**修复**: 为 `User`, `Device`, `Role` 类添加 getter/setter 方法

**修改文件**:
- `models/user_model.h`
- `models/device_model.h`
- `models/role_model.h`

### 4. Repository 缺少默认构造函数 ✅

**问题**: Services 使用 `UserRepository userRepo;` 无参构造，但 Repository 只有带参构造函数

**修复**: 添加默认构造函数和静态 `default_db_manager_`

**修改文件**:
- `src/common/repository/user_repository.h/cpp`
- `src/common/repository/device_repository.h/cpp`
- `src/common/repository/role_repository.h/cpp`
- `src/common/repository/task_repository.h/cpp`

---

## 可能仍存在的问题

### 1. DatabaseManager 未初始化 ⚠️

**位置**: `src/common/repository/*_repository.cpp`

**问题**: 默认构造函数使用 `nullptr` 作为 db_manager
```cpp
static DatabaseManager* default_db_manager_ = nullptr;

UserRepository::UserRepository() : db_manager_(nullptr) {}
```

**影响**: Services 调用 Repository 方法时，`db_manager_` 为空指针，可能导致崩溃

**建议修复**:
- 在 `main.cpp` 中初始化全局 DatabaseManager
- 或在 Repository 中延迟初始化

### 2. 潜在的编译问题 ⚠️

**位置**: 多个文件

**现象**: 本地 IDE 显示诊断错误（可能是 Qt 环境未配置）

**需要确认**:
- `models/task_model.h` 是否需要添加 QString include
- 插件目录 (`src/plugins/`) 是否有独立编译问题

### 3. 服务层代码不一致 ⚠️

**问题**: Services 期望的接口与 Repository 实现可能不完全匹配

**需要检查**:
- `src/services/AuthService.cpp`
- `src/services/UserManagementService.cpp`
- `src/services/DeviceManagementService.cpp`
- `src/services/RoleManagementService.cpp`
- `src/services/TaskService.cpp`

---

## 排查步骤

### 1. 查看 CI 日志

```bash
# 使用 GitHub CLI (需要安装 gh)
gh run view <run_id> --repo qq71307477/printburn-monitor

# 或直接访问网页
# https://github.com/qq71307477/printburn-monitor/actions
```

### 2. 本地构建测试

```bash
# 安装 Qt 5.12 后
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/5.12.12/<arch>
cmake --build . 2>&1 | head -100
```

### 3. 检查具体错误

```bash
# 搜索 error 关键字
cmake --build . 2>&1 | grep "error:"
```

---

## 代码结构

```
printburn-monitor-clean/
├── .github/workflows/build.yml  # CI 配置
├── CMakeLists.txt               # 主构建配置
├── models/                      # 数据模型
│   ├── user_model.h
│   ├── device_model.h
│   ├── role_model.h
│   └── task_model.h
├── src/
│   ├── main/main.cpp           # 程序入口
│   ├── ui/                     # UI 界面
│   ├── services/               # 业务逻辑
│   ├── common/
│   │   ├── PluginManager.*     # 插件管理
│   │   └── repository/         # 数据访问层
│   └── plugins/                # 插件目录
│       ├── client/
│       ├── task/
│       ├── approval/
│       └── security/
└── tests/                      # 测试代码 (已禁用)
```

---

## 快速修复命令

```bash
# 提交并推送
git add -A
git commit -m "fix: 描述修复内容"
git push origin main

# 查看 CI 状态
curl -s "https://api.github.com/repos/qq71307477/printburn-monitor/actions/runs?per_page=1" | jq '.workflow_runs[0] | {id: .id, status: .status, conclusion: .conclusion}'
```

---

## 更新日志

- **2026-03-23**: 创建文档，记录当前 CI 问题