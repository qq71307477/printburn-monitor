cmake_minimum_required(VERSION 3.16)

project(PrintBurnMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt库
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network Test)

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

# 创建主应用程序
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network)

# 添加插件子目录
add_subdirectory(src/plugins)

# 启用测试
enable_testing()

# 添加测试子目录 (暂时禁用，需要重构)
# add_subdirectory(tests)