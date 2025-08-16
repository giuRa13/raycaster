#include "player.h"
#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/System/Angle.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Keyboard.hpp"
#include <cmath>

constexpr float PI = 3.141592653589793f;
constexpr float TURN_SPEED = 100.0f;
constexpr float MOVE_SPEED = 100.0f;

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

void Player::update(float deltaTime)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
        angle -= TURN_SPEED * deltaTime;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
        angle += TURN_SPEED * deltaTime;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
        float radians = angle * PI / 180.0f;
        position.x += cos(radians) * MOVE_SPEED * deltaTime;
        position.y += sin(radians) * MOVE_SPEED * deltaTime;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
        float radians = angle * PI / 180.0f;
        position.x -= cos(radians) * MOVE_SPEED * deltaTime;
        position.y -= sin(radians) * MOVE_SPEED * deltaTime;
    }
}
