# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(myNpmLib)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Source directory: ${NATIVERENDER_ROOT_PATH}")

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(
    ${NATIVERENDER_ROOT_PATH}
    ${NATIVERENDER_ROOT_PATH}/include
    ${NATIVERENDER_ROOT_PATH}/common
    ${NATIVERENDER_ROOT_PATH}/utils
)

add_library(libnativerender SHARED
    render/plugin_render.cpp
    render/sample_graphics.cpp
    manager/plugin_manager.cpp
    utils/adaptation_util.cpp
    napi_init.cpp
)

find_library(
    # Sets the name of the path variable.
    libuv-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    uv
)

target_link_libraries(libnativerender PUBLIC
    EGL
    GLESv3
    libhilog_ndk.z.so
    libace_napi.z.so
    libnative_drawing.so
    libnative_window.so
    libace_ndk.z.so
    libnative_display_soloist.so
    libpixelmap.so
    libnative_display_manager.so
    ${libuv-lib}
    libnative_window.so
)
