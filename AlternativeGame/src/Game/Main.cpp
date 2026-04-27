#include "Core/GamePlayer.hpp"
#include <iostream>
#include <fstream>  //Видём логи игры

int main(int argc, char* argv[]) {
    // Перенаправляем ошибки в файл
    std::ofstream logFile("player_debug.log");
    auto oldCerr = std::cerr.rdbuf();
    std::cerr.rdbuf(logFile.rdbuf());

    std::string projectPath;

    // Ищем путь к проекту в аргументах командной строки
    if (argc > 1) {
        projectPath = argv[1];
    }
    else {
        // По умолчанию — ищем project.lge_config рядом с .exe
        projectPath = ".";
    }

    std::cout << "=== AlternativeGame Player v1.1 ===" << std::endl;
    std::cout << "Project: " << projectPath << std::endl;

    try {
        GamePlayer player(projectPath);

        if (!player.initialize()) {
            std::cerr << "Failed to initialize player" << std::endl;
            return 1;
        }

        player.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}