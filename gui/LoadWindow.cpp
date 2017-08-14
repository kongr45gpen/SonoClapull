#include <imgui.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include "LoadWindow.h"

using namespace boost;

LoadWindow::LoadWindow(const std::shared_ptr<Processing> &processing) : processing(processing) {}

void LoadWindow::draw() {
    ImGui::SetNextWindowSize(ImVec2(820, 300), ImGuiSetCond_Always);
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

    ImGui::Checkbox("Search in subdirectories", &recurse);

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

        auto addEntry = [&files, this] (filesystem::directory_entry &entry) {
            try {
                if (!filesystem::is_regular_file(entry)) {
                    return;
                }

                std::string thisExtension = entry.path().extension().string();
                thisExtension.erase(0, 1); // Remove the first dot (.mp3)
                if (strlen(extension) != 0 && thisExtension != extension) {
                    return;
                }

                files.push_back(entry);
            } catch (filesystem::filesystem_error &e) {
                // Don't prevent other files from being read
                std::cerr << "Error while accessing " << entry.path() << ": " << e.what() << std::endl;
            }
        };

        if (recurse) {
            for (filesystem::directory_entry &entry : filesystem::recursive_directory_iterator(path)) {
                addEntry(entry);
            }
        } else {
            for (filesystem::directory_entry &entry : filesystem::directory_iterator(path)) {
                addEntry(entry);
            }
        }


    } catch (filesystem::filesystem_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // Sort files alphabetically
    // TODO: Numeric sort
    std::sort(files.begin(), files.end(),
    [] (const filesystem::directory_entry& a, const filesystem::directory_entry &b) {
        return a.path() < b.path();
    });

    for (auto &entry : files) {
        processing->addFile(entry);
    }

    lastLoad = files.size();
}
