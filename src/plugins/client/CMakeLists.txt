# ClientPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    ClientPlugin.h
    ClientPlugin.cpp
)

add_library(ClientPlugin MODULE ${PLUGIN_SOURCES})

target_link_libraries(ClientPlugin PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)

# 安装插件到plugins目录
install(TARGETS ClientPlugin DESTINATION plugins)