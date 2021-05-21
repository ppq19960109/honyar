cmake_minimum_required(VERSION 3.10)
set (APP_NAME hytool)

# set the project name
project(${APP_NAME})
# set the project name and version
project(${APP_NAME} VERSION 1.0)

message(STATUS "PROJECT_SOURCE_DIR:${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR:${PROJECT_BINARY_DIR}")

option (USE_U86 "Use CROSS_COMPILE u86" OFF)

if (USE_U86)
MESSAGE("USE_U86 ON")
set (CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install_u86")
set (CROSS_COMPILE /home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-)
else()
set (CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install")
set (CROSS_COMPILE arm-rockchip-linux-gnueabihf-)
endif ()
set (CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set (CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
set (SRC_DIRS 
base64        
cJSON
klib
signall
tcp
tcp/epoll
tcp/thread
timer
tool
)
include_directories (${SRC_DIRS})
FOREACH(SRC_DIR ${SRC_DIRS})
        MESSAGE("SRC_DIR: ${SRC_DIR}")
        aux_source_directory(${SRC_DIR} SRC_FILES)
ENDFOREACH()

get_property(HEADER_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "SRC_DIRS:${SRC_DIRS}")
message(STATUS "HEADER_DIRS:${HEADER_DIRS}")
message(STATUS "SRC_FILES:${SRC_FILES}")

link_libraries(-ldl -lm pthread -lrt)
# add the executable
add_library(${APP_NAME} STATIC ${SRC_FILES})

# 指定安装路径
install (TARGETS ${APP_NAME} 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install (DIRECTORY ${PROJECT_SOURCE_DIR}/ DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "install*" EXCLUDE
        PATTERN "test" EXCLUDE
        PATTERN ".*" EXCLUDE
        REGEX "${CMAKE_CURRENT_BINARY_DIR}" EXCLUDE)