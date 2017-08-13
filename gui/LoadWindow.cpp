#include <imgui.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include "LoadWindow.h"

using namespace boost;

LoadWindow::LoadWindow() {
    // ???
}

void LoadWindow::draw() {
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Load files");

    ImGui::Text("Load all files from path:");
    ImGui::PushID(0);
    ImGui::InputText("", path, 4096);
    ImGui::PopID();

    ImGui::Text("Extension filter: \t *.");
    ImGui::SameLine();
    ImGui::PushItemWidth(70);
    ImGui::PushID(1);
    ImGui::InputText("", extension, EXT_MAX);
    ImGui::PopID();
    ImGui::PopItemWidth();

    ImGui::Checkbox("Recursive search", &recurse);

    ImGui::Spacing();

    if (ImGui::Button("Load", {ImGui::GetContentRegionAvailWidth(), 0})) {
        // No need to load in a separate thread
        performLoad();
    };

    if (lastLoad >= 0) {
        ImGui::Text("%d files loaded", lastLoad);
    }

    ImGui::End();
}

void LoadWindow::performLoad() {
    std::vector<filesystem::directory_entry> files;

    try {
        filesystem::path directory(path);

        // TODO: Implement recurse

        for (filesystem::directory_entry &entry : filesystem::directory_iterator(directory)) {
            if (!filesystem::is_regular_file(entry)) {
                continue;
            }

            std::string thisExtension = entry.path().extension().string();
            thisExtension.erase(0, 1); // Remove the first dot (.mp3)
            if (strlen(extension) != 0 && thisExtension != extension) {
                continue;
            }

            files.push_back(entry);
        }
    } catch (filesystem::filesystem_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::sort(files.begin(), files.end(),
    [] (const filesystem::directory_entry& a, const filesystem::directory_entry &b) {
        return a.path() < b.path();
    });

    for (auto &entry : files) {
        std::cout << entry.path() << std::endl;
    }

    lastLoad = files.size();
}
