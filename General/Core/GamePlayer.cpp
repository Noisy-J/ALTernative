#include "GamePlayer.hpp"
#include "Resources/ResourceManager.hpp"
#include "ECS/Systems/MovementSystem.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

GamePlayer::GamePlayer(const std::string& projectPath)
    : m_ProjectPath(projectPath) {
}

GamePlayer::~GamePlayer() {
    onShutdown();  // вызов хука, а не ImGui::SFML напрямую
}

bool GamePlayer::initialize() {
    if (!loadProjectConfig()) {
        std::cerr << "[Player] Failed to load project config" << std::endl;
        return false;
    }

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    m_Window.create(sf::VideoMode({ 1280, 720 }),
        "ALTernative Player - " + m_StartScene,
        sf::Style::Default);
    m_Window.setFramerateLimit(60);

    m_Viewport = std::make_unique<Viewport>(1280, 720);
    sf::View view = m_Viewport->getView();
    auto size = m_Viewport->getSize();
    view.setCenter({ static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f });
    view.setSize({ static_cast<float>(size.x), -static_cast<float>(size.y) });
    m_Viewport->setView(view);

    m_Serializer = std::make_unique<SceneSerializer>(m_Scene);

    fs::path scenePath = fs::path(m_ProjectPath) / "scenes" / (m_StartScene + ".alt_scene");
    if (!m_Serializer->load(scenePath.string())) {
        std::cerr << "[Player] Failed to load scene: " << scenePath << std::endl;
        return false;
    }

    std::cout << "[Player] Initialized successfully" << std::endl;
    std::cout << "[Player] Scene: " << m_Scene.sceneName << std::endl;
    std::cout << "[Player] Entities: " << m_Scene.transforms.size() << std::endl;

    onInit();  // хук для наследников (EditorPlayer добавит ImGui::SFML::Init)
    return true;
}

void GamePlayer::run() {
    while (m_Window.isOpen()) {
        sf::Time dt = m_Clock.restart();
        float deltaTime = dt.asSeconds();

        onProcessInput();  // хук
        handleInput();

        onUpdate(deltaTime);  // хук
        update(deltaTime);

        onRender();  // хук
        render();
    }
}

bool GamePlayer::loadProjectConfig() {
    fs::path configPath = fs::path(m_ProjectPath) / "project.alt_config";

    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "[Player] Config not found: " << configPath << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();

    size_t scenePos = json.find("\"start_scene\":");
    if (scenePos != std::string::npos) {
        size_t start = json.find("\"", scenePos + 14) + 1;
        size_t end = json.find("\"", start);
        if (start != std::string::npos && end != std::string::npos) {
            m_StartScene = json.substr(start, end - start);
            return true;
        }
    }

    return false;
}

void GamePlayer::handleInput() {
    while (const auto event = m_Window.pollEvent()) {
        if (event->is<sf::Event::Closed>() ||
            (event->is<sf::Event::KeyPressed>() &&
                event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            m_Window.close();
        }
    }
}

void GamePlayer::update(float deltaTime) {
    MovementSystem::update(m_Scene, deltaTime);
}

void GamePlayer::render() {
    m_Window.clear(sf::Color(40, 40, 40));
    m_Viewport->clear(sf::Color(100, 100, 100));

    RenderSystem::drawSprites(m_Scene, m_Viewport->getRenderTexture());

    m_Viewport->display();

    sf::Sprite viewportSprite(m_Viewport->getTexture());
    sf::Vector2u windowSize = m_Window.getSize();
    sf::Vector2u textureSize = m_Viewport->getSize();

    float scaleX = static_cast<float>(windowSize.x) / textureSize.x;
    float scaleY = static_cast<float>(windowSize.y) / textureSize.y;
    viewportSprite.setScale({ scaleX, scaleY });

    m_Window.draw(viewportSprite);
    m_Window.display();
}