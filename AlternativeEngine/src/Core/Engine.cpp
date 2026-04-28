#include "Engine.hpp"
#include <iostream>
#include <cmath>

#include "ECS/Systems/MovementSystem.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "Resources/ResourceManager.hpp"
#include "Design/StyleManager.hpp"
#include "Scripting/ScriptRuntime.hpp"

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
    m_EditorUI = std::make_unique<EditorUI>(m_Window, m_Scene, *m_Viewport, *this);
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

void Engine::executeScriptEvents(bool beginPlayOnly) {
    for (auto& [entity, script] : m_Scene.scripts) {
        if (script.scriptPath.empty()) continue;

        // Загружаем ноды и связи из JSON
        std::vector<Scripting::Node> nodes;
        std::vector<Scripting::Link> links;

        if (parseScriptGraph(script.scriptPath, nodes, links)) {
            ScriptRuntime runtime(m_Scene);

            if (beginPlayOnly) {
                runtime.executeBeginPlay(nodes, links, entity);
            }
            else {
                runtime.executeTick(nodes, links, entity);
            }
        }
    }
}


void Engine::update(float deltaTime) {
    if (m_IsPlaying && !m_IsPaused) {
        executeScriptEvents(false);  // false = только Tick
    }

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

void Engine::play() {
    m_IsPlaying = true;
    m_IsPaused = false;
    m_BeginPlayExecuted = false;  // Сбрасываем при каждом новом запуске
    std::cout << "[Engine] Play mode started" << std::endl;

    // Выполняем BeginPlay один раз
    executeScriptEvents(true);  // true = только BeginPlay
}

void Engine::stop() {
    m_IsPlaying = false;
    m_IsPaused = false;
    std::cout << "[Engine] Play mode stopped" << std::endl;
}

void Engine::pause() {
    if (m_IsPlaying) {
        m_IsPaused = !m_IsPaused;
        std::cout << "[Engine] " << (m_IsPaused ? "Paused" : "Resumed") << std::endl;
    }
}



bool Engine::parseScriptGraph(const std::string& json,
    std::vector<Scripting::Node>& nodes,
    std::vector<Scripting::Link>& links) {
    // Простой парсер JSON для нашего формата
    // Формат: {"nodes":[{"id":0,"type":"EventTick","x":100,"y":300,"pins":{"1":"0","2":"0.000000,0.000000"}}],"links":[{"id":0,"sn":1,"sp":0,"en":2,"ep":0}]}

    try {
        // Парсим nodes
        size_t nodesStart = json.find("\"nodes\":[");
        if (nodesStart == std::string::npos) return false;

        nodesStart += 9; // длина "\"nodes\":["

        size_t depth = 0;
        for (size_t i = nodesStart; i < json.length(); i++) {
            if (json[i] == '{') {
                // Начало ноды
                size_t nodeEnd = json.find("}", i);
                if (nodeEnd == std::string::npos) break;

                std::string nodeJson = json.substr(i, nodeEnd - i + 1);
                i = nodeEnd;

                Scripting::Node node;

                // ID
                size_t idPos = nodeJson.find("\"id\":");
                if (idPos != std::string::npos) {
                    node.Id = std::stoi(nodeJson.substr(idPos + 5));
                }

                // Type
                size_t typePos = nodeJson.find("\"type\":\"");
                if (typePos != std::string::npos) {
                    size_t typeEnd = nodeJson.find("\"", typePos + 8);
                    node.Type = nodeJson.substr(typePos + 8, typeEnd - typePos - 8);
                }

                // X, Y
                size_t xPos = nodeJson.find("\"x\":");
                size_t yPos = nodeJson.find("\"y\":");
                if (xPos != std::string::npos) node.PosX = std::stof(nodeJson.substr(xPos + 4));
                if (yPos != std::string::npos) node.PosY = std::stof(nodeJson.substr(yPos + 4));

                // Создаём ноду через NodeRegistry
                Scripting::Node templateNode = Scripting::NodeRegistry::Instance().CreateNode(node.Type);
                templateNode.Id = node.Id;
                templateNode.PosX = node.PosX;
                templateNode.PosY = node.PosY;

                // Загружаем значения пинов
                size_t pinsPos = nodeJson.find("\"pins\":{");
                if (pinsPos != std::string::npos) {
                    size_t pinsEnd = nodeJson.find("}", pinsPos + 7);
                    std::string pinsJson = nodeJson.substr(pinsPos + 8, pinsEnd - pinsPos - 8);

                    // Парсим пары "pinId":"value"
                    size_t pinStart = 0;
                    while ((pinStart = pinsJson.find("\"", pinStart)) != std::string::npos) {
                        size_t pinIdEnd = pinsJson.find("\"", pinStart + 1);
                        std::string pinIdStr = pinsJson.substr(pinStart + 1, pinIdEnd - pinStart - 1);

                        size_t valStart = pinsJson.find("\"", pinIdEnd + 2);
                        size_t valEnd = pinsJson.find("\"", valStart + 1);
                        std::string value = pinsJson.substr(valStart + 1, valEnd - valStart - 1);

                        int pinId = std::stoi(pinIdStr);
                        for (auto& pin : templateNode.InputPins) {
                            if (pin.Id == pinId) {
                                pin.DefaultValue = value;
                                break;
                            }
                        }

                        pinStart = valEnd + 1;
                    }
                }

                nodes.push_back(templateNode);
            }
        }

        // Парсим links
        size_t linksStart = json.find("\"links\":[");
        if (linksStart != std::string::npos) {
            linksStart += 9;

            for (size_t i = linksStart; i < json.length(); i++) {
                if (json[i] == '{') {
                    size_t linkEnd = json.find("}", i);
                    if (linkEnd == std::string::npos) break;

                    std::string linkJson = json.substr(i, linkEnd - i + 1);
                    i = linkEnd;

                    Scripting::Link link;

                    size_t idPos = linkJson.find("\"id\":");
                    size_t snPos = linkJson.find("\"sn\":");
                    size_t spPos = linkJson.find("\"sp\":");
                    size_t enPos = linkJson.find("\"en\":");
                    size_t epPos = linkJson.find("\"ep\":");

                    if (idPos != std::string::npos) link.Id = std::stoi(linkJson.substr(idPos + 5));
                    if (snPos != std::string::npos) link.StartNodeId = std::stoi(linkJson.substr(snPos + 5));
                    if (spPos != std::string::npos) link.StartPinId = std::stoi(linkJson.substr(spPos + 5));
                    if (enPos != std::string::npos) link.EndNodeId = std::stoi(linkJson.substr(enPos + 5));
                    if (epPos != std::string::npos) link.EndPinId = std::stoi(linkJson.substr(epPos + 5));

                    links.push_back(link);
                }
            }
        }

        return true;
    }
    catch (...) {
        std::cerr << "[Engine] Failed to parse script graph" << std::endl;
        return false;
    }
}