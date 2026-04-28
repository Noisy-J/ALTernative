#pragma once

#include "../Scene.hpp"
#include <SFML/Graphics.hpp>
#include <functional>

// Типы коллизий
enum class CollisionType {
    None,
    Overlap,    // Пересечение
    Contains,   // Один внутри другого
    Touching    // Касание
};

// Информация о коллизии
struct CollisionInfo {
    Entity entityA;
    Entity entityB;
    CollisionType type;
    sf::Vector2f overlap;      // Вектор перекрытия
    float penetrationDepth;     // Глубина проникновения
};

class CollisionSystem {
public:
    // Проверка коллизии между двумя AABB хитбоксами
    static bool checkAABB(const sf::FloatRect& a, const sf::FloatRect& b);

    // Проверка коллизии между AABB и кругом
    static bool checkCircleAABB(const sf::Vector2f& circleCenter, float radius, const sf::FloatRect& aabb);

    // Проверка коллизии между двумя кругами
    static bool checkCircles(const sf::Vector2f& centerA, float radiusA,
        const sf::Vector2f& centerB, float radiusB);

    // Получение AABB хитбокса сущности (Box коллайдер)
    static sf::FloatRect getAABB(const Scene& scene, Entity entity);

    // Получение кругового хитбокса сущности
    static sf::Vector2f getCircleCenter(const Scene& scene, Entity entity);
    static float getCircleRadius(const Scene& scene, Entity entity);

    // Проверка коллизии между двумя сущностями
    static CollisionInfo checkCollision(const Scene& scene, Entity a, Entity b);

    // Проверка коллизий для всех сущностей и вызов callback
    static void checkAllCollisions(Scene& scene,
        std::function<void(const CollisionInfo&)> onCollision);

    // Отрисовка хитбоксов для отладки
    void drawDebug(const Scene& scene, sf::RenderTarget& target);
};