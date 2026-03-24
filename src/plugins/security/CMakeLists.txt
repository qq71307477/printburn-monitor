# SecurityPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    SecurityPlugin.h
    SecurityPlugin.cpp
)

# 添加依赖的服务文件
set(DEPENDENCY_SOURCES
    ${CMAKE_SOURCE_DIR}/src/services/SensitiveWordsService.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/database_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/user_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/role_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/department_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/device_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/common/repository/task_repository.cpp
)

add_library(SecurityPlugin MODULE ${PLUGIN_SOURCES} ${DEPENDENCY_SOURCES})

target_include_directories(SecurityPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/common/repository
    ${CMAKE_SOURCE_DIR}/models
)

target_link_libraries(SecurityPlugin PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql)

# 安装插件到plugins目录
install(TARGETS SecurityPlugin DESTINATION plugins)