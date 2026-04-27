#include "ResourceManager.hpp"
#include <iostream>
#include <algorithm>
#include "stb_image.h"

namespace fs = std::filesystem;

// Статические переменные
std::unordered_map<std::string, std::shared_ptr<sf::Texture>> ResourceManager::s_Textures;
std::unordered_map<std::string, std::shared_ptr<sf::Font>> ResourceManager::s_Fonts;
std::string ResourceManager::s_BasePath;

// Вспомогательная функция
static std::string fixPath(const std::string& path) {
    std::string f = path;
    std::replace(f.begin(), f.end(), '\\', '/');
    return f;
}

void ResourceManager::setBasePath(const std::string& basePath) {
    s_BasePath = basePath;
    std::cout << "[RM] Base path set to: " << s_BasePath << std::endl;
}

std::string ResourceManager::getBasePath() {
    return s_BasePath;
}

std::string ResourceManager::resolvePath(const std::string& path) {
    if (path.empty()) return "";

    std::string fixed = fixPath(path);

    // Если путь абсолютный или файл существует — возвращаем как есть
    if (fs::path(fixed).is_absolute() || fs::exists(fixed)) {
        return fixed;
    }

    // Пробуем относительно базового пути
    if (!s_BasePath.empty()) {
        std::string relativePath = s_BasePath + "/" + fixed;
        if (fs::exists(relativePath)) {
            return relativePath;
        }
    }

    // Пробуем "assets/textures/" относительно базового пути
    if (!s_BasePath.empty()) {
        std::string assetPath = s_BasePath + "/assets/textures/" + fs::path(fixed).filename().string();
        if (fs::exists(assetPath)) {
            return assetPath;
        }
    }

    return fixed;
}

std::shared_ptr<sf::Texture> ResourceManager::loadTexture(const std::string& path) {
    if (path.empty()) return nullptr;

    std::string resolvedPath = resolvePath(path);
    std::string p = fixPath(resolvedPath);

    auto it = s_Textures.find(p);
    if (it != s_Textures.end()) return it->second;

    int w, h, c;
    unsigned char* data = stbi_load(p.c_str(), &w, &h, &c, 4);
    if (!data) {
        std::cerr << "[RM] stbi_load failed: " << p << std::endl;
        return nullptr;
    }

    auto tex = std::make_shared<sf::Texture>();
    if (!tex->resize({ (unsigned)w, (unsigned)h })) {
        stbi_image_free(data);
        return nullptr;
    }
    tex->update(data);
    stbi_image_free(data);

    s_Textures[p] = tex;
    std::cout << "[RM] Loaded texture: " << p << " (" << w << "x" << h << ")" << std::endl;
    return tex;
}

std::shared_ptr<sf::Font> ResourceManager::loadFont(const std::string& path) {
    if (path.empty()) return nullptr;

    std::string resolvedPath = resolvePath(path);
    std::string p = fixPath(resolvedPath);

    auto it = s_Fonts.find(p);
    if (it != s_Fonts.end()) return it->second;

    auto f = std::make_shared<sf::Font>();
    if (!f->openFromFile(p)) {
        std::cerr << "[RM] Failed to load font: " << p << std::endl;
        return nullptr;
    }

    s_Fonts[p] = f;
    std::cout << "[RM] Loaded font: " << p << std::endl;
    return f;
}

void ResourceManager::clearCache() {
    s_Textures.clear();
    s_Fonts.clear();
}

void ResourceManager::removeTexture(const std::string& path) {
    s_Textures.erase(fixPath(path));
}