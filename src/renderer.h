#pragma once

#include "SFML/Graphics/RenderTarget.hpp"
#include "map.h"
#include "player.h"

class Renderer {

public:
    void drawRays(sf::RenderTarget& target, const Player& player, const Map& map);

private:
};
