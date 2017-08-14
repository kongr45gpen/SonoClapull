cmake_minimum_required(VERSION 3.5)
project(SonoClapull)

set(CMAKE_CXX_STANDARD 14)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost 1.50 COMPONENTS filesystem REQUIRED )

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG_AVCODEC  libavcodec  REQUIRED)
pkg_check_modules(FFMPEG_AVUTIL   libavutil   REQUIRED)
pkg_check_modules(FFMPEG_AVFORMAT libavformat REQUIRED)
pkg_check_modules(FFMPEG_SWRESAMPLE libswresample REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui")
include_directories ("${PROJECT_SOURCE_DIR}/lib/nativefiledialog/src/include")
include_directories ("${PROJECT_SOURCE_DIR}/lib/kissfft")
include_directories ("${PROJECT_SOURCE_DIR}/lib/kissfft/tools")
add_subdirectory (lib)

set(SOURCE_FILES main.cpp imgui_impl_glfw.cpp gui/DebugWindow.cpp gui/LoadWindow.cpp MediaDecoder.cpp FileProcessException.cpp FileProcessException.h FileEndException.cpp FileEndException.h ToneLocator.cpp ToneLocator.h Processing.cpp Processing.h gui/ProcessingWindow.cpp gui/ProcessingWindow.h ProcessingThread.cpp ProcessingThread.h)
add_executable(SonoClapull ${SOURCE_FILES})
target_link_libraries (SonoClapull ImGui ImGuiDemo glfw GL pthread NativeFileDialog KissFFTr)
target_link_libraries (SonoClapull ${FFMPEG_AVCODEC_LIBRARIES} ${FFMPEG_AVUTIL_LIBRARIES} ${FFMPEG_AVFORMAT_LIBRARIES} ${FFMPEG_SWRESAMPLE_LIBRARIES})
target_link_libraries (SonoClapull ${Boost_LIBRARIES})

add_executable(experiments frequencyCalculator.cpp)
target_link_libraries(experiments ${FFMPEG_AVCODEC_LIBRARIES} ${FFMPEG_AVUTIL_LIBRARIES} ${FFMPEG_AVFORMAT_LIBRARIES})

add_executable(FFTPlayground FFTPlayground.cpp imgui_impl_glfw.cpp)
target_link_libraries(FFTPlayground ImGui ImGuiDemo glfw GL KissFFTr)

add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DBOOST_LOG_DYN_LINK)

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "-O3")
