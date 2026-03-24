# 测试目录的CMakeLists.txt

# 添加Qt测试模块
find_package(Qt5 REQUIRED COMPONENTS Core Sql Test)

# 启用测试编译
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/services)
include_directories(${CMAKE_SOURCE_DIR}/src/common)
include_directories(${CMAKE_SOURCE_DIR}/src/common/repository)
include_directories(${CMAKE_SOURCE_DIR}/models)

# 定义测试源文件
set(REPOSITORY_SOURCES
    ${CMAKE_SOURCE_DIR}/src/common/repository/database_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/user_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/role_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/department_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/device_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/task_repository.cpp
)

# 定义服务源文件 (用于 TestAuthService)
set(SERVICE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/services/AuthService.cpp
)

# ============================================================================
# 测试目标: test_database
# 数据库仓储层测试 (独立可执行文件，不使用 Qt Test 框架)
# ============================================================================
add_executable(test_database
    test_database.cpp
    ${REPOSITORY_SOURCES}
)

target_link_libraries(test_database
    Qt5::Core
    Qt5::Sql
)

add_test(NAME test_database COMMAND test_database)

# ============================================================================
# 测试目标: TestAuthService
# AuthService 单元测试 (使用 Qt Test 框架)
# ============================================================================
add_executable(TestAuthService
    TestAuthService.cpp
    TestAuthService.h
    ${SERVICE_SOURCES}
    ${REPOSITORY_SOURCES}
)

target_link_libraries(TestAuthService
    Qt5::Core
    Qt5::Sql
    Qt5::Test
)

add_test(NAME TestAuthService COMMAND TestAuthService)

# ============================================================================
# 测试目标: TestPermission
# 权限管理单元测试 (使用 Qt Test 框架)
# ============================================================================
add_executable(TestPermission
    TestPermission.cpp
    ${SERVICE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/services/UserManagementService.cpp
    ${CMAKE_SOURCE_DIR}/src/services/RoleManagementService.cpp
    ${REPOSITORY_SOURCES}
)

target_link_libraries(TestPermission
    Qt5::Core
    Qt5::Sql
    Qt5::Test
)

add_test(NAME TestPermission COMMAND TestPermission)