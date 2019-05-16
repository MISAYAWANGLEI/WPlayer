cmake_minimum_required(VERSION 3.4.1)

file(GLOB source_file src/main/cpp/*.cpp)
# ${source_file}不起作用，奇葩，目前用一下方式加入编译
add_library(
        native-lib
        SHARED
        src/main/cpp/AudioChannel.cpp
        src/main/cpp/CppCallJavaUtils.cpp
        src/main/cpp/VideoChannel.cpp
        src/main/cpp/native-lib.cpp
        src/main/cpp/WFFmpeg.cpp)

include_directories(src/main/cpp/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS -L${CMAKE_SOURCE_DIR}/src/main/cpp/libs/${ANDROID_ABI}")

target_link_libraries(
        native-lib
        avformat avcodec avfilter avutil swresample swscale
        log z android OpenSLES)