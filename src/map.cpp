#include "map.h"
#include "editor.h"
#include "resources.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

size_t Map::getWidth() { return grid[0].size(); }

size_t Map::getHeight() { return grid.size(); }

void Map::draw(sf::RenderTarget& target, float cellSize, int layer, int activeLayer) const
{
    if (grid.empty())
        return;

    const auto& atlas = Resources::texturesAtlas; // 12×5 atlas
    const int atlasCols = 12;
    const int atlasRows = 5;

    const sf::Vector2u atlasSize = atlas.getSize();
    const int tileWidth = static_cast<float>(atlasSize.x) / static_cast<float>(atlasCols);
    const int tileHeight = static_cast<float>(atlasSize.y) / static_cast<float>(atlasRows);

    sf::Vector2f size { cellSize * 0.95f, cellSize * 0.95f };
    sf::Sprite sprite(atlas);
    sprite.setScale(sf::Vector2f { size.x / tileWidth, size.y / tileHeight });

    sf::RectangleShape cell(sf::Vector2f(cellSize * 0.95f, cellSize * 0.95f));

    /*for (size_t y = 0; y < grid.size(); y++) {
        for (size_t x = 0; x < grid.size(); x++) {

            // if grid[y][x] is non-zero ---> White.  if grid[y][x] is 0 ---> Black
            // cell.setFillColor(grid[y][x] ? sf::Color::White : sf::Color(70, 70, 70));
            // cell.setPosition(sf::Vector2f(x, y) * cellSize + sf::Vector2f(cellSize * 0.025f, cellSize * 0.025f));
            if (grid[y][x] > 0) {
                sprite.setTextureRect(sf::IntRect({ (grid[y][x] - 1) * textureSize, 0 },
                    { textureSize, textureSize }));
                sprite.setPosition(sf::Vector2f(x, y) * cellSize + sf::Vector2f(cellSize * 0.025f, cellSize * 0.025f));
                target.draw(sprite);
            } else {
                cell.setFillColor(sf::Color(70, 70, 70));
                cell.setPosition(sf::Vector2f(x, y) * cellSize + sf::Vector2f(cellSize * 0.025f, cellSize * 0.025f));
                target.draw(cell);
            }
        }
    }*/

    // draw the empty grid background once
    for (size_t y = 0; y < grid.size(); ++y) {
        for (size_t x = 0; x < grid[y].size(); ++x) {
            cell.setFillColor(sf::Color(70, 70, 70));
            cell.setPosition(sf::Vector2f(x, y) * cellSize + sf::Vector2f(cellSize * 0.025f, cellSize * 0.025f));
            target.draw(cell);
        }
    }

    for (int worklayer = 0; worklayer < NUM_LAYERS; ++worklayer) {
        std::uint8_t alpha = (worklayer == activeLayer) ? 255 : 50;

        for (size_t y = 0; y < grid.size(); ++y) {
            for (size_t x = 0; x < grid[y].size(); ++x) {

                const int id = grid[y][x][worklayer];

                if (id == 0)
                    continue;

                // if (id > 0) {
                int textureIndex = id - 1;
                int tileX = textureIndex % atlasCols; // column (12 % 12 = 0 --> column 0)
                int tileY = textureIndex / atlasCols; // row

                sf::IntRect rect(
                    { static_cast<int>(tileX * tileWidth), static_cast<int>(tileY * tileHeight) },
                    { static_cast<int>(std::floor(tileWidth)), static_cast<int>(std::floor(tileHeight)) });

                sprite.setTextureRect(rect);

                sprite.setPosition(
                    sf::Vector2f(x, y) * cellSize + sf::Vector2f(cellSize * 0.025f, cellSize * 0.025f));

                sprite.setColor(sf::Color(255, 255, 255, alpha));

                target.draw(sprite);

                /*} else {
                    cell.setFillColor(sf::Color(70, 70, 70));
                    cell.setPosition(
                        sf::Vector2f(x, y) * cellSize + sf::Vector2f(cellSize * 0.025f, cellSize * 0.025f));
                    target.draw(cell);
                }*/
            }
        }
    }
}

void Map::setMapCell(int x, int y, int layer, int value)
{
    if (layer < NUM_LAYERS && y >= 0 && y < grid.size() && x >= 0 && x < grid[y].size()) {
        grid[y][x][layer] = value;
    }
}

int Map::getMapCell(int x, int y, int layer) const
{
    if (layer < NUM_LAYERS && y >= 0 && y < grid.size() && x >= 0 && x < grid[y].size()) {
        return grid[y][x][layer];
    } else {
        return 0;
    }
}

void Map::save(const std::filesystem::path& path) const
{
    std::ofstream out { path, std::ios::out | std::ios::binary };

    if (!out.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
    }

    if (grid.empty()) {
        return;
    }

    size_t h = grid.size();
    size_t w = grid[0].size();
    out.write(reinterpret_cast<const char*>(&w), sizeof(w));
    out.write(reinterpret_cast<const char*>(&h), sizeof(h));

    for (size_t y = 0; y < grid.size(); y++) {
        for (size_t x = 0; x < grid[y].size(); x++) {
            out.write(reinterpret_cast<const char*>(grid[y][x].data()), sizeof(grid[y][x][0]) * NUM_LAYERS);
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

    grid = std::vector(h, std::vector(w, std::array<int, NUM_LAYERS>()));

    for (size_t y = 0; y < grid.size(); y++) {
        for (size_t x = 0; x < grid[y].size(); x++) {
            in.read(reinterpret_cast<char*>(grid[y][x].data()), sizeof(grid[y][x][0]) * NUM_LAYERS);
        }
    }
}

void Map::fill(int layer, int value)
{
    if (layer < NUM_LAYERS) {
        for (auto& column : grid) {
            for (auto& cell : column) {
                cell[layer] = value;
            }
        }
    }
}

void Map::resize(size_t width, size_t height)
{
    grid.resize(height);
    for (auto& column : grid) {
        column.resize(width);
    }
}
