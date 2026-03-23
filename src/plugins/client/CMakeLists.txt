# ClientPlugin CMakeLists.txt

set(PLUGIN_SOURCES
    ClientPlugin.h
    ClientPlugin.cpp
)

add_library(ClientPlugin MODULE ${PLUGIN_SOURCES})

target_include_directories(ClientPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ClientPlugin PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql)

# 安装插件到plugins目录
install(TARGETS ClientPlugin DESTINATION plugins)