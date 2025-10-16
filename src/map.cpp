#include "map.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

Map::Map(float cellsize)
    : cellSize(cellsize)
    , grid()
{
}

Map::Map(float cellsize, int width, int height)
    : cellSize(cellsize)
    , grid(height, std::vector(width, 0))
{
}

Map::Map(float cellSize, const std::string& filename)
    : cellSize(cellSize)
{
    sf::Image image;
    if (!image.loadFromFile(filename)) {
        std::cerr << "Failed to load map image\n";
        return;
    }

    grid = std::vector(image.getSize().y, std::vector(image.getSize().x, 0));

    for (size_t y = 0; y < image.getSize().y; y++) {
        for (size_t x = 0; x < image.getSize().x; x++) {
            grid[y][x] = image.getPixel(sf::Vector2u(x, y)) == sf::Color::Black ? 0 : 1; // getPixel() returns color of pixel at given coord
        }
    }
}

void Map::draw(sf::RenderTarget& target)
{
    if (grid.empty()) {
        return;
    }

    // sf::RectangleShape background(sf::Vector2f((float)grid[0].size() * cellSize, (float)grid.size() * cellSize));
    // background.setFillColor(sf::Color(0, 255, 0));
    // target.draw(background);

    sf::RectangleShape cell(sf::Vector2f(cellSize * 0.95f, cellSize * 0.95f));

    for (size_t y = 0; y < grid.size(); y++) {
        for (size_t x = 0; x < grid.size(); x++) {
            /*if (grid[y][x] == 0) {
                cell.setFillColor(sf::Color(50, 57, 69));
            } else if (grid[y][x] == 1) {
                cell.setFillColor(sf::Color(234, 131, 165)); // 235,47,75
            }*/

            // if grid[y][x] is non-zero ---> White.  if grid[y][x] is 0 ---> Black
            cell.setFillColor(grid[y][x] ? sf::Color::White : sf::Color(70, 70, 70));
            cell.setPosition(sf::Vector2f(x, y) * cellSize + sf::Vector2f(cellSize * 0.025f, cellSize * 0.025f));

            target.draw(cell);
        }
    }
}

void Map::setMapCell(int x, int y, int value)
{
    if (y >= 0 && y < grid.size() && x >= 0 && x < grid[y].size()) {
        grid[y][x] = value;
    }
}

void Map::save(const std::filesystem::path& path)
{
    std::ofstream out { path, std::ios::out | std::ios::binary };

    if (!out.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
    }

    size_t w = grid.size();
    size_t h = grid[0].size();
    out.write(reinterpret_cast<const char*>(&w), sizeof(w));
    out.write(reinterpret_cast<const char*>(&h), sizeof(h));

    for (size_t y = 0; y < grid.size(); y++) {
        for (size_t x = 0; x < grid[y].size(); x++) {
            out.write(reinterpret_cast<const char*>(&grid[y][x]), sizeof(grid[y][x]));
        }
    }
}

void Map::load(const std::filesystem::path& path)
{
    std::ifstream in { path, std::ios::in | std::ios::binary };

    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
    }

    size_t w, h;
    in.read(reinterpret_cast<char*>(&w), sizeof(w));
    in.read(reinterpret_cast<char*>(&h), sizeof(h));

    grid = std::vector(h, std::vector(w, 0));

    for (size_t y = 0; y < grid.size(); y++) {
        for (size_t x = 0; x < grid[y].size(); x++) {
            in.read(reinterpret_cast<char*>(&grid[y][x]), sizeof(grid[y][x]));
        }
    }
}

// const std::vector<std::vector<int>>& Map::getGrid() const { return grid; }
const MapGrid& Map::getGrid() const { return grid; }

float Map::getCellSize() const { return cellSize; }
