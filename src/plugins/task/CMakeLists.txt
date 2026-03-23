# TaskPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    TaskPlugin.h
    TaskPlugin.cpp
)

add_library(TaskPlugin MODULE ${PLUGIN_SOURCES})

target_include_directories(TaskPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(TaskPlugin PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql)

# 安装插件到plugins目录
install(TARGETS TaskPlugin DESTINATION plugins)