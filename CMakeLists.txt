cmake_minimum_required(VERSION 3.28)
project(Alternative CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFML
set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/AlternativeEngine/libs/SFML-3.0.2")
set(SFML_INCLUDE "${SFML_ROOT}/include")
set(SFML_LIBS "${SFML_ROOT}/lib")

# Путь к ресурсам движка
set(ENGINE_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/AlternativeEngine")

# Функция для копирования ресурсов после сборки
function(copy_engine_assets TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${ENGINE_ASSETS_DIR}/assets"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${ENGINE_ASSETS_DIR}/scenes"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/scenes"
        COMMENT "Copying engine assets to build directory..."
    )
endfunction()

add_subdirectory(General)
add_subdirectory(AlternativeEngine)
add_subdirectory(AlternativeGame)