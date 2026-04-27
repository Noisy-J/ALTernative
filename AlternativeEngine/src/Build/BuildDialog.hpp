#pragma once

#include <string>
#include <functional>
#include <imgui.h>
#include "ECS/Scene.hpp"
#include "BuildSystem.hpp"

class BuildDialog {
public:
    BuildDialog(const Scene& scene);

    void open();
    void render();

private:
    const Scene& m_Scene;

    bool m_Open = false;
    bool m_BuildInProgress = false;

    char m_ProjectName[128] = "MyProject";
    char m_OutputPath[256] = "./builds/MyProject";

    std::string m_BuildLog;
    std::string m_ErrorMessage;

    void runBuild();
};