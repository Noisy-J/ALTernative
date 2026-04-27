add_executable(AlternativeEngine
    src/main.cpp
    src/Core/Engine.cpp
    src/Build/BuildSystem.cpp
    src/Build/BuildDialog.cpp
    src/Editor/EditorUI.cpp
    src/Editor/Panels/ContentBrowserPanel.cpp
    src/Editor/Panels/DebugPanel.cpp
    src/Editor/Panels/HierarchyPanel.cpp
    src/Editor/Panels/InspectorPanel.cpp
    src/Editor/Panels/ScriptEditorPanel.cpp
    src/Editor/Panels/ViewportPanel.cpp
    src/Editor/Dialogs/CreateEntityDialog.cpp
    src/Editor/Dialogs/FileDialog.cpp
    src/Editor/Dialogs/TextureSelectorDialog.cpp
    src/Input/InputManager.cpp
    src/Input/CameraController.cpp
    src/Input/EntityDragger.cpp
    src/Design/StyleManager.cpp
    libs/imgui/imgui.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui-SFML.cpp
    libs/stb/stb_image.cpp
)

target_include_directories(AlternativeEngine PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/General
    libs/imgui
    libs/stb
    ${SFML_INCLUDE}
)

target_link_libraries(AlternativeEngine PRIVATE
    AlternativeCore
    ${SFML_LIBS}/sfml-graphics.lib
    ${SFML_LIBS}/sfml-window.lib
    ${SFML_LIBS}/sfml-system.lib
    opengl32.lib
    winmm.lib
    gdi32.lib
)

set_target_properties(AlternativeEngine PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:CONSOLE"
)

# Копируем ассеты и сцены после сборки
copy_engine_assets(AlternativeEngine)

# Копируем SFML DLL
add_custom_command(TARGET AlternativeEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_ROOT}/bin/sfml-graphics-3.dll"
        "$<TARGET_FILE_DIR:AlternativeEngine>/sfml-graphics-3.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_ROOT}/bin/sfml-window-3.dll"
        "$<TARGET_FILE_DIR:AlternativeEngine>/sfml-window-3.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_ROOT}/bin/sfml-system-3.dll"
        "$<TARGET_FILE_DIR:AlternativeEngine>/sfml-system-3.dll"
    COMMENT "Copying SFML DLLs..."
)