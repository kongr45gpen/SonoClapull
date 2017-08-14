#include <imgui.h>
#include "ProcessingWindow.h"

void ProcessingWindow::draw() {
    ImGui::SetNextWindowSize(ImVec2(820, 700), ImGuiSetCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0, 0.0, 0.01, 0.97 });
    ImGui::Begin("Processing Queue");

    ImGui::BeginChild("QueueQueue", ImVec2(0,ImGui::GetContentRegionAvail().y / 2.0f), false);

    ImGui::Columns(4, "filequeue"); // 4-ways, with border
    ImGui::SetColumnOffset(1, ImGui::GetWindowContentRegionWidth() * 0.05f);
    ImGui::SetColumnOffset(2, ImGui::GetWindowContentRegionWidth() * 0.55f);
    ImGui::SetColumnOffset(3, ImGui::GetWindowContentRegionWidth() * 0.75f);

    ImGui::Separator();
    ImGui::Text("No.");
    ImGui::NextColumn();
    ImGui::Text("Name");
    ImGui::NextColumn();
    ImGui::Text("Status");
    ImGui::NextColumn();
    ImGui::Text("Progress");
    ImGui::NextColumn();
    ImGui::Separator();
    static int selected = -1;
    int i = 0;
    for (Processing::File &file : processing->getFiles()) {
        if (ImGui::Selectable(std::to_string(i + 1).c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns)) {
            selected = i;
        }
        i++;
        ImGui::NextColumn();
        ImGui::Text("%s", file.getName().c_str());
        ImGui::NextColumn();
        showStatus(file.getStatus());
        ImGui::NextColumn();

        // Sets the progress bar colour
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, statusColour(file.getStatus()));
        ImGui::ProgressBar(file.getProgress(), {
                ImGui::GetContentRegionAvailWidth() / 2.5f,
                16 });
        ImGui::SameLine();
        ImGui::ProgressBar(file.getProgress(), {
                ImGui::GetContentRegionAvailWidth(),
                16 });
        ImGui::PopStyleColor();

        ImGui::NextColumn();

    }
    ImGui::Columns(1);
    ImGui::EndChild();

    ImGui::BeginChild("QueueSelection", {0, 0}, true);
    ImGui::Text("Selected file");
    ImGui::Separator();
    if (selected != -1) {
        Processing::File &file = (processing->getFiles())[selected];

        int column1 = 150;

        ImGui::Text("Path:");
        ImGui::SameLine(column1);
        ImGui::Text("%s", file.getDirectoryEntry().path().c_str());

        ImGui::Text("Extension:");
        ImGui::SameLine(column1);
        ImGui::Text("%s", file.getDirectoryEntry().path().extension().c_str());

        ImGui::Spacing();
        ImGui::Text("Status:");
        ImGui::SameLine(column1);
        showStatus(file.getStatus());

        ImGui::Text("Progress:");
        ImGui::SameLine(column1);
        ImGui::Text("%.1f%%", file.getProgress());
    }
    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleColor();
}

ProcessingWindow::ProcessingWindow(const std::shared_ptr<Processing> &processing) : processing(processing) {}

void ProcessingWindow::showStatus(const Processing::File::Status &status) {
    ImVec4 colour = statusColour(status);
    switch (status) {
        case Processing::File::STATUS_QUEUED:
            ImGui::TextColored(colour, "Queued");
            break;
        case Processing::File::STATUS_WAITING:
            ImGui::TextColored(colour, "Waiting...");
            break;
        case Processing::File::STATUS_DEMUXING:
            ImGui::TextColored(colour, "Demuxing");
            break;
        case Processing::File::STATUS_SEARCHING_CLOCK:
            ImGui::TextColored(colour, "Clock");
            break;
        case Processing::File::STATUS_DECODING_DATA:
            ImGui::TextColored(colour, "Sampling Data");
            break;
        case Processing::File::STATUS_DONE:
            ImGui::TextColored(colour, "Done");
            break;
        case Processing::File::STATUS_ERROR:
            ImGui::TextColored(colour, "Error!");
            break;
        default:
            ImGui::Text("Unknown");
            break;
    }
}

ImVec4 ProcessingWindow::statusColour(const Processing::File::Status &status) {
    switch (status) {
        case Processing::File::STATUS_QUEUED:
            return {0.4, 0.4, 0.4, 0.9};
        case Processing::File::STATUS_WAITING:
            return {0.55, 0.55, 0.45, 1.0};
        case Processing::File::STATUS_DEMUXING:
            return {0.7, 0.6, 0.3, 1.0};
        case Processing::File::STATUS_SEARCHING_CLOCK:
            return {0.8, 0.8, 0.1, 1.0};
        case Processing::File::STATUS_DECODING_DATA:
            return {0.85, 0.75, 0.1, 1.0};
        case Processing::File::STATUS_DONE:
            return {0.0, 0.8, 0.05, 1.0};
        case Processing::File::STATUS_ERROR:
            return {0.8, 0.1, 0.1, 1.0};
        default:
            return {1, 1, 1, 1};
    }
}
