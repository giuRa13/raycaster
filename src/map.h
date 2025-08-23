#pragma once

#include "SFML/Graphics/RenderTarget.hpp"
#include <string>
#include <vector>

class Map {

public:
    Map(float cellSize, int width, int height);
    // Map(float cellSize, std::vector<std::vector<int>> grid);
    Map(float cellSize, const std::string& filename);

    void draw(sf::RenderTarget& target);

    const std::vector<std::vector<sf::Color>>& getGrid() const;
    float getCellSize() const;

private:
    std::vector<std::vector<sf::Color>> grid;
    float cellSize;
};
