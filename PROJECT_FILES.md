# 项目文件结构说明

## 正式应用文件清单

### 1. 主程序入口
```
src/main/main.cpp                          # 程序入口
```

### 2. 核心框架 (src/common/)
```
src/common/PluginManager.h/cpp             # 插件管理器
```

### 3. 数据访问层 (src/common/repository/)
```
src/common/repository/database_manager.h/cpp   # 数据库管理
src/common/repository/user_repository.h/cpp    # 用户数据访问
src/common/repository/role_repository.h/cpp    # 角色数据访问
src/common/repository/department_repository.h/cpp  # 部门数据访问
src/common/repository/device_repository.h/cpp  # 设备数据访问
src/common/repository/task_repository.h/cpp    # 任务数据访问
src/common/repository/security_level_repository.h/cpp  # 密级数据访问
src/common/repository/serial_number_config_repository.h/cpp  # 流水号配置
src/common/repository/approver_config_repository.h/cpp  # 审批员配置
src/common/repository/proxy_approver_repository.h/cpp   # 代理审批员
```

### 4. 业务服务层 (src/services/)
```
src/services/AuthService.h/cpp             # 认证服务（含密码加密）
src/services/UserManagementService.h/cpp   # 用户管理服务
src/services/RoleManagementService.h/cpp   # 角色管理服务
src/services/TaskService.h/cpp             # 任务服务
src/services/DeviceManagementService.h/cpp # 设备管理服务
src/services/SecurityLevelService.h/cpp    # 密级服务
src/services/SerialNumberService.h/cpp     # 流水号服务
src/services/BarcodeService.h/cpp          # 条码服务
src/services/ApproverConfigService.h/cpp   # 审批员配置服务
src/services/ProxyApproverService.h/cpp    # 代理审批服务
src/services/PrintExecutor.h/cpp           # 打印执行器
src/services/BurnExecutor.h/cpp            # 刻录执行器
src/services/SensitiveWordsService.h/cpp   # 敏感词服务
src/services/LogAuditService.h/cpp         # 日志审计服务（含哈希链防篡改）
src/services/FileEncryptionService.h/cpp   # 文件加密服务
```

### 5. UI界面层 (src/ui/)
```
src/ui/LoginWindow.h/cpp                   # 登录窗口
src/ui/MainWindow.h/cpp                    # 主窗口
src/ui/PersonalHomePage.h/cpp              # 个人主页
src/ui/PrintApplicationPage.h/cpp          # 打印申请
src/ui/BurnApplicationPage.h/cpp           # 刻录申请
src/ui/PrintOutputPage.h/cpp               # 打印输出
src/ui/BurnOutputPage.h/cpp                # 刻录输出
src/ui/TaskApprovalPage.h/cpp              # 任务审批
src/ui/SecurityLevelConfigPage.h/cpp       # 密级设置
src/ui/SerialNumberConfigPage.h/cpp        # 流水号设置
src/ui/BarcodeConfigPage.h/cpp             # 条码设置
src/ui/ApproverConfigPage.h/cpp            # 审批员配置
src/ui/ProxyApproverConfigPage.h/cpp       # 代理审批配置
src/ui/DeviceManagementPage.h/cpp          # 设备管理
src/ui/UserManagementPage.h/cpp            # 用户管理
src/ui/RoleManagementPage.h/cpp            # 角色管理
src/ui/DepartmentManagementPage.h/cpp      # 组织架构
src/ui/SecurityPolicyConfigPage.h/cpp      # 安全策略
src/ui/SystemManagementPage.h/cpp          # 系统管理入口
src/ui/LoginLogPage.h/cpp                  # 登录日志
src/ui/OperationLogPage.h/cpp              # 操作日志
```

### 6. 插件 (src/plugins/)
```
src/plugins/client/ClientPlugin.h/cpp      # 客户端插件
src/plugins/task/TaskPlugin.h/cpp          # 任务插件
src/plugins/approval/ApprovalPlugin.h/cpp  # 审批插件
src/plugins/security/SecurityPlugin.h/cpp  # 安全插件
```

### 7. 数据模型 (models/)
```
models/user_model.h                        # 用户模型
models/role_model.h                        # 角色模型
models/department_model.h                  # 部门模型
models/device_model.h                      # 设备模型
models/task_model.h                        # 任务模型
models/security_level_model.h              # 密级模型
models/serial_number_config_model.h        # 流水号配置模型
models/barcode_config_model.h              # 条码配置模型
models/approver_config_model.h             # 审批员配置模型
models/proxy_approver_model.h              # 代理审批员模型
```

### 8. 公共头文件 (include/)
```
include/IPlugin.h                          # 插件接口
include/qt_compat.h                        # Qt版本兼容宏
```

### 9. 构建配置
```
CMakeLists.txt                             # 主构建配置
src/plugins/CMakeLists.txt                 # 插件构建配置
src/plugins/client/CMakeLists.txt
src/plugins/task/CMakeLists.txt
src/plugins/approval/CMakeLists.txt
src/plugins/security/CMakeLists.txt
tests/CMakeLists.txt                       # 测试构建配置
```

### 10. 测试代码 (tests/)
```
tests/test_database.cpp                    # 数据库测试
tests/TestAuthService.h/cpp                # 认证服务测试
tests/TestPermission.cpp                   # 权限测试
```

---

## 编译产物

### Linux 编译结果 (云主机 112.124.17.56)

| 产物 | 大小 | 路径 |
|------|------|------|
| 主程序 | 1.8MB | build/PrintBurnMonitor |
| 客户端插件 | 29KB | build/src/plugins/client/libClientPlugin.so |
| 任务插件 | 33KB | build/src/plugins/task/libTaskPlugin.so |
| 审批插件 | 41KB | build/src/plugins/approval/libApprovalPlugin.so |
| 安全插件 | 424KB | build/src/plugins/security/libSecurityPlugin.so |

### 测试程序

| 程序 | 大小 |
|------|------|
| test_database | 400KB |
| TestAuthService | 446KB |
| TestPermission | 515KB |

---

## 文件统计

| 类别 | 数量 |
|------|------|
| .cpp 文件 | 51 |
| .h 文件 | 53 |
| 源代码总行数 | ~15,000 |
| 服务层 | 14 个 |
| UI 页面 | 21 个 |
| 数据模型 | 10 个 |
| 插件 | 4 个 |