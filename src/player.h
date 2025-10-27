#pragma once

#include "map.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

constexpr float PLAYER_TURN_SPEED = 100.0f;

class Player {

public:
    void draw(sf::RenderTarget& target);
    void update(float delatTime, const Map& map);

    sf::Vector2f position;
    float angle;

private:
};
