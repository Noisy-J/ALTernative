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
#include "Scripting/ScriptRuntime.hpp"

class Engine {
public:
    Engine();
    ~Engine() = default;

    void run();

    bool isPlaying() const { return m_IsPlaying; }
    bool isPaused() const { return m_IsPaused; }

    void play();
    void stop();
    void pause();

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

    bool m_IsPlaying = false;
    bool m_IsPaused = false;
    bool m_BeginPlayExecuted = false;

    void executeScriptEvents(bool beginPlayOnly);
    bool parseScriptGraph(const std::string& json,
        std::vector<Scripting::Node>& nodes,
        std::vector<Scripting::Link>& links);

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