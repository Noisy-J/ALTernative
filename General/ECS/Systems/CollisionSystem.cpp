#include "CollisionSystem.hpp"
#include <cmath>
#include <iostream>

// Проверка AABB коллизии
bool CollisionSystem::checkAABB(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

// Проверка коллизии круг-AABB
bool CollisionSystem::checkCircleAABB(const sf::Vector2f& circleCenter, float radius, const sf::FloatRect& aabb) {
    // Находим ближайшую точку на AABB к центру круга
    float closestX = std::max(aabb.position.x, std::min(circleCenter.x, aabb.position.x + aabb.size.x));
    float closestY = std::max(aabb.position.y, std::min(circleCenter.y, aabb.position.y + aabb.size.y));

    // Вычисляем расстояние
    float dx = circleCenter.x - closestX;
    float dy = circleCenter.y - closestY;

    return (dx * dx + dy * dy) < (radius * radius);
}

// Проверка коллизии круг-круг
bool CollisionSystem::checkCircles(const sf::Vector2f& centerA, float radiusA,
    const sf::Vector2f& centerB, float radiusB) {
    float dx = centerB.x - centerA.x;
    float dy = centerB.y - centerA.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    return distance < (radiusA + radiusB);
}

// Получение AABB для Box коллайдера
sf::FloatRect CollisionSystem::getAABB(const Scene& scene, Entity entity) {
    auto& transform = scene.transforms.at(entity);
    auto& collider = scene.colliders.at(entity);

    sf::Vector2f pos = transform.Pos + collider.ColliderPosition;
    sf::Vector2f halfSize = collider.ColliderSize / 2.0f;

    return sf::FloatRect(
        pos - halfSize,
        collider.ColliderSize
    );
}

// Получение центра круга
sf::Vector2f CollisionSystem::getCircleCenter(const Scene& scene, Entity entity) {
    auto& transform = scene.transforms.at(entity);
    auto& collider = scene.colliders.at(entity);

    return transform.Pos + collider.ColliderPosition;
}

// Получение радиуса круга
float CollisionSystem::getCircleRadius(const Scene& scene, Entity entity) {
    return scene.colliders.at(entity).ColliderRadius;
}

// Проверка коллизии между двумя сущностями
CollisionInfo CollisionSystem::checkCollision(const Scene& scene, Entity a, Entity b) {
    CollisionInfo info;
    info.entityA = a;
    info.entityB = b;
    info.type = CollisionType::None;

    if (!scene.isValid(a) || !scene.isValid(b)) return info;
    if (scene.colliders.find(a) == scene.colliders.end() ||
        scene.colliders.find(b) == scene.colliders.end()) return info;

    auto& colA = scene.colliders.at(a);
    auto& colB = scene.colliders.at(b);

    // Получаем AABB для обоих
    sf::FloatRect aabbA = getAABB(scene, a);
    sf::FloatRect aabbB = getAABB(scene, b);

    // Проверяем пересечение AABB
    auto intersection = aabbA.findIntersection(aabbB);

    if (intersection.has_value()) {
        info.type = CollisionType::Overlap;
        info.overlap = sf::Vector2f(intersection->size.x, intersection->size.y);
        info.penetrationDepth = std::min(intersection->size.x, intersection->size.y);
    }

    return info;
}

// Проверка всех коллизий на сцене
void CollisionSystem::checkAllCollisions(Scene& scene,
    std::function<void(const CollisionInfo&)> onCollision) {
    std::vector<Entity> entities;
    for (auto& [entity, _] : scene.transforms) {
        if (scene.colliders.find(entity) != scene.colliders.end()) {
            entities.push_back(entity);
        }
    }

    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            CollisionInfo info = checkCollision(scene, entities[i], entities[j]);
            if (info.type != CollisionType::None) {
                onCollision(info);
            }
        }
    }
}

// Отрисовка хитбоксов для отладки
void CollisionSystem::drawDebug(const Scene& scene, sf::RenderTarget& target) {
    for (auto& [entity, collider] : scene.colliders) {
        if (!collider.drawDebug) continue;  // Пропускаем если отладка выключена
        if (scene.transforms.find(entity) == scene.transforms.end()) continue;

        auto& transform = scene.transforms.at(entity);

        // Прямоугольный хитбокс
        sf::RectangleShape rect(collider.ColliderSize);
        rect.setOrigin(collider.ColliderSize / 2.0f);
        rect.setPosition(transform.Pos + collider.ColliderPosition);
        rect.setRotation(collider.ColliderRotation);

        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(collider.debugColor);
        rect.setOutlineThickness(2.0f);

        target.draw(rect);

        // Круг радиуса (для круглых коллайдеров)
        sf::CircleShape radiusCircle(collider.ColliderRadius);
        radiusCircle.setOrigin({ collider.ColliderRadius, collider.ColliderRadius });
        radiusCircle.setPosition(transform.Pos + collider.ColliderPosition);
        radiusCircle.setFillColor(sf::Color::Transparent);
        radiusCircle.setOutlineColor(sf::Color(255, 255, 0, 100));
        radiusCircle.setOutlineThickness(1.0f);

        target.draw(radiusCircle);

        // Точка центра
        sf::CircleShape centerPoint(3.0f);
        centerPoint.setOrigin({ 3.0f, 3.0f });
        centerPoint.setPosition(transform.Pos + collider.ColliderPosition);
        centerPoint.setFillColor(sf::Color::Yellow);

        target.draw(centerPoint);
    }
}