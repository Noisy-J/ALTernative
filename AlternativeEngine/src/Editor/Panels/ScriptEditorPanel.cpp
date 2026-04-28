#include "ScriptEditorPanel.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include <set>
#include <cstdlib>

ScriptEditorPanel::ScriptEditorPanel(Scene& scene) : m_Scene(scene) {}

void ScriptEditorPanel::openForEntity(Entity entity) {
    m_IsOpen = true;
    m_CurrentEntity = entity;

    if (m_Scene.scripts.contains(entity) && !m_Scene.scripts[entity].scriptPath.empty()) {
        loadGraph();
    }
    else {
        m_Nodes.clear();
        m_Links.clear();
        m_NextNodeId = 0;
        m_NextLinkId = 0;
        m_SelectedNodeId = -1;
        addNode("EventBeginPlay", ImVec2(100, 100));
        addNode("EventTick", ImVec2(100, 300));
    }
}

void ScriptEditorPanel::render() {
    if (!m_IsOpen) return;

    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);

    std::string title = "Script Editor - " + m_Scene.getEntityName(m_CurrentEntity);
    if (ImGui::Begin(title.c_str(), &m_IsOpen)) {
        renderToolbar();
        ImGui::Separator();

        ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetWindowWidth() - 300, 0), false);
        renderCanvas();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPanel", ImVec2(300, 0), true);
        renderNodeProperties();
        ImGui::EndChild();
    }
    ImGui::End();
}

void ScriptEditorPanel::renderToolbar() {
    if (ImGui::Button("Save")) saveGraph();
    ImGui::SameLine();
    if (ImGui::Button("Execute")) executeGraph();
    ImGui::SameLine();

    // Поиск нод
    ImGui::SetNextItemWidth(200);
    if (ImGui::InputText("##SearchNodes", m_SearchBuffer, sizeof(m_SearchBuffer))) {
        m_ShowSearchResults = (strlen(m_SearchBuffer) > 0);
    }

    if (m_ShowSearchResults && strlen(m_SearchBuffer) > 0) {
        ImGui::SameLine();
        if (ImGui::BeginCombo("##SearchResults", "Results...")) {
            auto nodes = Scripting::NodeRegistry::Instance().GetAvailableNodes();
            std::string search = m_SearchBuffer;
            std::transform(search.begin(), search.end(), search.begin(), ::tolower);

            for (const auto& [type, path] : nodes) {
                std::string lowerPath = path;
                std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

                if (lowerPath.find(search) != std::string::npos) {
                    if (ImGui::Selectable(path.c_str())) {
                        ImVec2 canvasCenter = screenToCanvas(ImVec2(400, 200));
                        addNode(type, canvasCenter);
                        m_SearchBuffer[0] = '\0';
                        m_ShowSearchResults = false;
                    }
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::SameLine();
    ImGui::Text("Nodes: %zu | Links: %zu", m_Nodes.size(), m_Links.size());
}

void ScriptEditorPanel::renderCanvas() {
    ImGui::BeginChild("CanvasArea", ImVec2(0, 0), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    m_CanvasOrigin = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Сетка
    float gridSize = 20.0f * m_CanvasScale;
    for (float x = fmodf(m_CanvasOffset.x, gridSize); x < canvasSize.x; x += gridSize)
        drawList->AddLine(ImVec2(m_CanvasOrigin.x + x, m_CanvasOrigin.y),
            ImVec2(m_CanvasOrigin.x + x, m_CanvasOrigin.y + canvasSize.y), IM_COL32(60, 60, 60, 40));
    for (float y = fmodf(m_CanvasOffset.y, gridSize); y < canvasSize.y; y += gridSize)
        drawList->AddLine(ImVec2(m_CanvasOrigin.x, m_CanvasOrigin.y + y),
            ImVec2(m_CanvasOrigin.x + canvasSize.x, m_CanvasOrigin.y + y), IM_COL32(60, 60, 60, 40));

    // Связи
    renderLinks();

    // Связь в процессе перетаскивания
    if (m_IsDraggingLink) {
        ImVec2 startPos = getPinScreenPos(m_DragStartNodeId, m_DragStartPinId, !m_DragStartIsOutput);
        ImVec2 endPos = ImGui::GetMousePos();

        if (startPos.x > 0) {
            float dist = fabsf(endPos.x - startPos.x) * 0.5f;
            ImVec2 cp1(startPos.x + dist, startPos.y);
            ImVec2 cp2(endPos.x - dist, endPos.y);
            drawList->AddBezierCubic(startPos, cp1, cp2, endPos, IM_COL32(255, 200, 100, 255), 3.0f);
            drawList->AddCircleFilled(endPos, 5, IM_COL32(255, 200, 100, 255));
        }
    }

    m_HoveredPinId = -1;
    m_HoveredPinNodeId = -1;
    m_HoveredNodeId = -1;
    m_HoveredPinIsInput = false;

    for (auto& node : m_Nodes) {
        renderNode(node);
    }

    if ((ImGui::IsMouseDragging(1) || ImGui::IsMouseDragging(2)) && ImGui::IsWindowHovered()) {
        m_CanvasOffset.x += ImGui::GetIO().MouseDelta.x;
        m_CanvasOffset.y += ImGui::GetIO().MouseDelta.y;
    }

    if (ImGui::GetIO().MouseWheel != 0 && ImGui::IsWindowHovered()) {
        m_CanvasScale += ImGui::GetIO().MouseWheel * 0.1f;
        m_CanvasScale = std::max(0.2f, std::min(m_CanvasScale, 4.0f));
    }

    renderContextMenu();

    if (m_IsDraggingLink) {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
            (ImGui::IsMouseClicked(0) && m_HoveredPinId < 0 && m_HoveredNodeId < 0)) {
            m_IsDraggingLink = false;
        }
    }

    ImGui::EndChild();
}

void ScriptEditorPanel::renderNode(Scripting::Node& node) {
    ImVec2 nodeScreenPos = canvasToScreen(ImVec2(node.PosX, node.PosY));

    // Базовая ширина и заголовок
    float nodeWidth = 220;
    float headerHeight = 30;
    float pinHeight = 28;
    float contentPadding = 8;

    // Вычисляем высоту ноды
    float extraContentHeight = 0;
    bool hasInlineFields = false;

    // Для SetPosition добавляем поля X/Y
    bool isSetPos = (node.Type == "SetPosition");
    if (isSetPos) {
        extraContentHeight = 30; // Место для полей X/Y
        hasInlineFields = true;
    }

    // Для других нод с полями ввода
    bool isPrint = (node.Type == "Print");
    bool isDelay = (node.Type == "Delay");
    bool isAddFloat = (node.Type == "AddFloat");

    if (isPrint || isDelay || isAddFloat) {
        extraContentHeight = 25;
        hasInlineFields = true;
    }

    float maxPins = static_cast<float>(std::max(node.InputPins.size(), node.OutputPins.size()));
    float nodeHeight = headerHeight + maxPins * pinHeight + contentPadding + extraContentHeight;

    ImVec2 nodeEnd(nodeScreenPos.x + nodeWidth, nodeScreenPos.y + nodeHeight);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    bool isSelected = (m_SelectedNodeId == node.Id);

    // Тень
    drawList->AddRectFilled(ImVec2(nodeScreenPos.x + 3, nodeScreenPos.y + 3),
        ImVec2(nodeEnd.x + 3, nodeEnd.y + 3), IM_COL32(0, 0, 0, 100));

    // Фон ноды
    ImU32 bgColor = isSelected ? IM_COL32(70, 70, 100, 240) : IM_COL32(55, 55, 70, 240);
    drawList->AddRectFilled(nodeScreenPos, nodeEnd, bgColor);
    drawList->AddRect(nodeScreenPos, nodeEnd, IM_COL32(100, 100, 140, 255));

    // Заголовок
    ImU32 headerColor;
    if (node.Type.find("Event") != std::string::npos) headerColor = IM_COL32(120, 30, 30, 255);
    else if (node.Type == "SetPosition" || node.Type == "GetPosition") headerColor = IM_COL32(30, 80, 120, 255);
    else if (node.Type == "Print") headerColor = IM_COL32(30, 100, 60, 255);
    else if (node.Type == "Branch") headerColor = IM_COL32(100, 80, 30, 255);
    else headerColor = IM_COL32(80, 40, 110, 255);

    drawList->AddRectFilled(nodeScreenPos, ImVec2(nodeEnd.x, nodeScreenPos.y + headerHeight), headerColor);
    drawList->AddText(ImVec2(nodeScreenPos.x + 10, nodeScreenPos.y + 6),
        IM_COL32(255, 255, 255, 255), node.Name.c_str());

    // Входные пины
    for (size_t i = 0; i < node.InputPins.size(); i++) {
        auto& pin = node.InputPins[i];
        float y = nodeScreenPos.y + headerHeight + 4 + i * pinHeight;

        ImVec2 pinCenter(nodeScreenPos.x + 8, y + pinHeight / 2);
        bool isHovered = (m_HoveredPinNodeId == node.Id && m_HoveredPinId == static_cast<int>(i) && m_HoveredPinIsInput);
        float pinRadius = isHovered ? 9.0f : 7.0f;

        drawList->AddCircleFilled(pinCenter, pinRadius, getPinColor(pin.Type));
        drawList->AddCircle(pinCenter, pinRadius, IM_COL32(0, 0, 0, 200));
        drawList->AddCircleFilled(pinCenter, pinRadius - 2, getPinColor(pin.Type));

        // Имя пина
        std::string pinLabel = pin.Name;
        if (!pin.DefaultValue.empty() && pin.Type != Scripting::PinType::Flow && !hasInlineFields) {
            pinLabel += ": " + pin.DefaultValue;
        }
        drawList->AddText(ImVec2(nodeScreenPos.x + 20, y + 4),
            IM_COL32(200, 200, 200, 255), pinLabel.c_str());

        // Зона для приёма связи
        ImGui::SetCursorScreenPos(ImVec2(nodeScreenPos.x - 4, y));
        ImGui::InvisibleButton(("##in_" + std::to_string(node.Id) + "_" + std::to_string(pin.Id)).c_str(),
            ImVec2(28, pinHeight));

        if (ImGui::IsItemHovered()) {
            m_HoveredPinId = static_cast<int>(i);
            m_HoveredPinNodeId = node.Id;
            m_HoveredPinIsInput = true;

            if (m_IsDraggingLink && ImGui::IsMouseReleased(0) && m_DragStartIsOutput) {
                if (m_HoveredPinNodeId != m_DragStartNodeId) {
                    createLink(m_DragStartNodeId, m_DragStartPinId, node.Id, pin.Id);
                }
                m_IsDraggingLink = false;
            }
        }
    }

    // Выходные пины
    for (size_t i = 0; i < node.OutputPins.size(); i++) {
        auto& pin = node.OutputPins[i];
        float y = nodeScreenPos.y + headerHeight + 4 + i * pinHeight;

        float textWidth = ImGui::CalcTextSize(pin.Name.c_str()).x;
        drawList->AddText(ImVec2(nodeEnd.x - textWidth - 28, y + 4),
            IM_COL32(200, 200, 200, 255), pin.Name.c_str());

        ImVec2 pinCenter(nodeEnd.x - 8, y + pinHeight / 2);
        bool isHovered = (m_HoveredPinNodeId == node.Id && m_HoveredPinId == static_cast<int>(i) && !m_HoveredPinIsInput);
        float pinRadius = isHovered ? 9.0f : 7.0f;

        drawList->AddCircleFilled(pinCenter, pinRadius, getPinColor(pin.Type));
        drawList->AddCircle(pinCenter, pinRadius, IM_COL32(0, 0, 0, 200));
        drawList->AddCircleFilled(pinCenter, pinRadius - 2, getPinColor(pin.Type));

        ImGui::SetCursorScreenPos(ImVec2(nodeEnd.x - 24, y));
        ImGui::InvisibleButton(("##out_" + std::to_string(node.Id) + "_" + std::to_string(pin.Id)).c_str(),
            ImVec2(28, pinHeight));

        if (ImGui::IsItemHovered()) {
            m_HoveredPinId = static_cast<int>(i);
            m_HoveredPinNodeId = node.Id;
            m_HoveredPinIsInput = false;

            if (ImGui::IsMouseClicked(0) && !m_IsDraggingLink) {
                m_IsDraggingLink = true;
                m_DragStartNodeId = node.Id;
                m_DragStartPinId = pin.Id;
                m_DragStartIsOutput = true;
            }
        }
    }

    // ============ ВСТРОЕННЫЕ ПОЛЯ ДЛЯ SET POSITION ============
    if (isSetPos) {
        float fieldY = nodeScreenPos.y + headerHeight + 4 + 3 * pinHeight + 6;

        // X field
        ImGui::SetCursorScreenPos(ImVec2(nodeScreenPos.x + 20, fieldY));
        ImGui::SetNextItemWidth(85);

        // Получаем текущие значения из пина Position
        float posX = 0, posY = 0;
        for (auto& pin : node.InputPins) {
            if (pin.Name == "Position" && !pin.DefaultValue.empty()) {
                sscanf_s(pin.DefaultValue.c_str(), "%f,%f", &posX, &posY);
            }
        }

        ImGui::PushID(("posX_" + std::to_string(node.Id)).c_str());
        if (ImGui::DragFloat("##X", &posX, 1.0f)) {
            for (auto& pin : node.InputPins) {
                if (pin.Name == "Position") {
                    pin.DefaultValue = std::to_string(posX) + "," + std::to_string(posY);
                }
            }
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::SetCursorScreenPos(ImVec2(nodeScreenPos.x + 115, fieldY));
        ImGui::SetNextItemWidth(85);

        ImGui::PushID(("posY_" + std::to_string(node.Id)).c_str());
        if (ImGui::DragFloat("##Y", &posY, 1.0f)) {
            for (auto& pin : node.InputPins) {
                if (pin.Name == "Position") {
                    pin.DefaultValue = std::to_string(posX) + "," + std::to_string(posY);
                }
            }
        }
        ImGui::PopID();
    }

    // Перетаскивание ноды
    ImGui::SetCursorScreenPos(nodeScreenPos);
    ImGui::InvisibleButton(("##node_" + std::to_string(node.Id)).c_str(), ImVec2(nodeWidth, nodeHeight));

    if (ImGui::IsItemHovered()) {
        m_HoveredNodeId = node.Id;
    }

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        node.PosX += ImGui::GetIO().MouseDelta.x / m_CanvasScale;
        node.PosY += ImGui::GetIO().MouseDelta.y / m_CanvasScale;
    }

    if (ImGui::IsItemClicked()) {
        m_SelectedNodeId = node.Id;
    }
}

void ScriptEditorPanel::renderLinks() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (size_t i = 0; i < m_Links.size(); i++) {
        const auto& link = m_Links[i];
        ImVec2 startPos = getPinScreenPos(link.StartNodeId, link.StartPinId, false);
        ImVec2 endPos = getPinScreenPos(link.EndNodeId, link.EndPinId, true);

        if (startPos.x > 0 && endPos.x > 0) {
            ImVec2 midPos((startPos.x + endPos.x) * 0.5f, (startPos.y + endPos.y) * 0.5f);
            ImGui::SetCursorScreenPos(ImVec2(midPos.x - 10, midPos.y - 10));
            ImGui::InvisibleButton(("##link_" + std::to_string(link.Id)).c_str(), ImVec2(20, 20));

            float dist = fabsf(endPos.x - startPos.x) * 0.5f;
            ImVec2 cp1(startPos.x + dist, startPos.y);
            ImVec2 cp2(endPos.x - dist, endPos.y);

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Click to delete");
                drawList->AddBezierCubic(startPos, cp1, cp2, endPos, IM_COL32(255, 80, 80, 255), 4.0f);
                if (ImGui::IsItemClicked()) {
                    removeLink(link.Id);
                    return;
                }
            }
            else {
                drawList->AddBezierCubic(startPos, cp1, cp2, endPos, IM_COL32(200, 200, 200, 255), 2.5f);
            }
        }
    }
}

void ScriptEditorPanel::renderContextMenu() {
    if (m_HoveredNodeId < 0 && ImGui::IsMouseClicked(1) && ImGui::IsWindowHovered()) {
        ImGui::OpenPopup("CanvasContext");
        m_SearchBuffer[0] = '\0';
    }

    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_Appearing);

    if (ImGui::BeginPopup("CanvasContext")) {
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Add Node");
        ImGui::Separator();

        ImGui::SetNextItemWidth(-1);
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputTextWithHint("##SearchNodes", "Search nodes...", m_SearchBuffer, sizeof(m_SearchBuffer));
        ImGui::Separator();

        ImGui::BeginChild("NodeList", ImVec2(0, 0), false);

        auto nodes = Scripting::NodeRegistry::Instance().GetAvailableNodes();
        std::string lastCategory;
        std::string search = m_SearchBuffer;
        std::transform(search.begin(), search.end(), search.begin(), ::tolower);

        int shownCount = 0;

        for (const auto& [type, path] : nodes) {
            std::string lowerPath = path;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

            if (!search.empty() && lowerPath.find(search) == std::string::npos) continue;

            size_t slash = path.find('/');
            std::string cat = path.substr(0, slash);
            std::string name = path.substr(slash + 1);

            if (cat != lastCategory) {
                if (!lastCategory.empty()) ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 0.9f, 1.0f), "%s", cat.c_str());
                ImGui::Separator();
                lastCategory = cat;
            }

            std::string icon = getNodeIcon(type);
            std::string label = icon + " " + name;

            if (ImGui::Selectable(label.c_str())) {
                ImVec2 mouseCanvas = screenToCanvas(ImGui::GetMousePos());
                addNode(type, mouseCanvas);
                m_SearchBuffer[0] = '\0';
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", getNodeDescription(type).c_str());
            }

            shownCount++;
        }

        if (shownCount == 0) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No nodes found");
        }

        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

void ScriptEditorPanel::renderNodeProperties() {
    ImGui::Text("Node Properties");
    ImGui::Separator();

    if (m_SelectedNodeId >= 0) {
        Scripting::Node* selectedNode = nullptr;
        for (auto& node : m_Nodes) {
            if (node.Id == m_SelectedNodeId) {
                selectedNode = &node;
                break;
            }
        }

        if (!selectedNode) return;

        ImGui::Text("Name: %s", selectedNode->Name.c_str());
        ImGui::Text("Type: %s", selectedNode->Type.c_str());
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Default Values:");

        for (auto& pin : selectedNode->InputPins) {
            if (pin.Type == Scripting::PinType::Flow) continue;

            ImGui::PushID(pin.Id);

            bool hasConnection = false;
            for (const auto& link : m_Links) {
                if (link.EndNodeId == selectedNode->Id && link.EndPinId == pin.Id) {
                    hasConnection = true;
                    break;
                }
            }

            if (hasConnection) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s (connected)", pin.Name.c_str());
            }
            else {
                switch (pin.Type) {
                case Scripting::PinType::Float: {
                    float val = 0.0f;
                    if (!pin.DefaultValue.empty()) val = std::stof(pin.DefaultValue);
                    if (ImGui::DragFloat(pin.Name.c_str(), &val, 0.1f)) {
                        pin.DefaultValue = std::to_string(val);
                    }
                    break;
                }
                case Scripting::PinType::Int: {
                    int val = 0;
                    if (!pin.DefaultValue.empty()) val = std::stoi(pin.DefaultValue);
                    if (ImGui::DragInt(pin.Name.c_str(), &val)) {
                        pin.DefaultValue = std::to_string(val);
                    }
                    break;
                }
                case Scripting::PinType::Bool: {
                    bool val = (pin.DefaultValue == "true");
                    if (ImGui::Checkbox(pin.Name.c_str(), &val)) {
                        pin.DefaultValue = val ? "true" : "false";
                    }
                    break;
                }
                case Scripting::PinType::String: {
                    char buf[128];
                    strcpy_s(buf, sizeof(buf), pin.DefaultValue.c_str());
                    if (ImGui::InputText(pin.Name.c_str(), buf, sizeof(buf))) {
                        pin.DefaultValue = buf;
                    }
                    break;
                }
                case Scripting::PinType::Entity: {
                    int currentEntity = -1;
                    if (!pin.DefaultValue.empty()) {
                        currentEntity = std::stoi(pin.DefaultValue);
                    }

                    std::string preview = "None";
                    if (currentEntity >= 0 && m_Scene.isValid(currentEntity)) {
                        preview = m_Scene.getEntityName(currentEntity) + " (#" + std::to_string(currentEntity) + ")";
                    }

                    if (ImGui::BeginCombo(pin.Name.c_str(), preview.c_str())) {
                        if (ImGui::Selectable("Self", currentEntity == m_CurrentEntity)) {
                            pin.DefaultValue = std::to_string(m_CurrentEntity);
                        }

                        for (auto& [entity, _] : m_Scene.transforms) {
                            std::string label = m_Scene.getEntityName(entity) + " (#" + std::to_string(entity) + ")";
                            if (ImGui::Selectable(label.c_str(), currentEntity == entity)) {
                                pin.DefaultValue = std::to_string(entity);
                            }
                        }
                        ImGui::EndCombo();
                    }
                    break;
                }
                default:
                    break;
                }
            }

            ImGui::PopID();
        }

        ImGui::Separator();
        if (ImGui::Button("Delete Node", ImVec2(-1, 0))) {
            deleteNode(selectedNode->Id);
            m_SelectedNodeId = -1;
        }
    }
    else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select a node to edit");
    }
}

void ScriptEditorPanel::addNode(const std::string& type, ImVec2 canvasPos) {
    Scripting::Node node = Scripting::NodeRegistry::Instance().CreateNode(type);
    node.Id = m_NextNodeId++;
    node.PosX = canvasPos.x;
    node.PosY = canvasPos.y;

    for (auto& pin : node.InputPins) {
        if (pin.Type == Scripting::PinType::Entity && pin.DefaultValue.empty()) {
            pin.DefaultValue = std::to_string(m_CurrentEntity);
        }
    }

    m_Nodes.push_back(node);
    m_SelectedNodeId = node.Id;
}

void ScriptEditorPanel::deleteNode(int nodeId) {
    m_Nodes.erase(std::remove_if(m_Nodes.begin(), m_Nodes.end(),
        [nodeId](const Scripting::Node& n) { return n.Id == nodeId; }), m_Nodes.end());
    m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
        [nodeId](const Scripting::Link& l) { return l.StartNodeId == nodeId || l.EndNodeId == nodeId; }), m_Links.end());
    if (m_SelectedNodeId == nodeId) m_SelectedNodeId = -1;
}

void ScriptEditorPanel::createLink(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    for (auto& link : m_Links) {
        if (link.EndNodeId == endNodeId && link.EndPinId == endPinId) {
            removeLink(link.Id);
            break;
        }
    }

    m_Links.push_back({ m_NextLinkId++, startNodeId, startPinId, endNodeId, endPinId });
    std::cout << "[Script] Link: " << startNodeId << ":" << startPinId
        << " -> " << endNodeId << ":" << endPinId << std::endl;
}

void ScriptEditorPanel::removeLink(int linkId) {
    m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
        [linkId](const Scripting::Link& l) { return l.Id == linkId; }), m_Links.end());
}

void ScriptEditorPanel::executeGraph() {
    ScriptRuntime runtime(m_Scene);
    runtime.executeAll(m_Nodes, m_Links, m_CurrentEntity);
    std::cout << "[Script] Execute complete" << std::endl;
}

void ScriptEditorPanel::executeNodeChain(Scripting::Node& node) {
    // Выполняем действие ноды
    if (node.Type == "Print") {
        std::string msg = "Print";
        for (auto& pin : node.InputPins) {
            if (pin.Type == Scripting::PinType::String && !pin.DefaultValue.empty()) {
                msg = pin.DefaultValue;
            }
        }
        std::cout << "[Print] " << msg << std::endl;
    }
    else if (node.Type == "SetPosition") {
        Entity target = m_CurrentEntity;
        sf::Vector2f pos = { 0, 0 };

        for (auto& pin : node.InputPins) {
            if (pin.Name == "Target" && !pin.DefaultValue.empty()) {
                target = static_cast<Entity>(std::stoi(pin.DefaultValue));
            }
            if (pin.Name == "Position" && !pin.DefaultValue.empty()) {
                // Парсим "X,Y"
                std::string val = pin.DefaultValue;
                size_t comma = val.find(',');
                if (comma != std::string::npos) {
                    pos.x = std::stof(val.substr(0, comma));
                    pos.y = std::stof(val.substr(comma + 1));
                }
                else {
                    pos.x = std::stof(val);
                }
            }
        }

        if (m_Scene.isValid(target)) {
            m_Scene.setPosition(target, pos);
            std::cout << "[SetPosition] Entity " << target << " -> (" << pos.x << ", " << pos.y << ")" << std::endl;
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

void ScriptEditorPanel::saveGraph() {
    std::stringstream ss;
    ss << "{";
    ss << "\"nodes\":[";
    for (size_t i = 0; i < m_Nodes.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"id\":" << m_Nodes[i].Id
            << ",\"type\":\"" << m_Nodes[i].Type << "\""
            << ",\"x\":" << m_Nodes[i].PosX
            << ",\"y\":" << m_Nodes[i].PosY;

        ss << ",\"pins\":{";
        bool firstPin = true;
        for (auto& pin : m_Nodes[i].InputPins) {
            if (!pin.DefaultValue.empty()) {
                if (!firstPin) ss << ",";
                ss << "\"" << pin.Id << "\":\"" << pin.DefaultValue << "\"";
                firstPin = false;
            }
        }
        ss << "}";

        ss << "}";
    }
    ss << "],\"links\":[";
    for (size_t i = 0; i < m_Links.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"id\":" << m_Links[i].Id
            << ",\"sn\":" << m_Links[i].StartNodeId
            << ",\"sp\":" << m_Links[i].StartPinId
            << ",\"en\":" << m_Links[i].EndNodeId
            << ",\"ep\":" << m_Links[i].EndPinId << "}";
    }
    ss << "]}";

    if (!m_Scene.scripts.contains(m_CurrentEntity)) {
        m_Scene.scripts[m_CurrentEntity] = ScriptComponent{};
    }
    m_Scene.scripts[m_CurrentEntity].scriptPath = ss.str();
    std::cout << "[Script] Saved: " << m_Nodes.size() << " nodes, "
        << m_Links.size() << " links" << std::endl;
}

void ScriptEditorPanel::loadGraph() {
    std::string data = m_Scene.scripts[m_CurrentEntity].scriptPath;
    if (data.empty()) return;
    std::cout << "[Script] Graph loaded" << std::endl;
}

ImVec2 ScriptEditorPanel::screenToCanvas(const ImVec2& screenPos) const {
    return ImVec2(
        (screenPos.x - m_CanvasOrigin.x - m_CanvasOffset.x) / m_CanvasScale,
        (screenPos.y - m_CanvasOrigin.y - m_CanvasOffset.y) / m_CanvasScale
    );
}

ImVec2 ScriptEditorPanel::canvasToScreen(const ImVec2& canvasPos) const {
    return ImVec2(
        m_CanvasOrigin.x + canvasPos.x * m_CanvasScale + m_CanvasOffset.x,
        m_CanvasOrigin.y + canvasPos.y * m_CanvasScale + m_CanvasOffset.y
    );
}

ImVec2 ScriptEditorPanel::getPinScreenPos(int nodeId, int pinId, bool isInput) const {
    for (const auto& node : m_Nodes) {
        if (node.Id != nodeId) continue;

        ImVec2 nodeScreen = canvasToScreen(ImVec2(node.PosX, node.PosY));
        float headerHeight = 30;
        float pinHeight = 28;
        float nodeWidth = 220;

        const auto& pins = isInput ? node.InputPins : node.OutputPins;
        for (size_t i = 0; i < pins.size(); i++) {
            if (pins[i].Id == pinId) {
                float y = nodeScreen.y + headerHeight + 4 + i * pinHeight + pinHeight / 2;
                float x = isInput ? nodeScreen.x + 8 : nodeScreen.x + nodeWidth - 8;
                return ImVec2(x, y);
            }
        }
    }
    return ImVec2(-1, -1);
}

ImU32 ScriptEditorPanel::getPinColor(Scripting::PinType type) const {
    switch (type) {
    case Scripting::PinType::Flow: return IM_COL32(255, 255, 255, 255);
    case Scripting::PinType::Float:
    case Scripting::PinType::Int: return IM_COL32(100, 200, 100, 255);
    case Scripting::PinType::Bool: return IM_COL32(255, 100, 100, 255);
    case Scripting::PinType::String: return IM_COL32(200, 150, 50, 255);
    case Scripting::PinType::Vector2: return IM_COL32(200, 200, 50, 255);
    case Scripting::PinType::Entity: return IM_COL32(100, 150, 255, 255);
    default: return IM_COL32(150, 150, 150, 255);
    }
}

const char* ScriptEditorPanel::getPinTypeName(Scripting::PinType type) const {
    switch (type) {
    case Scripting::PinType::Flow: return "Flow";
    case Scripting::PinType::Float: return "Float";
    case Scripting::PinType::Int: return "Int";
    case Scripting::PinType::Bool: return "Bool";
    case Scripting::PinType::String: return "String";
    case Scripting::PinType::Vector2: return "Vector2";
    case Scripting::PinType::Entity: return "Entity";
    default: return "Any";
    }
}

std::string ScriptEditorPanel::getNodeIcon(const std::string& type) const {
    if (type.find("Event") != std::string::npos) return "[>]";
    if (type == "SetPosition" || type == "GetPosition") return "[P]";
    if (type == "Print") return "[D]";
    if (type == "Branch") return "[?]";
    if (type == "Delay") return "[T]";
    if (type.find("Vector") != std::string::npos || type == "MakeVector" || type == "BreakVector") return "[V]";
    if (type.find("Float") != std::string::npos || type == "AddFloat") return "[#]";
    if (type == "GetOwner") return "[E]";
    return "[ ]";
}

std::string ScriptEditorPanel::getNodeDescription(const std::string& type) const {
    if (type == "EventBeginPlay") return "Triggers when the game starts";
    if (type == "EventTick") return "Triggers every frame";
    if (type == "SetPosition") return "Sets entity position";
    if (type == "GetPosition") return "Gets entity position";
    if (type == "Print") return "Prints to console";
    if (type == "Branch") return "If/Else condition";
    if (type == "Delay") return "Wait for duration";
    return "";
}