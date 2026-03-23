# ApprovalPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    ApprovalPlugin.h
    ApprovalPlugin.cpp
)

add_library(ApprovalPlugin MODULE ${PLUGIN_SOURCES})

target_include_directories(ApprovalPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ApprovalPlugin PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql)

# 安装插件到plugins目录
install(TARGETS ApprovalPlugin DESTINATION plugins)