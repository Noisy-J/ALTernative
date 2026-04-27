add_executable(AlternativeGame
    src/Game/Main.cpp
    ${CMAKE_SOURCE_DIR}/AlternativeEngine/libs/stb/stb_image.cpp
)

target_include_directories(AlternativeGame PRIVATE
    ${CMAKE_SOURCE_DIR}/General
    ${SFML_INCLUDE}
)

target_link_libraries(AlternativeGame PRIVATE
    AlternativeCore
    ${SFML_LIBS}/sfml-graphics.lib
    ${SFML_LIBS}/sfml-window.lib
    ${SFML_LIBS}/sfml-system.lib
    opengl32.lib
    winmm.lib
    gdi32.lib
)

set_target_properties(AlternativeGame PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:CONSOLE"
)

# Копируем ассеты и сцены после сборки
copy_engine_assets(AlternativeGame)

# Копируем SFML DLL
add_custom_command(TARGET AlternativeGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_ROOT}/bin/sfml-graphics-3.dll"
        "$<TARGET_FILE_DIR:AlternativeGame>/sfml-graphics-3.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_ROOT}/bin/sfml-window-3.dll"
        "$<TARGET_FILE_DIR:AlternativeGame>/sfml-window-3.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_ROOT}/bin/sfml-system-3.dll"
        "$<TARGET_FILE_DIR:AlternativeGame>/sfml-system-3.dll"
    COMMENT "Copying SFML DLLs..."
)