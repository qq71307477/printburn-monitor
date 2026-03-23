# SecurityPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    SecurityPlugin.h
    SecurityPlugin.cpp
)

add_library(SecurityPlugin MODULE ${PLUGIN_SOURCES})

target_link_libraries(SecurityPlugin PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)

# 安装插件到plugins目录
install(TARGETS SecurityPlugin DESTINATION plugins)