# TaskPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    TaskPlugin.h
    TaskPlugin.cpp
)

add_library(TaskPlugin MODULE ${PLUGIN_SOURCES})

target_link_libraries(TaskPlugin PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)

# 安装插件到plugins目录
install(TARGETS TaskPlugin DESTINATION plugins)