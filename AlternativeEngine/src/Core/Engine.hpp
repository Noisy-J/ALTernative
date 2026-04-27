#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "imgui.h"
#include "imgui-SFML.h"

#include "ECS/Scene.hpp"
#include "Rendering/Viewport.hpp"
#include "Input/InputManager.hpp"
#include "Editor/EditorUI.hpp"
#include "Resources/TextureBrowser.hpp"

class Engine {
public:
    Engine();
    ~Engine() = default;

    void run();

private:
    sf::RenderWindow m_Window;
    sf::Clock m_Clock;

    Scene m_Scene;
    std::unique_ptr<Viewport> m_Viewport;

    std::unique_ptr<InputManager> m_InputManager;
    std::unique_ptr<EditorUI> m_EditorUI;
    std::unique_ptr<TextureBrowser> m_TextureBrowser;

    std::shared_ptr<sf::Texture> m_DefaultTexture;
    Entity m_Player;

    void initializeWindow();
    void initializeImGui();
    void initializeViewport();
    void initializeSubsystems();
    void initializeDefaultEntities();

    void processFrame();
    void handleInput();
    void update(float deltaTime);
    void render();
};