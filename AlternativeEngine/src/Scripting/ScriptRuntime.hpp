#pragma once

#include "ECS/Scene.hpp"
#include "Scripting/NodeSystem.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

class ScriptRuntime {
public:
    ScriptRuntime(Scene& scene) : m_Scene(scene) {}

    // Выполнить все EventBeginPlay (один раз при старте)
    void executeBeginPlay(const std::vector<Scripting::Node>& nodes,
        const std::vector<Scripting::Link>& links,
        Entity ownerEntity) {
        m_Nodes = nodes;
        m_Links = links;
        m_OwnerEntity = ownerEntity;

        for (auto& node : m_Nodes) {
            if (node.Type == "EventBeginPlay") {
                executeNodeChain(node);
            }
        }
    }

    // Выполнить все EventTick (каждый кадр)
    void executeTick(const std::vector<Scripting::Node>& nodes,
        const std::vector<Scripting::Link>& links,
        Entity ownerEntity) {
        m_Nodes = nodes;
        m_Links = links;
        m_OwnerEntity = ownerEntity;

        for (auto& node : m_Nodes) {
            if (node.Type == "EventTick") {
                executeNodeChain(node);
            }
        }
    }

    // Выполнить всё (для кнопки Execute в редакторе)
    void executeAll(const std::vector<Scripting::Node>& nodes,
        const std::vector<Scripting::Link>& links,
        Entity ownerEntity) {
        executeBeginPlay(nodes, links, ownerEntity);
        executeTick(nodes, links, ownerEntity);
    }

private:
    Scene& m_Scene;
    Entity m_OwnerEntity;
    std::vector<Scripting::Node> m_Nodes;
    std::vector<Scripting::Link> m_Links;

    void executeNodeChain(Scripting::Node& node) {
        // SetPosition
        if (node.Type == "SetPosition") {
            Entity target = m_OwnerEntity;
            sf::Vector2f pos = { 0, 0 };

            for (auto& pin : node.InputPins) {
                if (pin.Name == "Target" && !pin.DefaultValue.empty()) {
                    target = static_cast<Entity>(std::stoi(pin.DefaultValue));
                }
                if (pin.Name == "Position" && !pin.DefaultValue.empty()) {
                    std::string val = pin.DefaultValue;
                    size_t comma = val.find(',');
                    if (comma != std::string::npos) {
                        pos.x = std::stof(val.substr(0, comma));
                        pos.y = std::stof(val.substr(comma + 1));
                    }
                }
            }

            if (m_Scene.isValid(target)) {
                m_Scene.setPosition(target, pos);
            }
        }

        // Print
        else if (node.Type == "Print") {
            std::string msg = "Print";
            for (auto& pin : node.InputPins) {
                if (pin.Type == Scripting::PinType::String && !pin.DefaultValue.empty()) {
                    msg = pin.DefaultValue;
                }
            }
            std::cout << "[Script Print] " << msg << std::endl;
        }

        // GetPosition
        else if (node.Type == "GetPosition") {
            Entity target = m_OwnerEntity;
            for (auto& pin : node.InputPins) {
                if (pin.Name == "Target" && !pin.DefaultValue.empty()) {
                    target = static_cast<Entity>(std::stoi(pin.DefaultValue));
                }
            }

            if (m_Scene.isValid(target)) {
                sf::Vector2f pos = m_Scene.getPosition(target);
                // Записываем позицию в выходной пин
                for (auto& pin : node.OutputPins) {
                    if (pin.Name == "Position") {
                        pin.DefaultValue = std::to_string(pos.x) + "," + std::to_string(pos.y);
                    }
                }
            }
        }

        // Переход к следующим нодам по Exec
        for (auto& outPin : node.OutputPins) {
            if (outPin.Type == Scripting::PinType::Flow) {
                for (auto& link : m_Links) {
                    if (link.StartNodeId == node.Id && link.StartPinId == outPin.Id) {
                        for (auto& nextNode : m_Nodes) {
                            if (nextNode.Id == link.EndNodeId) {
                                executeNodeChain(nextNode);
                            }
                        }
                    }
                }
            }
        }
    }
};