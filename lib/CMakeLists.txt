find_package(GTK2 REQUIRED)

add_library(ImGui imgui/imgui.cpp imgui/imgui_draw.cpp)
add_library(ImGuiDemo imgui/imgui_demo.cpp)
add_library(NativeFileDialog nativefiledialog/src/nfd_common.c nativefiledialog/src/nfd_gtk.c)
add_library(KissFFT kissfft/kiss_fft.c)

include_directories(${GTK2_INCLUDE_DIRS})

target_link_libraries (NativeFileDialog ${GTK2_LIBRARIES})
