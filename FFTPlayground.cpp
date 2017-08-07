#include <iostream>

#include "lib/imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <kiss_fftr.h>
#include <kiss_fft.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui OpenGL2 example", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui binding
    ImGui_ImplGlfw_Init(window, true);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/extra_fonts/Cousine-Regular.ttf", 15.0f);
    io.Fonts->AddFontFromFileTTF("../lib/imgui/extra_fonts/DroidSans.ttf", 16.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/extra_fonts/ProggyClean.ttf", 13.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = true;
    ImVec4 clear_color = ImColor(35, 44, 59);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfw_NewFrame();

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("FFT Playground");

        static const int maxExtra = 7000;

        static int T = 4;
        static int start = 0;
        static int stop = 1024;
        static float dc = 0;
        static float ac = 1;
        int shift = 0;
        int distance = 0;
        static bool sampleWindow = false;
        static bool copy = false;
        static int extra = 0;
        double f = 1.0/T;

        int startstop[2] = {start,stop};
        int shiftwindow[2] = {shift,distance};
        float dcac[2] = {dc,ac};

        ImGui::SliderFloat2("dc/ac coefficient", dcac, -10.0, 10.0, "%.3f", 5);
        ImGui::SameLine(); if(ImGui::Button("Reset DC")) dcac[0] = 0;
        ImGui::SameLine(); if(ImGui::Button("Reset AC")) dcac[1] = 1;
        ImGui::SliderInt("sine period", &T, 1, 1024);
        ImGui::SliderInt2("sine start/stop", startstop, 0, 1024);
        ImGui::SliderInt2("start/stop shift/window", shiftwindow, -5, 5);
        ImGui::Checkbox("window", &sampleWindow);
        ImGui::SameLine(); ImGui::Checkbox("copy instead of empty extra samples", &copy);
        ImGui::SliderInt("extra samples", &extra, 0, 6024);

        start = startstop[0]; stop = startstop[1];
        shift = shiftwindow[0]; distance = shiftwindow[1];
        dc = dcac[0]; ac = dcac[1];

        static std::vector<float> samples(1024+maxExtra);
        static std::vector<float> windowFactor(1024+maxExtra);
        static std::vector<float> results(512+maxExtra/2);

        if (extra % 2 != 0) {
            extra += 1;
        }

        if (stop < start) {
            std::swap(start, stop);
        }

        // Calculate modifiers
        if (shift != 0) {
            start += shift;
            stop  += shift;
        }
        if (distance != 0) {
            start += distance;
            stop  -= distance;
        }

        for (int i = 0; i < 1024; i++) {
            if (i < start || i > stop) {
                samples[i] = 0;
            } else {
                samples[i] = ac * (float) sin(2 * M_PI * f * i) + dc;
            }
        }
        if (copy) {
            for (int i = 1024; i < 1024+extra; i++) {
                samples[i] = samples[i % 1024];
            }
        } else {
            for (int i = 1024; i < 1024 + extra; i++) {
                samples[i] = 0;
            }
        }
        if (sampleWindow) {
            for (int i = 0; i < 1024; i ++) {
                static float a = 0.54;
                static float b = 1-a;
                windowFactor[i] = a - b* (float) cos(2*M_PI*i/(1024-1));
                samples[i] *= windowFactor[i];
            }
        }

        ImGui::PlotLines("", samples.data(), 1024+extra, 0, NULL, -2.0f, 2.0f, ImVec2(ImGui::GetContentRegionAvailWidth(),(ImGui::GetContentRegionAvail()).y/4));

        kiss_fft_cpx fft_out[512+extra/2];

        kiss_fftr_cfg cfg = kiss_fftr_alloc(1024+extra, 0, nullptr, nullptr);
        kiss_fftr(cfg, samples.data(), fft_out);

        float max = 0;
        float min = 0;

        static int selection = 0;

        for (int i = 0 ; i < 512+extra/2; i ++) {
            results[i] = (float) sqrt(pow(fft_out[i].i , 2)+ pow(fft_out[i].r, 2));

            if (selection == 1 || selection == 2) {
                results[i] = log(results[i]);
            }
            if (selection == 2) {
                results[i] = pow(results[i], 2);
            }
            if (selection == 3) {
                results[i] = exp(results[i]);
            }

            if (results[i] > max) max = results[i];
            if (results[i] < min) min = results[i];
        }

        ImGui::Text("Max: %f,\t Min: %f", max, min);
        ImGui::SameLine();
        ImGui::Combo("", &selection, "Linear\0dB\0db^2\0Exp\0\0");


        ImGui::PlotLines("", results.data(), 512+extra/2, 0, NULL, min, max, ImVec2(ImGui::GetContentRegionAvailWidth(),(ImGui::GetContentRegionAvail()).y/3));

        static bool showReconstructedWave = true;
        static bool keepPhase = true;

        if (showReconstructedWave) {
            max = 0;
            min = 0;
            for (int j = 0; j < 1024 + extra; j++) {
                samples[j] = 0;
                for (int i = 0; i < 512 + extra / 2; i++) {
                    if (keepPhase) {
                        samples[j] += fft_out[i].r * cos(-2 * M_PI * (i / (1024.0 + extra)) * j) +
                                      fft_out[i].i * sin(-2 * M_PI * (i / (1024.0 + extra)) * j);
                    } else {
                        samples[j] += results[i] * cos(2 * M_PI * (i / (512.0 + extra / 2.0)) * j);
                    }
                }
                if (samples[j] > max) max = samples[j];
                if (samples[j] < min) min = samples[j];
            }
        }


        ImGui::Checkbox("",&showReconstructedWave);
        ImGui::SameLine();
        ImGui::TextColored({0.2, 0.5, 0.9, 1.0}, "Reconstructed wave");
        if (showReconstructedWave) {
            ImGui::SameLine();
            ImGui::Text("(CPU-intensive) \tMax: %f,\t Min: %f", max, min);
            ImGui::SameLine();
            ImGui::Checkbox("Keep phase data", &keepPhase);
            ImGui::PlotLines("", samples.data(), 1024 + extra, 0, NULL, min, max,
                             ImVec2(ImGui::GetContentRegionAvailWidth(), (ImGui::GetContentRegionAvail()).y / 2));
        }

        if (sampleWindow) {
            ImGui::PushStyleColor(ImGuiCol_PlotLines, {0.9, 0.8, 0.2, 1.0});
            ImGui::TextColored({0.9, 0.8, 0.2, 1.0}, "Window");
            ImGui::PlotLines("", windowFactor.data(), 1024, 0, NULL, 0.0f, 1.0f, ImVec2(ImGui::GetContentRegionAvailWidth(),(ImGui::GetContentRegionAvail()).y));
            ImGui::PopStyleColor();
        }

        ImGui::End();


        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();

    return 0;
}
