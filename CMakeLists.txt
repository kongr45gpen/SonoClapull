cmake_minimum_required(VERSION 3.8)
project(SonoClapull)

set(CMAKE_CXX_STANDARD 11)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui")
include_directories ("${PROJECT_SOURCE_DIR}/lib/nativefiledialog/src/include")
include_directories ("${PROJECT_SOURCE_DIR}/lib/kissfft")
add_subdirectory (lib)

set(SOURCE_FILES main.cpp imgui_impl_glfw.cpp DebugWindow.cpp DebugWindow.h)
add_executable(SonoClapull ${SOURCE_FILES})

target_link_libraries (SonoClapull ImGui ImGuiDemo glfw GL NativeFileDialog sndfile KissFFT)

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "-O3")
