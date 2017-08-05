cmake_minimum_required(VERSION 3.8)
project(SonoClapull)

set(CMAKE_CXX_STANDARD 11)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG_AVCODEC  libavcodec  REQUIRED)
pkg_check_modules(FFMPEG_AVUTIL   libavutil   REQUIRED)
pkg_check_modules(FFMPEG_AVFORMAT libavformat REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui")
include_directories ("${PROJECT_SOURCE_DIR}/lib/nativefiledialog/src/include")
include_directories ("${PROJECT_SOURCE_DIR}/lib/kissfft")
include_directories ("${PROJECT_SOURCE_DIR}/lib/kissfft/tools")
add_subdirectory (lib)

set(SOURCE_FILES main.cpp imgui_impl_glfw.cpp DebugWindow.cpp DebugWindow.h MediaDecoder.cpp FileProcessException.cpp FileProcessException.h)
add_executable(SonoClapull ${SOURCE_FILES})
target_link_libraries (SonoClapull ImGui ImGuiDemo glfw GL NativeFileDialog KissFFT)
target_link_libraries (SonoClapull ${FFMPEG_AVCODEC_LIBRARIES} ${FFMPEG_AVUTIL_LIBRARIES} ${FFMPEG_AVFORMAT_LIBRARIES})

add_executable(experiments frequencyCalculator.cpp)
target_link_libraries(experiments ${FFMPEG_AVCODEC_LIBRARIES} ${FFMPEG_AVUTIL_LIBRARIES} ${FFMPEG_AVFORMAT_LIBRARIES})

add_executable(FFTPlayground FFTPlayground.cpp imgui_impl_glfw.cpp)
target_link_libraries(FFTPlayground ImGui ImGuiDemo glfw GL KissFFTr)

add_definitions(-D__STDC_CONSTANT_MACROS)

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "-O3")
