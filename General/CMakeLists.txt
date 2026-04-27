add_library(AlternativeCore STATIC
    Core/GamePlayer.cpp
    ECS/Scene.cpp
    ECS/Systems/MovementSystem.cpp
    ECS/Systems/RenderSystem.cpp
    Serialization/SceneSerializer.cpp
    Rendering/Viewport.cpp
    Rendering/DebugRenderer.cpp
    Resources/ResourceManager.cpp
    Resources/TextureBrowser.cpp
    Utils/FileUtils.cpp
    Utils/CoordinateConverter.cpp
)

target_include_directories(AlternativeCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SFML_INCLUDE}
    # Новая строка для stb:
    ${CMAKE_SOURCE_DIR}/AlternativeEngine/libs/stb
)