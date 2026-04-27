#pragma once

#include <string>

class Scene;

class BuildSystem {
public:
    BuildSystem(const Scene& scene);

    bool buildProject(const std::string& outputDir,
        const std::string& projectName,
        const std::string& startScene);

    std::string getLastError() const { return m_LastError; }
    std::string getLog() const { return m_Log; }

private:
    const Scene& m_Scene;
    std::string m_LastError;
    std::string m_Log;

    void log(const std::string& message);
    bool copyScenes(const std::string& outputDir, const std::string& sceneName);
    bool copyTextures(const std::string& outputDir);
    bool copyDlls(const std::string& outputDir);
    bool copyPlayer(const std::string& outputDir, const std::string& projectName);
    bool generateConfig(const std::string& outputDir, const std::string& startScene);
    bool copyDirectory(const std::string& from, const std::string& to);
    bool copyFile(const std::string& from, const std::string& to);
};