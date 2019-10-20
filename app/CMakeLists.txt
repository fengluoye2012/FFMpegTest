#目前没有引用，只是作为事例

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

#CMakeLists.txt配置：https://blog.csdn.net/u012528526/article/details/80647537文档；
# Sets the minimum version of CMake required to build the native library.
# 构建jni 所需的CMake的最低版本（必须项）
cmake_minimum_required(VERSION 3.4.1)

#定义工程名称
PROJECT(FFmpegTest)

# 设置生成的so动态库最后输出的路径，
# set后面两个参数， 前面一个是命令，后面是路径。
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})


#设置生成so库的名称，并为源文件提供一个相对路径
#设置统配符
# 将.cpp文件配置在CMakeLists.txt中，如果是在子目录下，
file(GLOB native_srcs_dy "${CMAKE_SOURCE_DIR}/src/main/cpp/*.cpp")
file(GLOB native_srcs_singleton "${CMAKE_SOURCE_DIR}/src/main/cpp/Singleton/*.cpp")
file(GLOB native_srcs_play "${CMAKE_SOURCE_DIR}/src/main/cpp/play/*.cpp")
file(GLOB native_srcs_videoPlay "${CMAKE_SOURCE_DIR}/src/main/cpp/videoPlay/*.cpp")
file(GLOB ffmpeg_srcs "${CMAKE_SOURCE_DIR}/src/main/cpp/*.c")
file(GLOB ffmpeg_log "${CMAKE_SOURCE_DIR}/src/main/cpp/log/*.c")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#add_library( # Sets the name of the library.  设置库的名称，也就是生成的so名称，在java代码中加载so库需要与此一致。
#        native-lib
#
#        # Sets the library as a shared library. 将library设置为一个共享库;STATIC:静态库，SHARED：生成动态库或共享库；
#        SHARED
#
#        # Provides a relative path to your source file(s).
#        # 为源文件提供一个相对路径,可以使用通配符，也可以一个一个文件的引入；
#        ${native_srcs}
#        ${native_log})

#
add_library(
        dynamic-native-lib

        SHARED

        ${ffmpeg_log}
        ${native_srcs_dy}
        ${native_srcs_singleton}
        ${native_srcs_play}
        ${native_srcs_videoPlay}
        ${ffmpeg_srcs})


# CMAKE_SOURCE_DIR CMAKE_SOURCE_DIR 表示CMakeLists.txt所在文件的根目录，如当前的根目录是app文件夹；
#设置头文件目录
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)

# ffmpeg so库
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec
        PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/armeabi-v7a/libavcodec.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(
        avformat
        PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/armeabi-v7a/libavformat.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(
        avfilter
        PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/armeabi-v7a/libavfilter.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(
        avutil
        PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/armeabi-v7a/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(
        swresample
        PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/armeabi-v7a/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(
        swscale
        PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/armeabi-v7a/libswscale.so)


# ffmpeg  end

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

#使用本地系统库
find_library( # Sets the name of the path variable.
        log-lib #//log日志,默认都需要添加

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

#找打Android lib库里面的libOpenSLES.so的库
find_library(OpenSLES-lib
        OpenSLES)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#加载第三方库
#只需要修改将两个libpng名称改成你需要的，可以随意取，该名称是后面用来链接使用的，再修改一下对应文件路径就ok。

#dynamic-native-lib 链接日志库
target_link_libraries( # Specifies the target library.
        #目标库
        dynamic-native-lib

        #目标库需要链接的库
        avcodec
        avformat
        avfilter
        avutil
        swresample
        swscale

        # shader渲染视频
        EGL
        GLESv2

        ${OpenSLES-lib}

        android
        ${log-lib})