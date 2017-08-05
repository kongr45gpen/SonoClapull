#include <imgui.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sndfile.h>
#include <nfd.h>
#include <kiss_fft.h>
#include "DebugWindow.h"
#include "MediaDecoder.h"
#include "FileProcessException.h"

DebugWindow::DebugWindow() {

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
        mediaDecoder = std::make_shared<MediaDecoder>(location);

        dataExists = 1;
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
