cmake_minimum_required(VERSION 3.16)

project(PrintBurnMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt库
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql Network Test PrintSupport)

# 设置Qt相关编译选项
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/ui)
include_directories(${CMAKE_SOURCE_DIR}/src/services)
include_directories(${CMAKE_SOURCE_DIR}/src/common)
include_directories(${CMAKE_SOURCE_DIR}/models)

# 定义源文件
set(SOURCES
    src/main/main.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/LoginWindow.h
    src/ui/LoginWindow.cpp
    src/ui/PersonalHomePage.h
    src/ui/PersonalHomePage.cpp
    src/ui/PrintApplicationPage.h
    src/ui/PrintApplicationPage.cpp
    src/ui/BurnApplicationPage.h
    src/ui/BurnApplicationPage.cpp
    src/ui/PrintOutputPage.h
    src/ui/PrintOutputPage.cpp
    src/ui/BurnOutputPage.h
    src/ui/BurnOutputPage.cpp
    src/ui/TaskApprovalPage.h
    src/ui/TaskApprovalPage.cpp
    src/common/PluginManager.h
    src/common/PluginManager.cpp
)

# 定义数据访问层源文件
file(GLOB_RECURSE REPOSITORY_SOURCES "src/common/repository/*.cpp" "src/common/repository/*.h")
list(APPEND SOURCES ${REPOSITORY_SOURCES})

# 定义模型文件
file(GLOB_RECURSE MODEL_SOURCES "models/*.h")
list(APPEND SOURCES ${MODEL_SOURCES})

# 定义服务层源文件
file(GLOB_RECURSE SERVICE_SOURCES "src/services/*.cpp" "src/services/*.h")
list(APPEND SOURCES ${SERVICE_SOURCES})

# 添加 SecurityPlugin 源文件（被 TaskService 直接使用）
list(APPEND SOURCES
    src/plugins/security/SecurityPlugin.h
    src/plugins/security/SecurityPlugin.cpp
    src/ui/SecurityLevelConfigPage.h
    src/ui/SecurityLevelConfigPage.cpp
    src/ui/ApproverConfigPage.h
    src/ui/ApproverConfigPage.cpp
    src/ui/SerialNumberConfigPage.h
    src/ui/SerialNumberConfigPage.cpp
    src/ui/BarcodeConfigPage.h
    src/ui/BarcodeConfigPage.cpp
    src/ui/SystemManagementPage.h
    src/ui/SystemManagementPage.cpp
    src/ui/UserManagementPage.h
    src/ui/UserManagementPage.cpp
    src/ui/RoleManagementPage.h
    src/ui/RoleManagementPage.cpp
    src/ui/DepartmentManagementPage.h
    src/ui/DepartmentManagementPage.cpp
    src/ui/SecurityPolicyConfigPage.h
    src/ui/SecurityPolicyConfigPage.cpp
    src/ui/ProxyApproverConfigPage.h
    src/ui/ProxyApproverConfigPage.cpp
)

# 创建主应用程序
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Network Qt5::Test Qt5::PrintSupport)

# 添加插件子目录
add_subdirectory(src/plugins)

# 启用测试
enable_testing()

# 添加测试子目录
add_subdirectory(tests)