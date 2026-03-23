# 测试目录的CMakeLists.txt

# 添加Qt测试模块
find_package(Qt6 REQUIRED COMPONENTS Core Test)

# 启用测试编译
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# 暂时禁用测试，需要重构后启用
# add_subdirectory(tests) 在主 CMakeLists.txt 中注释掉