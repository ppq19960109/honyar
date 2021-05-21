cmake_minimum_required(VERSION 3.10)
set (APP_NAME hylink)

# set the project name
project(${APP_NAME})
# set the project name and version
project(${APP_NAME} VERSION 1.0)

message(STATUS "PROJECT_SOURCE_DIR:${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR:${PROJECT_BINARY_DIR}")

option (USE_U86 "Use CROSS_COMPILE u86" OFF)

if (USE_U86)
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
src
src/frame
src/hylink
src/hytool
)
if (USE_U86)
set (LIBHYTOOL_DIR ../libhytool/install_u86) 
add_definitions(-DARCH)
set (INSTALL_DIRS ${PROJECT_SOURCE_DIR}/src/) 
else()
set (LIBHYTOOL_DIR ../libhytool/install) 
set (SRC_DIRS ${SRC_DIRS} rk3308) 
set (INSTALL_DIRS ${PROJECT_SOURCE_DIR}/src/ rk33088) 
endif ()

include_directories (${SRC_DIRS})
FOREACH(SRC_DIR ${SRC_DIRS})
        MESSAGE("SRC_DIR: ${SRC_DIR}")
        aux_source_directory(${SRC_DIR} SRC_FILES)
ENDFOREACH()

# 添加 libhytool

set (LIBHYTOOL_HEADER_DIRS
${LIBHYTOOL_DIR}/include
${LIBHYTOOL_DIR}/include/base64        
${LIBHYTOOL_DIR}/include/cJSON
${LIBHYTOOL_DIR}/include/klib
${LIBHYTOOL_DIR}/include/signal
${LIBHYTOOL_DIR}/include/tcp
${LIBHYTOOL_DIR}/include/tcp/epoll
${LIBHYTOOL_DIR}/include/tcp/thread
${LIBHYTOOL_DIR}/include/timer
${LIBHYTOOL_DIR}/include/tool)

# message(STATUS "LIBHYTOOL_HEADER_DIRS:${LIBHYTOOL_HEADER_DIRS}")
include_directories (${LIBHYTOOL_HEADER_DIRS})
link_directories(${LIBHYTOOL_DIR}/lib)

get_property(HEADER_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "SRC_DIRS:${SRC_DIRS}")
message(STATUS "HEADER_DIRS:${HEADER_DIRS}")
message(STATUS "SRC_FILES:${SRC_FILES}")

link_libraries(-ldl -lm pthread -lrt)
# add the executable
add_library(${APP_NAME} STATIC ${SRC_FILES})
target_link_libraries (${APP_NAME}  libhytool.a)
# 指定安装路径
install (TARGETS ${APP_NAME} 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install (DIRECTORY ${INSTALL_DIRS} DESTINATION include
        FILES_MATCHING
        PATTERN "*.h")