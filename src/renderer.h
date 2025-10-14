#pragma once

#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "map.h"
#include "player.h"
#include <memory>

constexpr float SCREEN_W = 960.0f;
constexpr float SCREEN_H = 540.0f;

class Renderer {

public:
    Renderer() = default;

    void init();
    void draw3dView(sf::RenderTarget& target, const Player& player, const Map& map);
    // void drawRays(sf::RenderTarget& target, const Player& player, const Map& map);

private:
    sf::Texture wallTexture, skyTexture;
    std::unique_ptr<sf::Sprite> wallSprite; // sprite is constructed later;
    sf::Image floorImage;
};
