#include "ResourceManager.hpp"
#include <iostream>
#include <algorithm>
#include "stb_image.h"

std::unordered_map<std::string, std::shared_ptr<sf::Texture>> ResourceManager::s_Textures;
std::unordered_map<std::string, std::shared_ptr<sf::Font>> ResourceManager::s_Fonts;

static std::string fixPath(const std::string& path) {
    std::string f = path;
    std::replace(f.begin(), f.end(), '\\', '/');
    return f;
}

std::shared_ptr<sf::Texture> ResourceManager::loadTexture(const std::string& path) {
    if (path.empty()) return nullptr;
    std::string p = fixPath(path);
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
    return tex;
}

std::shared_ptr<sf::Font> ResourceManager::loadFont(const std::string& path) {
    if (path.empty()) return nullptr;
    std::string p = fixPath(path);
    auto it = s_Fonts.find(p);
    if (it != s_Fonts.end()) return it->second;
    auto f = std::make_shared<sf::Font>();
    if (!f->openFromFile(p)) return nullptr;
    s_Fonts[p] = f;
    return f;
}

void ResourceManager::clearCache() { s_Textures.clear(); s_Fonts.clear(); }
void ResourceManager::removeTexture(const std::string& path) { s_Textures.erase(fixPath(path)); }