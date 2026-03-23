# ApprovalPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    ApprovalPlugin.h
    ApprovalPlugin.cpp
)

add_library(ApprovalPlugin MODULE ${PLUGIN_SOURCES})

target_link_libraries(ApprovalPlugin PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)

# 安装插件到plugins目录
install(TARGETS ApprovalPlugin DESTINATION plugins)