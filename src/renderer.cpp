#include "renderer.h"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "map.h"
#include <cmath>
#include <cstddef>
#include <limits>

constexpr float PI = 3.141592653589793f;
constexpr float PLAYER_FOV = 60.0f;
constexpr size_t MAX_RAYCAST_DEPTH = 16;

struct Ray {
    sf::Vector2f hitPosition;
    float distance;
    bool hit;
};

static Ray castRay(sf::Vector2f start, float angleInDegrees, const Map& map);

void Renderer::drawRays(sf::RenderTarget& target, const Player& player, const Map& map)
{
    for (float angle = player.angle - PLAYER_FOV / 2.0f; angle < player.angle + PLAYER_FOV; angle += 1.0f) {

        Ray ray = castRay(player.position, angle, map);

        if (ray.hit) {

            sf::Vertex line[] = {
                sf::Vertex { player.position, sf::Color::Yellow },
                sf::Vertex { ray.hitPosition, sf::Color::Yellow },
            };

            target.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }
}

Ray castRay(sf::Vector2f start, float angleInDegrees, const Map& map) // ( start = player.position )
{
    float angle = angleInDegrees * PI / 180.0f;
    float vtan = -tan(angle); // ( tangent )
    float htan = -1.0f / tan(angle); // ( a-tangent )
    float cellSize = map.getCellSize();

    bool hit = false;

    // Depth Of Field = if horizontal shorter drow that, if vertical shorter draw that
    size_t vdof = 0, hdof = 0;

    float vdist = std::numeric_limits<float>::max();
    float hdist = std::numeric_limits<float>::max();

    sf::Vector2f vRayPos, hRayPos, offset;

    // Vertical (look right or left)
    if (cos(angle) > 0.001f) {
        vRayPos.x = std::floor(start.x / cellSize) * cellSize + cellSize;
        vRayPos.y = (start.x - vRayPos.x) * vtan + start.y;
        offset.x = cellSize;
        offset.y = -offset.x * vtan;
    } else if (cos(angle) < -0.001f) {
        vRayPos.x = std::floor(start.x / cellSize) * cellSize - 0.01f; // 0.01f = floating points calculation errors
        vRayPos.y = (start.x - vRayPos.x) * vtan + start.y;
        offset.x = -cellSize;
        offset.y = -offset.x * vtan;
    } else {
        vdof = MAX_RAYCAST_DEPTH;
    }

    const auto& grid = map.getGrid();
    for (; vdof < MAX_RAYCAST_DEPTH; vdof++) {
        int mapX = (int)(vRayPos.x / cellSize);
        int mapY = (int)(vRayPos.y / cellSize);

        if (mapY < grid.size() && mapX < grid[mapY].size() && grid[mapY][mapX]) {
            hit = true;
            vdist = std::sqrt((vRayPos.x - start.x) * (vRayPos.x - start.x) + (vRayPos.y - start.y) * (vRayPos.y - start.y));
            break;
        }

        vRayPos += offset;
    }

    // horizontal (look up or down)
    // if looking neither UP nor DOWN: looking straight left or right (dont need to cast ray cause never hit an horizontal wall)
    if (sin(angle) > 0.001f) {
        hRayPos.y = std::floor(start.y / cellSize) * cellSize + cellSize; // Y nearest
        hRayPos.x = (start.y - hRayPos.y) * htan + start.x; // X nearest
        offset.y = cellSize;
        offset.x = -offset.y * htan;
    } else if (sin(angle) < -0.001f) {
        hRayPos.y = std::floor(start.y / cellSize) * cellSize - 0.01f; // Y-nearest = player.y - (player.y / cellsize) * cellsize
        hRayPos.x = (start.y - hRayPos.y) * htan + start.x; // X-nearest = Ynearest / tan(a)  [ tan(a) = yn / xn]
        offset.y = -cellSize; // Ystep = cellsize
        offset.x = -offset.y * htan; // Xstep =  Ystep / tan(a)  [ tan(a) = ys / xs ]
    } else {
        hdof = MAX_RAYCAST_DEPTH;
    }

    for (; hdof < MAX_RAYCAST_DEPTH; hdof++) {
        int mapX = (int)(hRayPos.x / cellSize);
        int mapY = (int)(hRayPos.y / cellSize);

        if (mapY < grid.size() && mapX < grid[mapY].size() && grid[mapY][mapX]) {
            hit = true;
            hdist = std::sqrt((hRayPos.x - start.x) * (hRayPos.x - start.x) + (hRayPos.y - start.y) * (hRayPos.y - start.y));
            break;
        }

        hRayPos += offset;
    }

    return Ray {
        sf::Vector2f { hdist < vdist ? hRayPos : vRayPos },
        std::min(hdist, vdist),
        hit
    };
}
