#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <filesystem>
#include <string>
#include <vector>

class Map {

public:
    static constexpr int LAYER_WALLS = 0;
    static constexpr int LAYER_FLOOR = 1;
    static constexpr int LAYER_CEILING = 2;
    static constexpr int NUM_LAYERS = 3;

    static constexpr const char* LAYER_NAMES[NUM_LAYERS] = {
        "Walls",
        "Floor",
        "Ceiling",
    };

    Map(float cellSize);
    // Map(float cellSize, int width, int height);
    // Map(float cellSize, const std::string& filename);

    void draw(sf::RenderTarget& target, int layer, int activeLayer) const;
    void load(const std::filesystem::path& path);
    void save(const std::filesystem::path& path) const;

    void setMapCell(int x, int y, int layer, int value);
    void fill(int layer, int value);
    void resize(size_t width, size_t height);

    float getCellSize() const;
    int getMapCell(int x, int y, int layer) const;
    size_t getWidth();
    size_t getHeight();

private:
    std::vector<std::vector<std::array<int, NUM_LAYERS>>> grid;
    float cellSize;
};
