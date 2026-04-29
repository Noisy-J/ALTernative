# 🎮 Alternative Engine

> Современный 2D игровой движок на C++20 с ECS-архитектурой, встроенным редактором и системой сборки проектов

<p align="center">
  <img src="https://github.com/user-attachments/assets/6486bd54-4c93-421c-a563-ee723ed758e9" alt="Alternative Engine Banner" />
</p>

<p align="center">
  <a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++20" /></a>
  <a href="https://www.sfml-dev.org/"><img src="https://img.shields.io/badge/SFML-3.0-8CC445?style=for-the-badge&logo=sfml&logoColor=white" alt="SFML 3.0" /></a>
  <a href="https://github.com/ocornut/imgui"><img src="https://img.shields.io/badge/Dear%20ImGui-1.90-5B3E8C?style=for-the-badge" alt="Dear ImGui" /></a>
  <a href="https://cmake.org/"><img src="https://img.shields.io/badge/CMake-3.20%2B-064F8C?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake" /></a>
  <br/>
  <a href="https://github.com/Noisy-J/AlternativeEngine/actions"><img src="https://img.shields.io/github/actions/workflow/status/Noisy-J/AlternativeEngine/build.yml?style=flat-square&logo=githubactions&label=Build" alt="Build Status" /></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square" alt="MIT License" /></a>
  <a href="https://github.com/Noisy-J/AlternativeEngine/releases"><img src="https://img.shields.io/github/v/release/Noisy-J/AlternativeEngine?style=flat-square&color=blue" alt="Release" /></a>
  <img src="https://img.shields.io/badge/Platform-Windows%2010%2B-0078D6?style=flat-square&logo=windows&logoColor=white" alt="Windows 10+" />
</p>

---

## 📋 Оглавление

- [✨ Возможности](#-возможности)
- [🏗️ Архитектура](#️-архитектура)
- [🎬 Быстрый старт](#-быстрый-старт)
- [🎮 Управление](#-управление)
- [📁 Структура проекта](#-структура-проекта)
- [🛠️ Сборка из исходников](#️-сборка-из-исходников)
- [🔮 Roadmap](#-roadmap)
- [🤝 Участие в разработке](#-участие-в-разработке)
- [👥 Авторы](#-авторы)
- [📄 Лицензия](#-лицензия)

---

## ✨ Возможности

### 🎯 ECS Архитектура
*   **Entity Component System** — гибкая и производительная архитектура, лежащая в основе движка.
*   **Поддерживаемые компоненты:**
    *   `Transform` — позиция, поворот, масштаб
    *   `Sprite` — рендеринг спрайтов с поддержкой слоёв
    *   `Velocity` — скорость и физическое движение
    *   `Collider` — Box/Circle коллайдеры с триггерами (в разработке)
    *   `Health` — система здоровья (в разработке)
    *   `Tag` — теги для идентификации объектов

### 🖥️ Встроенный редактор
*   **Viewport** с поддержкой:
    *   Перемещение камеры (Middle Mouse Button)
    *   Зум (Колёсико мыши)
    *   Визуализация границ текстур
    *   Drag & Drop перемещение объектов
    
*   **Панели редактора**:
    *   `Inspector` — просмотр и редактирование компонентов
    *   `Content Browser` — навигация по ассетам проекта
    *   `Debug Panel` — отладочная информация в реальном времени
    *   `Viewport` — окно рендеринга сцены
    *   `Hierarchy` — иерархия сущностей на сцене

### 🔧 Инструменты разработчика
*   **Контекстное меню** (ПКМ во Viewport):
    *   Создание пустых сущностей
    *   Мастер создания с пошаговой настройкой
    *   Готовые префабы (Player, Enemy, Item, Camera Target)
    
*   **Texture Browser** с предпросмотром и поиском
*   **Система перетаскивания** объектов мышью
*   **Горячие клавиши**:
    *   `Delete` — удалить выбранную сущность
    *   `WASD` / `Стрелки` — управление игроком
    *   `Ctrl+Z/Y` — Undo/Redo (в разработке)

### 📦 Система сборки (Build System)
*   **Экспорт** собранной игры в отдельную папку
*   **Копирование** всех необходимых ресурсов (текстуры, сцены, DLL)
*   **Автоматическая генерация** конфигурационных файлов
*   **Настройка** имени проекта и выходной директории

### 🎨 Рендеринг
*   Рендеринг в текстуру через `sf::RenderTexture`
*   Поддержка прозрачности и tint-цветов
*   Сортировка спрайтов по слоям
*   Отладочная отрисовка хитбоксов

---

## 🏗️ Архитектура

Движок построен на многоуровневой модульной архитектуре, где каждый слой имеет четкую зону ответственности.

```
┌──────────────────────────────────────────────────────────────────┐
│                      Application Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────────────┐  │
│  │   Editor UI  │  │   Systems    │  │   Resource Manager     │  │
│  │   • Panels   │  │  • Movement  │  │   • Texture Cache      │  │
│  │   • Dialogs  │  │  • Render    │  │   • Font Cache         │  │
│  │   • Widgets  │  │              │  │   • stb_image Loader   │  │
│  └──────────────┘  └──────────────┘  └────────────────────────┘  │
├──────────────────────────────────────────────────────────────────┤
│                        Core Layer                                │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────────────┐  │
│  │   Engine     │  │    Scene     │  │   Entity Manager       │  │
│  │  Game Loop   │◄─┤  Container   │◄─┤   Component Store      │  │
│  │  Build Sys   │  │  Serializer  │  │   Scene Management     │  │
│  └──────────────┘  └──────────────┘  └────────────────────────┘  │
├──────────────────────────────────────────────────────────────────┤
│                     Foundation Layer                             │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────────────┐  │
│  │   SFML 3.0   │  │  Dear ImGui  │  │   Input Manager        │  │
│  │  • Window    │  │  • Docking   │  │   • Camera Controller  │  │
│  │  • Graphics  │  │  • Widgets   │  │   • Entity Dragger     │  │
│  └──────────────┘  └──────────────┘  └────────────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

### Модульная структура

| Модуль | Описание |
|--------|------------|
| **Core** | Основной цикл приложения, инициализация подсистем, управление состоянием |
| **ECS** | Сущности, компоненты, системы (Movement, Render), контейнер сцены |
| **Rendering** | Viewport, камера, рендеринг в текстуру, отладочная отрисовка |
| **Input** | Обработка ввода, управление камерой, перетаскивание объектов |
| **Editor** | Панели ImGui, диалоговые окна, инспектор компонентов |
| **Resources** | Кэширование текстур, загрузка изображений через stb_image |
| **Serialization** | Сохранение/загрузка сцен в формате `.alt_scene` |
| **Utils** | Конвертация координат, файловые утилиты |

---

## 🎬 Быстрый старт

### Предварительные требования

*   **ОС**: Windows 10 или новее
*   **IDE**: Visual Studio 2022/2026 с нагрузкой *"Разработка классических приложений на C++"*
*   **CMake**: 3.20 или новее (установите через Visual Studio Installer)

### Клонирование и сборка

```bash
# Клонируйте репозиторий
git clone https://github.com/Noisy-J/AlternativeEngine.git
cd AlternativeEngine

# Сборка через CMake (Release)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Запуск редактора
./build/AlternativeEngine/Release/AlternativeEngine.exe
```

После запуска вы можете сразу начать создавать сцены, добавлять сущности, настраивать компоненты и экспортировать готовую игру.

---

## 🎮 Управление

### В редакторе
| Действие | Горячая клавиша |
|----------|:---------------:|
| Перемещение камеры | `СКМ` + Drag |
| Зум камеры | `Колёсико мыши` |
| Выделить объект | `ЛКМ` по объекту |
| Переместить объект | `ЛКМ` + Drag |
| Контекстное меню | `ПКМ` по пустому месту |
| Удалить объект | `Delete` |
| Открыть Inspector | Панель справа |

### В игре (по умолчанию)
| Действие | Горячая клавиша |
|----------|:---------------:|
| Движение влево | `A` / `←` |
| Движение вправо | `D` / `→` |
| Движение вверх | `W` / `↑` |
| Движение вниз | `S` / `↓` |

---

## 📁 Структура проекта

```
AlternativeEngine/
│
├── AlternativeEngine/                  # Исходный код редактора
│   ├── src/
│   │   ├── Core/                       # Ядро движка
│   │   │   └── Engine.cpp/hpp          # Главный класс, игровой цикл
│   │   │
│   │   ├── Build/                      # Система сборки проектов
│   │   │   ├── BuildSystem.cpp/hpp     # Логика сборки
│   │   │   └── BuildDialog.cpp/hpp     # UI диалога сборки
│   │   │
│   │   ├── Editor/                     # Редактор
│   │   │   ├── EditorUI.cpp/hpp        # Главный интерфейс редактора
│   │   │   ├── Panels/                 # Панели редактора
│   │   │   │   ├── InspectorPanel      # Инспектор компонентов
│   │   │   │   ├── ContentBrowserPanel # Браузер ассетов
│   │   │   │   ├── ViewportPanel       # Панель вьюпорта
│   │   │   │   ├── HierarchyPanel      # Иерархия сущностей
│   │   │   │   └── DebugPanel          # Отладочная панель
│   │   │   └── Dialogs/                # Диалоговые окна
│   │   │       ├── CreateEntityDialog  # Мастер создания сущностей
│   │   │       ├── FileDialog          # Диалог открытия/сохранения
│   │   │       └── TextureSelectorDialog # Выбор текстуры
│   │   │
│   │   ├── Input/                      # Система ввода
│   │   │   ├── InputManager.cpp/hpp    # Менеджер ввода
│   │   │   ├── CameraController.cpp/hpp# Управление камерой
│   │   │   └── EntityDragger.cpp/hpp   # Перетаскивание объектов
│   │   │
│   │   └── Design/                     # Дизайн и стили
│   │       └── StyleManager.cpp/hpp    # Управление стилями ImGui
│   │
│   └── libs/                           # Внешние библиотеки
│       ├── SFML-3.0.2/                 # SFML (графика, окна, аудио)
│       ├── imgui/                      # Dear ImGui (UI редактора)
│       └── stb/                        # stb_image (загрузка изображений)
│
├── General/                            # Общая библиотека (ядро)
│   ├── ECS/                            # Entity Component System
│   │   ├── Entity.hpp                  # Тип сущности (Entity)
│   │   ├── Components.hpp              # Все компоненты
│   │   ├── Scene.cpp/hpp               # Контейнер сцены
│   │   └── Systems/                    # ECS системы
│   │       ├── MovementSystem.cpp/hpp  # Система движения
│   │       └── RenderSystem.cpp/hpp    # Система рендеринга
│   │
│   ├── Rendering/                      # Графическая подсистема
│   │   ├── Viewport.cpp/hpp            # Вьюпорт и камера
│   │   └── DebugRenderer.cpp/hpp       # Отладка
│   │
│   ├── Resources/                      # Ресурсы
│   │   ├── ResourceManager.cpp/hpp     # Кэш ресурсов
│   │   └── TextureBrowser.cpp/hpp      # Сканер текстур
│   │
│   ├── Serialization/                  # Сериализация
│   │   └── SceneSerializer.cpp/hpp     # Сохранение/загрузка сцен (.alt_scene)
│   │
│   ├── Utils/                          # Утилиты
│   │   ├── CoordinateConverter.cpp/hpp # Конвертация координат
│   │   └── FileUtils.cpp/hpp           # Работа с файлами
│   │
│   └── Core/                           # Ядро плеера
│       └── GamePlayer.cpp/hpp          # Базовый класс игрового плеера
│
├── AlternativeGame/                    # Шаблонный проект игры (Player)
│   └── src/
│       └── Game/
│           └── Main.cpp                # Точка входа в игру
│
└── CMakeLists.txt                      # Корневой CMake-файл
```

---

## 🛠️ Сборка из исходников

### Через Visual Studio (рекомендуется)
1.  Убедитесь, что установлен компонент *"Разработка классических приложений на C++"* и *"Инструменты CMake C++ для Windows"*.
2.  Клонируйте репозиторий: `git clone https://github.com/Noisy-J/AlternativeEngine.git`
3.  Откройте папку проекта **как CMake-проект** в Visual Studio: `File` → `Open` → `CMake...` → выберите `CMakeLists.txt`
4.  Visual Studio автоматически сконфигурирует и соберет проект. Выберите цель `AlternativeEngine.exe` и нажмите `F5`.

### Через терминал
```bash
git clone https://github.com/Noisy-J/AlternativeEngine.git
cd AlternativeEngine
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
cd build/AlternativeEngine/Release
./AlternativeEngine.exe
```

---

## 🔮 Roadmap

- [x] ECS архитектура
- [x] Встроенный редактор с докинг-интерфейсом
- [x] Drag & Drop объектов
- [x] Мастер создания сущностей
- [x] Сохранение/загрузка сцен (JSON-based `.alt_scene`)
- [x] Система сборки экспортируемых проектов
- [ ] Undo/Redo система
- [ ] Физический движок (интеграция с Box2D)
- [ ] Скриптовая система (интеграция с Lua)
- [ ] Система частиц
- [ ] Анимация спрайтов (Spritesheet)
- [ ] Звуковая система
- [ ] Улучшенный сборщик проектов

---

## 🤝 Участие в разработке

Мы приветствуем вклад в развитие движка! Вот как можно помочь:

1.  **Нашли баг** или **есть идея**? Откройте [Issue](https://github.com/Noisy-J/AlternativeEngine/issues).
2.  **Хотите исправить баг или добавить фичу?**
    - Сделайте форк репозитория
    - Создайте ветку для фичи (`git checkout -b feature/amazing-feature`)
    - Зафиксируйте изменения (`git commit -m 'Add amazing feature'`)
    - Отправьте изменения (`git push origin feature/amazing-feature`)
    - Откройте Pull Request

Пожалуйста, придерживайтесь существующего стиля кода и структуры модулей.

---

## 👥 Авторы

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/Noisy-J">
        <img src="https://github.com/Noisy-J.png" width="100px;" alt="Noisy-J"/>
        <br />
        <sub><b>Noisy-J</b></sub>
      </a>
      <br />
      <sub>Архитектура, ECS, Редактор,<br/>Система сборки</sub>
    </td>
    <td align="center">
      <a href="https://github.com/egor417">
        <img src="https://github.com/egor417.png" width="100px;" alt="egor417"/>
        <br />
        <sub><b>egor417</b></sub>
      </a>
      <br />
      <sub>Физика, игровые процессы,<br/>отладка</sub>
    </td>
  </tr>
</table>

---

## 📄 Лицензия

Проект распространяется под лицензией MIT. Подробнее в файле [LICENSE](LICENSE).

---

<p align="center">
  <b>Alternative Engine</b> — создан с ❤️ для курса ОПД<br/>
  <sub>© 2026 Noisy-J & egor417</sub>
</p>
