#pragma once

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "map.h"
#include "player.h"

constexpr float SCREEN_W = 1200.0f;
constexpr float SCREEN_H = 675.0f;

class Renderer {

public:
    Renderer() = default;

    void init();
    void draw3dView(sf::RenderTarget& target, const Player& player, const Map& map);
    // void drawRays(sf::RenderTarget& target, const Player& player, const Map& map);

private:
    sf::Texture wallTexture;
    std::unique_ptr<sf::Sprite> wallSprite; // sprite is constructed later;
};
