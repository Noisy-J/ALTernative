#include "BuildSystem.hpp"
#include "ECS/Scene.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

BuildSystem::BuildSystem(const Scene& scene)
    : m_Scene(scene) {
}

bool BuildSystem::buildProject(const std::string& outputDir,
    const std::string& projectName,
    const std::string& startScene) {
    m_Log.clear();
    m_LastError.clear();

    log("=== Alternative Engine Build ===");
    log("Project: " + projectName);
    log("Output: " + outputDir);
    log("Start scene: " + startScene);

    std::error_code ec;
    fs::create_directories(outputDir, ec);
    if (ec) {
        m_LastError = "Failed to create output directory: " + ec.message();
        return false;
    }

    fs::create_directories(outputDir + "/scenes", ec);
    fs::create_directories(outputDir + "/assets/textures", ec);

    if (!copyScenes(outputDir, startScene)) return false;
    if (!copyTextures(outputDir)) return false;
    if (!generateConfig(outputDir, startScene)) return false;
    if (!copyDlls(outputDir)) return false;
    if (!copyPlayer(outputDir, projectName)) return false;

    log("=== Build completed successfully ===");
    return true;
}

void BuildSystem::log(const std::string& message) {
    m_Log += message + "\n";
    std::cout << "[Build] " << message << std::endl;
}

// Копирует все сохранённые сцены в builds/ProjectName/scenes/
bool BuildSystem::copyScenes(const std::string& outputDir,
    const std::string& sceneName) {
    log("Copying scenes...");

    std::string scenesDir = "./scenes";
    std::string destDir = outputDir + "/scenes";

    if (fs::exists(scenesDir)) {
        for (const auto& entry : fs::directory_iterator(scenesDir)) {
            if (entry.path().extension() == ".alt_scene") {
                std::string from = entry.path().string();
                std::string to = destDir + "/" + entry.path().filename().string();
                if (!copyFile(from, to)) return false;
                log("  " + entry.path().filename().string());
            }
        }
    }

    log("Scenes copied.");
    return true;
}

// Копирует текстуры в builds/ProjectName/assets/textures/
bool BuildSystem::copyTextures(const std::string& outputDir) {
    log("Copying textures...");

    std::string texturesDir = "./assets/textures";
    std::string destDir = outputDir + "/assets/textures";

    if (fs::exists(texturesDir)) {
        int count = 0;
        for (const auto& entry : fs::directory_iterator(texturesDir)) {
            std::string ext = entry.path().extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
                std::string from = entry.path().string();
                std::string to = destDir + "/" + entry.path().filename().string();
                if (!copyFile(from, to)) return false;
                count++;
            }
        }
        log("  " + std::to_string(count) + " textures copied.");
    }

    return true;
}

// Копирует SFML DLL
bool BuildSystem::copyDlls(const std::string& outputDir) {
    log("Copying DLLs...");

    std::vector<std::string> dlls = {
        "sfml-graphics-3.dll",
        "sfml-window-3.dll",
        "sfml-system-3.dll"
    };

    // Ищем в нескольких местах
    std::vector<std::string> searchPaths = {
        "./",
        "../",
        "../AlternativeEngine/libs/SFML-3.0.2/bin/"
    };

    for (const auto& dll : dlls) {
        bool found = false;
        for (const auto& searchPath : searchPaths) {
            std::string from = searchPath + dll;
            if (fs::exists(from)) {
                std::string to = outputDir + "/" + dll;
                if (!copyFile(from, to)) return false;
                log("  " + dll);
                found = true;
                break;
            }
        }
        if (!found) {
            log("  WARNING: " + dll + " not found (skipping)");
        }
    }

    return true;
}

// Копирует и переименовывает AlternativeGame.exe
// Копирует и переименовывает AlternativeGame.exe
bool BuildSystem::copyPlayer(const std::string& outputDir,
    const std::string& projectName) {
    log("Copying game executable...");

    std::vector<std::string> playerPaths = {
        // Пути относительно той папки, откуда запущен AlternativeEngine
        "./AlternativeGame/AlternativeGame.exe",
        "./AlternativeGame.exe",
        // Дополнительные пути, куда CMake мог положить исполняемый файл
        "../AlternativeGame/AlternativeGame.exe",
        "../../AlternativeGame/AlternativeGame.exe",
        // Если рабочая папка = корень проекта, то путь до out/build/...
        "./out/build/x64-Debug/AlternativeGame/AlternativeGame.exe",
        // Если ничего не помогло, можно указать и так:
        "../out/build/x64-Debug/AlternativeGame/AlternativeGame.exe",
    };

    std::string playerPath;
    for (const auto& p : playerPaths) {
        if (fs::exists(p)) {
            playerPath = p;
            break;
        }
    }

    if (playerPath.empty()) {
        m_LastError = "AlternativeGame.exe not found! Build it first. Searched paths:\n";
        for (const auto& p : playerPaths) {
            m_LastError += "  " + p + "\n";
        }
        log("ERROR: " + m_LastError);
        return false;
    }

    std::string dest = outputDir + "/" + projectName + ".exe";
    log("  From: " + playerPath);
    log("  To: " + dest);

    return copyFile(playerPath, dest);
}

// Генерирует project.alt_config (аналог .lge_config, теперь .alt_config)
bool BuildSystem::generateConfig(const std::string& outputDir,
    const std::string& startScene) {
    log("Generating project config...");

    std::string configPath = outputDir + "/project.alt_config";
    std::ofstream config(configPath);
    if (!config.is_open()) {
        m_LastError = "Failed to create project.alt_config";
        return false;
    }

    config << "{\n";
    config << "    \"start_scene\": \"" << startScene << "\",\n";
    config << "    \"engine_version\": \"1.0\"\n";
    config << "}\n";
    config.close();

    log("  project.alt_config created");
    return true;
}

bool BuildSystem::copyDirectory(const std::string& from, const std::string& to) {
    try {
        if (!fs::exists(from)) return true;
        fs::copy(from, to,
            fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        return true;
    }
    catch (const std::exception& e) {
        m_LastError = "Failed to copy directory: " + std::string(e.what());
        return false;
    }
}

bool BuildSystem::copyFile(const std::string& from, const std::string& to) {
    try {
        std::error_code ec;
        fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            m_LastError = "Failed to copy " + from + ": " + ec.message();
            return false;
        }
        return true;
    }
    catch (const std::exception& e) {
        m_LastError = "Failed to copy " + from + ": " + std::string(e.what());
        return false;
    }
}