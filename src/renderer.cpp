#include "renderer.h"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "map.h"
#include <cmath>
#include <cstddef>
#include <limits>

constexpr float PI = 3.141592653589793f;
constexpr float PLAYER_FOV = 60.0f;
constexpr size_t MAX_RAYCAST_DEPTH = 64;
constexpr size_t NUM_RAYS = 600;
constexpr float COLUMN_WIDTH = SCREEN_W / (float)NUM_RAYS;

struct Ray {
    sf::Vector2f hitPosition;
    sf::Vector2u mapPosition;
    float distance;
    bool hit;
    bool isHitVertical;
};

static Ray castRay(sf::Vector2f start, float angleInDegrees, const Map& map);

void Renderer::draw3dView(sf::RenderTarget& target, const Player& player, const Map& map)
{
    sf::RectangleShape rectangle(sf::Vector2f(SCREEN_W, SCREEN_H / 2.0f));
    rectangle.setFillColor(sf::Color(100, 170, 250));
    target.draw(rectangle);

    rectangle.setPosition(sf::Vector2f { 0.0f, SCREEN_H / 2.0f });
    rectangle.setFillColor(sf::Color(70, 70, 70));
    target.draw(rectangle);

    float angle = player.angle - PLAYER_FOV / 2.0f;
    float angleIncrement = PLAYER_FOV / (float)NUM_RAYS;
    const float maxRenderDistance = MAX_RAYCAST_DEPTH * map.getCellSize();
    const float maxFogDistance = maxRenderDistance / 4.0f;

    for (size_t i = 0; i < NUM_RAYS; i++, angle += angleIncrement) {

        Ray ray = castRay(player.position, angle, map);

        if (ray.hit) {
            // take the Perpendicular not the Ipotenusa to avoil fisheye
            ray.distance *= std::cos((player.angle - angle) * PI / 180.0f);

            float wallHeight = (map.getCellSize() * SCREEN_H) / ray.distance;
            if (wallHeight > SCREEN_H) {
                wallHeight = SCREEN_H;
            }

            float brightness = 1.0f - (ray.distance / maxRenderDistance);
            if (brightness < 0.0f) {
                brightness = 0.0f;
            }

            float shade = (ray.isHitVertical ? 0.8f : 1.0f) * brightness;

            float wallOffset = SCREEN_H / 2.0f - wallHeight / 2.0f; // ( center in screen )
            sf::RectangleShape column(sf::Vector2f { COLUMN_WIDTH, wallHeight });
            column.setPosition(sf::Vector2f { i * COLUMN_WIDTH, wallOffset });

            // const sf::Color fogColor = sf::Color(100, 170, 250);
            const sf::Color fogColor = sf::Color(20, 20, 20);
            float fogPercentage = (ray.distance / maxFogDistance);
            if (fogPercentage > 1.0f) {
                fogPercentage = 1.0f;
            }

            sf::Color color = map.getGrid()[ray.mapPosition.y][ray.mapPosition.x];
            // column.setFillColor(sf::Color(color.r * shade, color.g * shade, color.b * shade)); // no fog
            color = sf::Color(color.r * shade, color.g * shade, color.b * shade);
            column.setFillColor(
                sf::Color((1.0f - fogPercentage) * color.r + fogPercentage * fogColor.r,
                    (1.0f - fogPercentage) * color.g + fogPercentage * fogColor.g,
                    (1.0f - fogPercentage) * color.b + fogPercentage * fogColor.b));

            target.draw(column);
        }
    }
}

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

    sf::Vector2u vMapPos, hMapPos;
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

        if (mapY < grid.size() && mapX < grid[mapY].size() && grid[mapY][mapX] != sf::Color::Black) {
            hit = true;
            vdist = std::sqrt((vRayPos.x - start.x) * (vRayPos.x - start.x) + (vRayPos.y - start.y) * (vRayPos.y - start.y));
            vMapPos = sf::Vector2u(mapX, mapY);
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
        offset.x = -offset.y * htan; // Xstep =  Ystep / tan(a)  [ tan(a) = ys / xs ]     (rect triangle)
    } else {
        hdof = MAX_RAYCAST_DEPTH;
    }

    for (; hdof < MAX_RAYCAST_DEPTH; hdof++) {
        int mapX = (int)(hRayPos.x / cellSize);
        int mapY = (int)(hRayPos.y / cellSize);

        if (mapY < grid.size() && mapX < grid[mapY].size() && grid[mapY][mapX] != sf::Color::Black) {
            hit = true;
            hdist = std::sqrt((hRayPos.x - start.x) * (hRayPos.x - start.x) + (hRayPos.y - start.y) * (hRayPos.y - start.y));
            hMapPos = sf::Vector2u(mapX, mapY);
            break;
        }

        hRayPos += offset;
    }

    return Ray {
        hdist < vdist ? hRayPos : vRayPos,
        hdist < vdist ? hMapPos : vMapPos,
        std::min(hdist, vdist), hit,
        vdist <= hdist
    };
}
