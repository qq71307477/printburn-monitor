# SecurityPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    SecurityPlugin.h
    SecurityPlugin.cpp
)

add_library(SecurityPlugin MODULE ${PLUGIN_SOURCES})

target_include_directories(SecurityPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(SecurityPlugin PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql)

# 安装插件到plugins目录
install(TARGETS SecurityPlugin DESTINATION plugins)