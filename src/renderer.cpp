#include "renderer.h"
#include "player.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
// #include "SFML/Graphics/Rect.hpp"
#include "map.h"
#include "resources.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <iostream>

constexpr float PI = 3.141592653589793f;
constexpr float PLAYER_FOV = 60.0f;
constexpr float CAMERA_Z = 0.5f * SCREEN_H; // ( vertical cam position in 3D world)
constexpr size_t MAX_RAYCAST_DEPTH = 64;
// constexpr size_t NUM_RAYS = 600;
// constexpr float COLUMN_WIDTH = SCREEN_W / (float)NUM_RAYS;

struct Ray {
    sf::Vector2f hitPosition;
    sf::Vector2u mapPosition;
    float distance;
    bool hit;
    bool isHitVertical;
};

// static Ray castRay(sf::Vector2f start, float angleInDegrees, const Map& map);

void Renderer::init()
{
    if (!skyTexture.loadFromFile(RESOURCES_PATH "sky_texture.png")) {
        std::cerr << "Failed to load sky_texture.png\n";
    }
    skyTexture.setRepeated(true);

    // wallSprite = std::make_unique<sf::Sprite>(Resources::texturesAtlas);

    screenTexture = sf::Texture(sf::Vector2u((unsigned)SCREEN_W, (unsigned)SCREEN_H));
    screenSprite = std::make_unique<sf::Sprite>(screenTexture);

    // Pixel buffer for floor (RGBA)
    screenPixels.resize((size_t)SCREEN_W * (size_t)SCREEN_H * 4, 0);
}

void Renderer::draw3dView(sf::RenderTarget& target, const Player& player, const Map& map)
{
    // Clear screen pixel buffer
    std::fill(screenPixels.begin(), screenPixels.end(), 0);

    const sf::Color fogColor = sf::Color(20, 20, 20);

    // Player direction and camera plane
    float radians = player.angle * PI / 180.0f;
    sf::Vector2f direction { std::cos(radians), std::sin(radians) }; // convert radians to a direction vector
    sf::Vector2f plane { -direction.y, direction.x * 0.66f }; // camera plane (perpendicular to the direction) (horizontal line in front of player and each screen column(y) is a point on it)
    sf::Vector2f position = player.position / map.getCellSize();

    // Sky texture
    int xOffset = SCREEN_W / PLAYER_TURN_SPEED * player.angle;
    while (xOffset < 0)
        xOffset += skyTexture.getSize().x;

    sf::Vertex sky[] = {
        sf::Vertex { { 0.f, 0.f }, sf::Color::White, { (float)xOffset, 0.f } },
        sf::Vertex { { 0.f, SCREEN_H / 2.f }, sf::Color::White, { (float)xOffset, (float)skyTexture.getSize().y } },
        sf::Vertex { { SCREEN_W, SCREEN_H / 2.f }, sf::Color::White, { (float)xOffset + (float)skyTexture.getSize().x, (float)skyTexture.getSize().y } },

        sf::Vertex { { 0.f, 0.f }, sf::Color::White, { (float)xOffset, 0.f } },
        sf::Vertex { { SCREEN_W, SCREEN_H / 2.f }, sf::Color::White, { (float)xOffset + (float)skyTexture.getSize().x, (float)skyTexture.getSize().y } },
        sf::Vertex { { SCREEN_W, 0.f }, sf::Color::White, { (float)xOffset + (float)skyTexture.getSize().x, 0.f } },
    };
    target.draw(sky, 6, sf::PrimitiveType::Triangles, sf::RenderStates(&skyTexture));

    // Floor and Ceiling Rendering
    for (size_t y = SCREEN_H / 2 + 1; y < SCREEN_H; y++) { // FLOOR: start from the first row *below* the horizon down to bottom screen
        sf::Vector2f rayDirLeft = direction - plane;
        sf::Vector2f rayDirRight = direction + plane;

        float denom = static_cast<float>(y) - SCREEN_H / 2.0f; // vertical offset from the horizon
        if (denom == 0.0f)
            continue;
        float rowDistance = CAMERA_Z / denom; // distance from Camera to Floor (at current row)

        sf::Vector2f floorStep = rowDistance * (rayDirRight - rayDirLeft) / static_cast<float>(SCREEN_W);
        sf::Vector2f floorPos = position + rowDistance * rayDirLeft;

        // Atlas layout
        const int atlasCols = 12;
        const int atlasRows = 5;
        const auto& texImg = Resources::texturesImage;
        const sf::Vector2u texSize = texImg.getSize();
        const unsigned tileWidth = texSize.x / atlasCols;
        const unsigned tileHeight = texSize.y / atlasRows;

        for (size_t x = 0; x < SCREEN_W; x++) {
            int fx = static_cast<int>(floorPos.x);
            int fy = static_cast<int>(floorPos.y);

            int floorTex = map.getMapCell(fx, fy, Map::LAYER_FLOOR);
            int ceilTex = map.getMapCell(fx, fy, Map::LAYER_CEILING);

            sf::Color floorColor(70, 70, 70, 255);
            sf::Color ceilingColor(0, 0, 0, 0);

            float fracX = floorPos.x - std::floor(floorPos.x); // if floorPos.x = 2.25 --> std::floor = 2 --> fracX = 0.25 (you are 25% inside tile #2 along the x-axis)
            float fracY = floorPos.y - std::floor(floorPos.y);
            // Convert fractional position into texture pixel coordinates
            unsigned tx = static_cast<unsigned>(fracX * tileWidth);
            unsigned ty = static_cast<unsigned>(fracY * tileHeight);

            // Floor sampling
            if (floorTex > 0) {
                unsigned tileIndex = static_cast<unsigned>(floorTex - 1);
                unsigned tileX = tileIndex % atlasCols; // get columns number ( 12 % 12 = 0 , column 0 )
                unsigned tileY = tileIndex / atlasCols;
                unsigned px = tileX * tileWidth + tx;
                unsigned py = tileY * tileHeight + ty;
                if (px < texSize.x && py < texSize.y)
                    floorColor = texImg.getPixel({ px, py });
            }

            // Ceiling sampling
            if (ceilTex > 0) {
                unsigned tileIndex = static_cast<unsigned>(ceilTex - 1);
                unsigned tileX = tileIndex % atlasCols;
                unsigned tileY = tileIndex / atlasCols;
                unsigned px = tileX * tileWidth + tx;
                unsigned py = tileY * tileHeight + ty;
                if (px < texSize.x && py < texSize.y)
                    ceilingColor = texImg.getPixel({ px, py });
            }

            // Write to screen buffer
            // Pixel 0 → [R0, G0, B0, A0]
            // Pixel 1 → [R1, G1, B1, A1]
            // Pixel 2 → [R2, G2, B2, A2]
            // because sf::Texture::update() expects a contiguous RGBA array, like: [R0, G0, B0, A0,  R1, G1, B1, A1,  R2, G2, B2, A2,  ...]
            size_t floorIdx = (x + y * (size_t)SCREEN_W) * 4; // Start of the pixel in the floor area.  ( 4 --> RGBA)
            size_t ceilIdx = (x + ((size_t)SCREEN_H - y - 1) * (size_t)SCREEN_W) * 4; // (SCREEN_H - y - 1) mirror the y-coord vertically

            screenPixels[floorIdx + 0] = floorColor.r;
            screenPixels[floorIdx + 1] = floorColor.g;
            screenPixels[floorIdx + 2] = floorColor.b;
            screenPixels[floorIdx + 3] = floorColor.a;

            screenPixels[ceilIdx + 0] = ceilingColor.r;
            screenPixels[ceilIdx + 1] = ceilingColor.g;
            screenPixels[ceilIdx + 2] = ceilingColor.b;
            screenPixels[ceilIdx + 3] = ceilingColor.a;

            floorPos += floorStep;
        }
    }

    // Update screen texture
    screenTexture.update(screenPixels.data());
    target.draw(*screenSprite);

    // Wall Rendering
    for (size_t i = 0; i < SCREEN_W; i++) {
        sf::VertexArray walls { sf::PrimitiveType::Lines };

        // column in camera space (-1 to 1)
        float cameraX = i * 2.0f / SCREEN_W - 1.0f; // i = 1200 -> 1200/1200 -> 1*2 -> 2-1 -> 1
        sf::Vector2f rayPos = position;
        sf::Vector2f rayDir = direction + plane * cameraX; // direction the ray supposed to move

        // in DDA only move the amount needed to get to the next side of the wall ( from one X/Y side to the next X/Y side)
        // no need the correct lenght, in DDA only ratio (between X and Y) matters
        sf::Vector2f deltaDist { std::abs(1.0f / rayDir.x), std::abs(1.0f / rayDir.y) };
        sf::Vector2i mapPos { rayPos }; // truncate floating point (get Integer)
        sf::Vector2i step;
        sf::Vector2f sideDist; // make shure we start at a right value X/Y side (initial distance might not be at the wall)

        if (rayDir.x < 0.0f) {
            step.x = -1;
            sideDist.x = (-mapPos.x + rayPos.x) * deltaDist.x; // perpendicular distance * delta = actual distance
        } else {
            step.x = 1;
            sideDist.x = (mapPos.x - rayPos.x + 1.0f) * deltaDist.x;
        }
        if (rayDir.y < 0.0f) {
            step.y = -1;
            sideDist.y = (-mapPos.y + rayPos.y) * deltaDist.y;
        } else {
            step.y = 1;
            sideDist.y = (mapPos.y - rayPos.y + 1.0f) * deltaDist.y;
        }

        // DDA
        int hit {}, isHitVertical {};
        size_t depth = 0;
        while (hit == 0 && depth < MAX_RAYCAST_DEPTH) {
            if (sideDist.x < sideDist.y) {
                sideDist.x += deltaDist.x;
                mapPos.x += step.x;
                isHitVertical = false;
            } else {
                sideDist.y += deltaDist.y;
                mapPos.y += step.y;
                isHitVertical = true;
            }
            hit = map.getMapCell(mapPos.x, mapPos.y, Map::LAYER_WALLS);
            depth++;
        }

        const int atlasCols = 12;
        const int atlasRows = 5;
        const auto& atlas = Resources::texturesAtlas;
        const sf::Vector2u atlasSize = atlas.getSize();
        const float tileWidth = static_cast<float>(atlasSize.x) / atlasCols;
        const float tileHeight = static_cast<float>(atlasSize.y) / atlasRows;

        if (hit > 0) {
            float perpWallDist = isHitVertical ? sideDist.y - deltaDist.y : sideDist.x - deltaDist.x;
            float wallHeight = SCREEN_H / perpWallDist;
            float wallStart = (-wallHeight + SCREEN_H) / 2.0f;
            float wallEnd = (wallHeight + SCREEN_H) / 2.0f;

            float wallX = isHitVertical ? rayPos.x + perpWallDist * rayDir.x : rayPos.y + perpWallDist * rayDir.y;
            // to calculate position relative to the current wall starting position :
            wallX -= std::floor(wallX); // if hit at middle of second tile ->  wallX = 1.5 -> - 1(floor of 1.5) = 0.5
            float textureX = wallX * tileWidth; // float textureX = wallX * textureSize;

            float brightness = 1.0f - (perpWallDist / static_cast<float>(MAX_RAYCAST_DEPTH));
            if (isHitVertical)
                brightness *= 0.7f;
            sf::Color color = sf::Color(255 * brightness, 255 * brightness, 255 * brightness);

            // choose which tile (based on your map hit ID)
            int textureIndex = hit - 1;
            int tileX = textureIndex % atlasCols;
            int tileY = textureIndex / atlasCols;

            float uOffset = tileX * tileWidth;
            float vOffset = tileY * tileHeight;

            walls.append(sf::Vertex { { (float)i, wallStart }, color, { uOffset + textureX, vOffset } });
            walls.append(sf::Vertex { { (float)i, wallEnd }, color, { uOffset + textureX, vOffset + tileHeight } });

            sf::RenderStates states { &Resources::texturesAtlas };
            target.draw(walls, states);
        }
    }
}
/*void Renderer::drawRays(sf::RenderTarget& target, const Player& player, const Map& map)
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
}*/

/*Ray castRay(sf::Vector2f start, float angleInDegrees, const Map& map) // ( start = player.position )
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
}*/
