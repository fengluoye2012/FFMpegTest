# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

#CMakeLists.txt配置：https://blog.csdn.net/u012528526/article/details/80647537文档；
# Sets the minimum version of CMake required to build the native library.
# 构建jni 所需的CMake的最低版本（必须项）
cmake_minimum_required(VERSION 3.4.1)

# 设置生成的so动态库最后输出的路径，
# set后面两个参数， 前面一个是命令，后面是路径。
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

#设置生成so库的名称，并为源文件提供一个相对路径
#设置统配符
# 将.cpp文件配置在CMakeLists.txt中，如果是在子目录下，
file(GLOB native_srcs "*.cpp")
file(GLOB native_srcs_dy "dynamic/*.cpp")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.  设置库的名称，也就是生成的so名称，在java代码中加载so库需要与此一致。
        native-lib

        # Sets the library as a shared library. 将library设置为一个共享库
        SHARED

        # Provides a relative path to your source file(s).
        # 为源文件提供一个相对路径,可以使用通配符，也可以一个一个文件的引入；
        #native-lib.cpp
        ${native_srcs}
        )

add_library(
        dynamic-native-lib
        SHARED
        ${native_srcs_dy}
)


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

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#加载第三方库
#todo

#链接所有的库，比如上面我们添加的第三方库，添加的本地log库
target_link_libraries( # Specifies the target library.
        #通过add_library添加的直接写设置的名字，
        #一种是SHARED,一般来说就是我们需要生成的so文件
        #二种是STATIC IMPORTED 添加的第三方静态库
        native-lib
        dynamic-native-lib

        # Links the target library to the log library
        # included in the NDK.
        #链接本地NDK里面的库
        ${log-lib})