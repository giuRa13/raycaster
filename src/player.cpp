#include "player.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

constexpr float PI = 3.141592653589793f;
constexpr float TURN_SPEED = PLAYER_TURN_SPEED;
constexpr float MOVE_SPEED = 2.5f;
constexpr float PLAYER_SIZE = 0.2f;

void Player::draw(sf::RenderTarget& target)
{
    sf::CircleShape circle(5.0f);
    circle.setOrigin(sf::Vector2f(circle.getRadius(), circle.getRadius()));
    circle.setPosition(position);
    circle.setFillColor(sf::Color(51, 153, 255));

    sf::RectangleShape line(sf::Vector2f(10.0f, 2.0f));
    line.setPosition(position);
    line.setRotation(sf::degrees(angle));
    line.setFillColor(sf::Color(51, 153, 255));

    target.draw(circle);
    target.draw(line);
}

void Player::update(float deltaTime, const Map& map)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
        angle -= TURN_SPEED * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
        angle += TURN_SPEED * deltaTime;
    }

    float radians = angle * PI / 180.0f;
    sf::Vector2f move {};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
        move.x += cos(radians);
        move.y += sin(radians);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
        move.x -= cos(radians);
        move.y -= sin(radians);
    }

    float xOffset = move.x > 0.0f ? PLAYER_SIZE : -PLAYER_SIZE;
    float yOffset = move.y > 0.0f ? PLAYER_SIZE : -PLAYER_SIZE;
    move *= MOVE_SPEED * deltaTime;

    if (map.getMapCell(position.x + move.x + xOffset, position.y, Map::LAYER_WALLS) == 0) {
        position.x += move.x;
    }
    if (map.getMapCell(position.x, position.y + move.y + yOffset, Map::LAYER_WALLS) == 0) {
        position.y += move.y;
    }
}
