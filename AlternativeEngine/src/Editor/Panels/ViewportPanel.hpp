#pragma once

#include <imgui.h>
#include "Rendering/Viewport.hpp"
#include "ECS/Scene.hpp"

class ViewportPanel {
public:
    ViewportPanel(Viewport& viewport);
    ~ViewportPanel() = default;

    void render(Scene& scene);  // Теперь принимает ссылку на сцену

    bool isShowingColliders() const { return m_ShowColliders; }

private:
    Viewport& m_Viewport;

    // Состояние тулбара
    bool m_ShowGridOverlay{ true };
    bool m_ShowBounds{ false };
    bool m_ShowColliders{ false };
    int m_MouseMode{ 0 };
    bool m_ShowDebugPopup{ false };

    void renderToolbar();
    void handleResize();
    //void renderOverlaySettings();
    void renderColliders(Scene& scene);  // Новая функция
};