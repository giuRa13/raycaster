#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <string>
#include <vector>

typedef std::vector<std::vector<int>> MapGrid;

class Map {

public:
    Map(float cellSize, int width, int height);
    // Map(float cellSize, std::vector<std::vector<int>> grid);
    Map(float cellSize, const std::string& filename);

    void draw(sf::RenderTarget& target);
    void setMapCell(int x, int y, int value);

    const MapGrid& getGrid() const;
    float getCellSize() const;

private:
    MapGrid grid;
    float cellSize;
};
