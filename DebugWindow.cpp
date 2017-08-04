#include <imgui.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sndfile.h>
#include <nfd.h>
#include <kiss_fft.h>
#include "DebugWindow.h"

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
        const float* datas = data;
        ImGui::PlotLines("", datas, 4096, 0, NULL, -1.0f, 1.0f, ImVec2(ImGui::GetContentRegionAvailWidth(),(ImGui::GetContentRegionAvail()).y/2));
        ImGui::PlotLines("", fftdata, 512, 0, NULL, 0.0f, 100.0f, ImVec2(ImGui::GetContentRegionAvailWidth(),(ImGui::GetContentRegionAvail()).y));
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
    printf("Analysing\n");

    try {
//        std::ifstream audio;
//        audio.open(location);
//
//        if (!audio.is_open()) {
//            throw Exception(strerror(errno));
//        }
        SNDFILE *infile;
        SF_INFO sfinfo;
        // The SF_INFO struct must be initialized before using it.
        memset (&sfinfo, 0, sizeof (sfinfo)) ;

        if (! (infile = sf_open (location, SFM_READ, &sfinfo))) {
            throw Exception(sf_strerror (NULL));
        }

        if (sfinfo.channels != 1) {
            throw Exception("Please provide a file with 1 audio channel.");
        }
        if ((sfinfo.format & SF_FORMAT_PCM_16) == 0) {
            throw Exception("Please provide a file in signed 16-bit format");
        }

        sf_read_float(infile, data, 4096);
        dataExists = true;

        kiss_fft_cpx fft_in[4096];
        kiss_fft_cpx fft_out[4096];

        for (int i = 0; i < 4096; i ++) {
            fft_in[i] = { data[i], 0 };
        }

        kiss_fft_cfg cfg = kiss_fft_alloc( 1024 ,0 ,0,0 );
        kiss_fft( cfg , fft_in , fft_out );

        for (int i = 0; i < 1024; i++) {
            fftdata[i] = sqrt(pow(fft_out[i].r,2) + pow(fft_out[i].i, 2));
            std::cout << fftdata[i] << std::endl;
        }

        free(cfg);

//        double data[87];
//        int readcount;
//
//        while ((readcount = sf_read_double (infile, data, 87)))
//        {
//            std::cout << data[0] << std::endl;
//        } ;

        sf_close(infile);
    } catch (Exception &e) {
        ImGui::OpenPopup("Error");
        openError = e.getWhat();
    }
}

DebugWindow::Exception::Exception(const std::string &what) : what(what) {}

const std::string &DebugWindow::Exception::getWhat() const {
    return what;
}
