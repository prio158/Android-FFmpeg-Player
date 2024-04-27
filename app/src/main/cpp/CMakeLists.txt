# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.18.1)

project("player")

aux_source_directory(src SOURCE)

# 生成最终的动态库
add_library(
        ${CMAKE_PROJECT_NAME} SHARED
        common/Log.h
        common/NoCpoyable.h
        common/Mutex.h
        common/Mutex.cpp
        common/SafeQueue.h
        common/SafeQueueNoLock.h
        common/Timer.h
        common/Timer.cpp
        common/IOSchedule.h
        common/IOSchedule.cpp
        common/Tools.h
        common/Tools.cpp
        ${SOURCE}
)

# ANDROID_ABI 代表Android中build.gradle指定的CPU架构：abiFilters，如果abiFilters指定了多个架构，那么这个工程会编译多次
set(libs ${CMAKE_SOURCE_DIR}/${ANDROID_ABI})
# 给编译器传入指定参数
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${libs}")


add_library(
        avfilter
        SHARED
        IMPORTED
)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${libs}/libavfilter.so
)
add_library(
        avformat
        SHARED
        IMPORTED
)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${libs}/libavformat.so
)
add_library(
        avcodec
        SHARED
        IMPORTED
)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${libs}/libavcodec.so
)
add_library(
        avutil
        SHARED
        IMPORTED
)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${libs}/libavutil.so
)
add_library(
        swresample
        SHARED
        IMPORTED
)
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${libs}/libswresample.so
)
add_library(
        swscale
        SHARED
        IMPORTED
)
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${libs}/libswscale.so
)
add_library(
        avdevice
        SHARED
        IMPORTED
)
set_target_properties(
        avdevice
        PROPERTIES IMPORTED_LOCATION
        ${libs}/libavdevice.so
)
add_library(
        rtmp
        STATIC
        IMPORTED
)
set_target_properties(
        rtmp
        PROPERTIES IMPORTED_LOCATION
        ${libs}/librtmp.a
)

# 引入ffmpeg头文件
include_directories(include common)

# android、OpenSLES、log、z都是NDK提供给我们的动态链接库
target_link_libraries(${CMAKE_PROJECT_NAME}
        avfilter
        avformat
        avcodec
        avutil
        swresample
        swscale
        avdevice
        rtmp
        z
        android
        OpenSLES
        log)