#pragma once

#include "SFML/Graphics/RenderTarget.hpp"
#include <vector>

class Map {

public:
    Map(float cellSize, int width, int height);
    Map(float cellSize, std::vector<std::vector<int>> grid);

    void draw(sf::RenderTarget& target);

private:
    std::vector<std::vector<int>> grid;
    float cellSize;
};
