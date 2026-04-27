#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include "ECS/Scene.hpp"
#include "Rendering/Viewport.hpp"
#include "Serialization/SceneSerializer.hpp"
#include "ECS/Systems/RenderSystem.hpp"

class GamePlayer {
public:
    GamePlayer(const std::string& projectPath);
    virtual ~GamePlayer();  // виртуальный — для наследования редактором

    bool initialize();
    void run();

protected:
    std::string m_ProjectPath;
    std::string m_StartScene;

    sf::RenderWindow m_Window;
    Scene m_Scene;
    std::unique_ptr<Viewport> m_Viewport;
    std::unique_ptr<SceneSerializer> m_Serializer;
    sf::Clock m_Clock;

    // Хуки для наследников (EditorPlayer)
    virtual void onInit() {}
    virtual void onShutdown() {}
    virtual void onUpdate(float deltaTime) {}
    virtual void onRender() {}
    virtual void onProcessInput() {}

    bool loadProjectConfig();
    virtual void handleInput();
    virtual void update(float deltaTime);
    virtual void render();
};