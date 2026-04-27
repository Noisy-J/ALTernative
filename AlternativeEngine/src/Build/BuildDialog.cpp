#include "BuildDialog.hpp"
#include <imgui.h>
#include <filesystem>

namespace fs = std::filesystem;

BuildDialog::BuildDialog(const Scene& scene)
    : m_Scene(scene) {
}

void BuildDialog::open() {
    m_Open = true;
    m_BuildInProgress = false;
    m_BuildLog.clear();
    m_ErrorMessage.clear();
}

void BuildDialog::render() {
    if (!m_Open) return;

    ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Build Project", &m_Open)) {

        ImGui::Text("Project Name:");
        ImGui::InputText("##BuildName", m_ProjectName, sizeof(m_ProjectName));

        ImGui::Text("Output Path:");
        ImGui::InputText("##BuildPath", m_OutputPath, sizeof(m_OutputPath));
        ImGui::SameLine();
        if (ImGui::Button("...")) {
            // В будущем — диалог выбора папки
            std::string dir = "./builds/" + std::string(m_ProjectName);
            strncpy_s(m_OutputPath, sizeof(m_OutputPath), dir.c_str(), dir.length());
        }

        ImGui::Separator();

        if (!m_BuildInProgress) {
            if (ImGui::Button("Build", ImVec2(120, 30))) {
                runBuild();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 30))) {
                m_Open = false;
            }
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Building...");
        }

        ImGui::Separator();

        // Лог
        if (!m_ErrorMessage.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "ERROR:");
            ImGui::TextWrapped("%s", m_ErrorMessage.c_str());
            ImGui::Separator();
        }

        ImGui::Text("Build Log:");
        ImGui::BeginChild("BuildLog", ImVec2(0, 0), true);
        ImGui::TextUnformatted(m_BuildLog.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void BuildDialog::runBuild() {
    m_BuildInProgress = true;
    m_BuildLog.clear();
    m_ErrorMessage.clear();

    BuildSystem builder(m_Scene);

    std::string projectName(m_ProjectName);
    std::string outputPath(m_OutputPath);

    // Определяем стартовую сцену (текущую открытую)
    std::string startScene = m_Scene.sceneName;
    if (startScene.empty()) {
        startScene = "main";
    }

    if (builder.buildProject(outputPath, projectName, startScene)) {
        m_BuildLog = builder.getLog();
        m_BuildLog += "\n\n=== Build successful! ===\n";
        m_BuildLog += "Output: " + outputPath + "/" + projectName + ".exe\n";
    }
    else {
        m_BuildLog = builder.getLog();
        m_ErrorMessage = builder.getLastError();
    }

    m_BuildInProgress = false;
}