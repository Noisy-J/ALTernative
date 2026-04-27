#include "Engine.hpp"
#include <iostream>
#include <cmath>

#include "ECS/Systems/MovementSystem.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "Resources/ResourceManager.hpp"
#include "Design/StyleManager.hpp"

Engine::Engine() {
    initializeWindow();
    initializeImGui();
    initializeViewport();
    initializeSubsystems();
}

void Engine::initializeWindow() {
    auto desktop = sf::VideoMode::getDesktopMode();
    m_Window.create(desktop, "ALTernative Engine v1.1.0b", sf::Style::None);
    m_Window.setFramerateLimit(60);
}

void Engine::initializeImGui() {
    if (!ImGui::SFML::Init(m_Window)) {
        throw std::runtime_error("Failed to initialize ImGui");
    }
    StyleManager::getInstance().applyStyle(StyleManager::Preset::ModernDark);
}

void Engine::initializeViewport() {
    m_Viewport = std::make_unique<Viewport>(800, 600);

    sf::View view = m_Viewport->getView();
    auto size = m_Viewport->getSize();
    view.setCenter({ static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f });
    view.setSize({ static_cast<float>(size.x), -static_cast<float>(size.y) });
    m_Viewport->setView(view);
}

void Engine::initializeSubsystems() {
    m_InputManager = std::make_unique<InputManager>(m_Window, *m_Viewport);
    m_EditorUI = std::make_unique<EditorUI>(m_Window, m_Scene, *m_Viewport);
    m_TextureBrowser = std::make_unique<TextureBrowser>(m_Scene);
}

void Engine::initializeDefaultEntities() {
    // Пока пусто
}

void Engine::run() {
    initializeDefaultEntities();

    while (m_Window.isOpen()) {
        processFrame();
    }

    ImGui::SFML::Shutdown();
}

void Engine::processFrame() {
    sf::Time dt = m_Clock.restart();
    float deltaTime = dt.asSeconds();

    handleInput();
    update(deltaTime);
    render();
}

void Engine::handleInput() {
    m_InputManager->processInput(m_Scene, m_Player);
}

void Engine::update(float deltaTime) {
    MovementSystem::update(m_Scene, deltaTime);
    ImGui::SFML::Update(m_Window, sf::seconds(deltaTime));

    Entity selectedEntity = m_InputManager->getSelectedEntity();
    m_EditorUI->update(selectedEntity, deltaTime);

    if (m_InputManager->isContextMenuOpen()) {
        sf::Vector2f menuPos = m_InputManager->getContextMenuPos();
        m_EditorUI->showContextMenu(menuPos);
        m_InputManager->closeContextMenu();
    }
}

void Engine::render() {
    m_Window.clear(sf::Color(40, 40, 40));
    m_Viewport->clear(sf::Color(100, 100, 100));

    RenderSystem::drawSprites(m_Scene, m_Viewport->getRenderTexture());

    Entity selectedEntity = m_InputManager->getSelectedEntity();
    m_Viewport->renderDebugOverlays(m_Scene, selectedEntity, m_InputManager->getMouseWorldPos());

    m_Viewport->display();
    m_EditorUI->render();

    ImGui::SFML::Render(m_Window);
    m_Window.display();
}