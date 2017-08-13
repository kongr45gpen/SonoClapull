#include <imgui.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sndfile.h>
#include <nfd.h>
#include <kiss_fftr.h>
#include <algorithm>
#include "DebugWindow.h"
#include "../MediaDecoder.h"
#include "../FileProcessException.h"
#include "../FileEndException.h"
#include "../ToneLocator.h"

DebugWindow::DebugWindow() {
    for(int i = 0 ; i < 512; i ++) {
        fftdata[i] = 0;
    }
}

void DebugWindow::draw() {
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("FFT Debug");

    ImGui::Text("Selected file:");
    ImGui::SameLine();

    ImGui::InputText("", location, PATH_MAX);
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        // TODO: Don't stop thread
        nfdchar_t *outPath = nullptr;
        nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &outPath );

        if ( result == NFD_OKAY ) {
            if (strlen(outPath) >= PATH_MAX) {
                outPath[PATH_MAX - 1] = '\0';
            }
            strcpy(location, outPath);
            free(outPath);
        }
    };

    ImGui::PushStyleColor(ImGuiCol_Button, {0.3f, 0.5f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.3f, 0.7f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.3f, 0.9f, 0.3f, 1.0f});
    if (ImGui::Button("Analyse", {ImGui::GetContentRegionAvailWidth(), 0})) {
        analyse();
    };
    ImGui::PopStyleColor(3);

    if (dataExists) {
        ImGui::Text("Format: %s", mediaDecoder->getFormat().c_str());
        ImGui::Text("Sample Rate: %d", mediaDecoder->getSampleRate());

        static int start = 00;
        static bool move = false;
        ImGui::Text("Start %d", start); ImGui::SameLine(); ImGui::Checkbox("move", &move); ImGui::SameLine();
        if (ImGui::Button("-64")) {
            start -= 64;
        } else if (move) {
            start += 1;
        }



        if (ImGui::Button("next samplepack") || dataExists) {
            try {
                data = mediaDecoder->getNextSamples();

                kiss_fft_cpx fft[512];

                kiss_fftr_cfg cfg = kiss_fftr_alloc(1024, 0, nullptr, nullptr);
                kiss_fftr(cfg, data->data(), fft);
                for (int i = 0 ; i < 512 ; i++) {
                    fftdata[i] = (float) sqrt(pow(fft[i].r, 2) + pow(fft[i].i, 2));
//                    fftdata[i] = (float) atan2(fft[i].i+0.000000007, fft[i].r);
                }
            } catch (FileEndException &e) {
                dataExists = false;
            }
        }
        ImGui::PlotLines("", fftdata, 512, 0, NULL, *std::min_element(fftdata, fftdata+512), *std::max_element(fftdata, fftdata+512), ImVec2(ImGui::GetContentRegionAvailWidth(),(ImGui::GetContentRegionAvail()).y/2));
        ImGui::Text("Progress: %f", mediaDecoder->getProgress());
    }

    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Error analysing file (%s):", location);
        ImGui::Text("%s", openError.c_str());

        if (ImGui::Button("OK", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void DebugWindow::analyse() {
    try {
//        mediaDecoder = std::make_shared<MediaDecoder>(location,1024);
//
//        data = mediaDecoder->getNextSamples();
//        dataExists = 1;
        ToneLocator toneLocator(location);
        toneLocator.locateClock();
        toneLocator.getData();

    } catch (Exception &e) {
        ImGui::OpenPopup("Error");
        openError = e.getWhat();
    } catch (FileProcessException &e) {
        ImGui::OpenPopup("Error");
        openError = e.what();
    }
}

DebugWindow::Exception::Exception(const std::string &what) : what(what) {}

const std::string &DebugWindow::Exception::getWhat() const {
    return what;
}
