#include "renderer.h"
#include "player.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
// #include "SFML/Graphics/Rect.hpp"
#include "map.h"
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
// #include <limits>

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

    if (!wallTexture.loadFromFile(RESOURCES_PATH "wall_texture.png")) {
        std::cerr << "Failed to load wall_texture.png\n";
    }

    if (wallTexture.getSize().x != wallTexture.getSize().y) {
        std::cerr << "ERROR: Texture is not square\n";
    }

    if (!floorImage.loadFromFile(RESOURCES_PATH "floor_texture.png")) {
        std::cerr << "Failed to load floor_texture.png\n";
    }

    if (floorImage.getSize().x != floorImage.getSize().y) {
        std::cerr << "ERROR: Texture is not square\n";
    }

    wallSprite = std::make_unique<sf::Sprite>(wallTexture);

    screenTexture = sf::Texture(sf::Vector2u((unsigned)SCREEN_W, (unsigned)SCREEN_H));
    screenSprite = std::make_unique<sf::Sprite>(screenTexture);

    // Pixel buffer for floor (RGBA)
    screenPixels.resize((size_t)SCREEN_W * (size_t)SCREEN_H * 4, 0);
}

void Renderer::draw3dView(sf::RenderTarget& target, const Player& player, const Map& map)
{
    // sf::Image image(sf::Vector2u(SCREEN_W, SCREEN_H), sf::Color(0, 0, 0, 0)); // alpha transparent avoid hide sky
    //  Clear lower half (floor area)
    std::fill(screenPixels.begin(), screenPixels.end(), 0);

    // sf::RectangleShape rectangle(sf::Vector2f(SCREEN_W, SCREEN_H / 2.0f));
    // rectangle.setFillColor(sf::Color(100, 170, 250));
    // target.draw(rectangle);

    // rectangle.setPosition(sf::Vector2f { 0.0f, SCREEN_H / 2.0f });
    // rectangle.setFillColor(sf::Color(70, 70, 70));
    // target.draw(rectangle);

    // const sf::Color fogColor = sf::Color(100, 170, 250);
    const sf::Color fogColor = sf::Color(20, 20, 20);
    // const float maxRenderDistance = MAX_RAYCAST_DEPTH * map.getCellSize();
    // const float maxFogDistance = maxRenderDistance / 4.0f;

    // float angle = player.angle - PLAYER_FOV / 2.0f;
    // float angleIncrement = PLAYER_FOV / (float)NUM_RAYS;

    // sf::RectangleShape column { sf::Vector2f { 1.0f, 1.0f } };

    float radians = player.angle * PI / 180.0f;
    sf::Vector2f direction { std::cos(radians), std::sin(radians) }; // convert radians to a direction vector
    sf::Vector2f plane { -direction.y, direction.x * 0.66f }; // camera plane (perpendicular to the direction)
    sf::Vector2f position = player.position / map.getCellSize();

    // sky texture
    /*int xOffset = (int)(player.angle * (skyTexture.getSize().x / 360.f)); // sroll sky texture
    xOffset %= (int)skyTexture.getSize().x;
    if (xOffset < 0)
        xOffset += skyTexture.getSize().x;*/
    int xOffset = SCREEN_W / PLAYER_TURN_SPEED * player.angle;
    while (xOffset < 0) {
        xOffset += skyTexture.getSize().x;
    }

    sf::Vertex sky[] = {
        // first triangle
        sf::Vertex { sf::Vector2f { 0.f, 0.f }, sf::Color::White, sf::Vector2f { (float)xOffset, 0.f } },
        sf::Vertex { sf::Vector2f { 0.f, SCREEN_H / 2.f }, sf::Color::White, sf::Vector2f { (float)xOffset, (float)skyTexture.getSize().y } },
        sf::Vertex { sf::Vector2f { SCREEN_W, SCREEN_H / 2.f }, sf::Color::White, { (float)xOffset + (float)skyTexture.getSize().x, (float)skyTexture.getSize().y } },
        // second triangle
        sf::Vertex { sf::Vector2f { 0.f, 0.f }, sf::Color::White, sf::Vector2f { (float)xOffset, 0.f } },
        sf::Vertex { sf::Vector2f { SCREEN_W, SCREEN_H / 2.f }, sf::Color::White, sf::Vector2f { (float)xOffset + (float)skyTexture.getSize().x, (float)skyTexture.getSize().y } },
        sf::Vertex { sf::Vector2f { SCREEN_W, 0.f }, sf::Color::White, sf::Vector2f { (float)xOffset + (float)skyTexture.getSize().x, 0.f } },
    };
    target.draw(sky, 6, sf::PrimitiveType::Triangles, sf::RenderStates(&skyTexture));

    // floor textures
    const auto floorSize = floorImage.getSize().x;
    for (size_t y = (int)SCREEN_H / 2 + 1; y < SCREEN_H; y++) { // FLOOR: start from the first row *below* the horizon to avoid division by zero
        sf::Vector2f rayDirLeft { direction - plane }, rayDirRight { direction + plane };
        float denom = (float)y - (SCREEN_H / 2.0f);
        if (denom == 0.0f)
            continue;
        float rowDistance = CAMERA_Z / denom; // distance from Camera to Floor (at current row)
                                              //
        sf::Vector2f floorStep = rowDistance * (rayDirRight - rayDirLeft) / (float)SCREEN_W;
        sf::Vector2f floorPos = position + rowDistance * rayDirLeft;

        for (size_t x = 0; x < SCREEN_W; x++) {
            sf::Vector2i cell { (int)std::floor(floorPos.x), (int)std::floor(floorPos.y) };

            float textureSize = floorImage.getSize().x;
            sf::Vector2i texCoords { (int)(textureSize * (floorPos.x - (float)cell.x)), (int)(textureSize * (floorPos.y - (float)cell.y)) };
            texCoords.x &= (int)textureSize - 1;
            texCoords.y &= (int)textureSize - 1;

            sf::Color c = floorImage.getPixel((sf::Vector2u)texCoords);
            size_t idx = (x + y * (size_t)SCREEN_W) * 4;
            screenPixels[idx + 0] = c.r;
            screenPixels[idx + 1] = c.g;
            screenPixels[idx + 2] = c.b;
            screenPixels[idx + 3] = 255;
            floorPos += floorStep;
        }
    }

    // Sky
    /*for (size_t y = 0; y < SCREEN_H / 2; ++y) {
        for (size_t x = 0; x < SCREEN_W; ++x)
            image.setPixel({ (unsigned)x, (unsigned)y }, sf::Color(100, 170, 250));
    }*/

    // Update the persistent screen sprite with the new image
    screenTexture.update(screenPixels.data());
    // Draw the screen sprite
    target.draw(*screenSprite);

    sf::VertexArray walls { sf::PrimitiveType::Lines };

    // loop every column not angles (each column is one pixel)
    for (size_t i = 0; i < SCREEN_W; i++) {
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

        if (rayDir.x < 0.0f) { // left
            step.x = -1;
            sideDist.x = (-mapPos.x + rayPos.x) * deltaDist.x; // perpendicular distance * delta = actual distance
        } else { // right
            step.x = 1;
            sideDist.x = (mapPos.x - rayPos.x + 1.0f) * deltaDist.x;
        }

        if (rayDir.y < 0.0f) { // down
            step.y = -1;
            sideDist.y = (-mapPos.y + rayPos.y) * deltaDist.y;
        } else { // up
            step.y = 1;
            sideDist.y = (mapPos.y - rayPos.y + 1.0f) * deltaDist.y;
        }

        // DDA
        bool didHit {}, isHitVertical {};
        size_t depth = 0;
        while (!didHit && depth < MAX_RAYCAST_DEPTH) {

            if (sideDist.x < sideDist.y) { // Horizontal wall
                sideDist.x += deltaDist.x;
                mapPos.x += step.x;
                isHitVertical = false;
            } else {
                sideDist.y += deltaDist.y;
                mapPos.y += step.y;
                isHitVertical = true;
            }

            int x = mapPos.x, y = mapPos.y;
            const auto& grid = map.getGrid();

            if (y >= 0 && y < (int)grid.size() && x >= 0 && x < (int)grid[y].size()) {
                if (grid[y][x]) { // grid[y][x] != sf::Color::Black
                    didHit = true;
                }
            }

            depth++;
        }

        // Drawing
        if (didHit) {
            float perpWallDist = isHitVertical ? sideDist.y - deltaDist.y : sideDist.x - deltaDist.x; // perpendicular distance to avoid fisheye
            float wallHeight = SCREEN_H / perpWallDist;

            float wallStart = (-wallHeight + SCREEN_H) / 2.0f;
            float wallEnd = (wallHeight + SCREEN_H) / 2.0f;

            // texture coords
            float textureSize = wallTexture.getSize().x;
            float wallX = isHitVertical ? rayPos.x + perpWallDist * rayDir.x : rayPos.y + perpWallDist * rayDir.y;
            // to calculate position relative to the current wall starting position :
            wallX -= std::floor(wallX); // if hit at middle of second tile ->  wallX = 1.5 -> - 1(floor of 1.5) = 0.5
            float textureX = wallX * textureSize;

            float brightness = 1.0f - (perpWallDist / (float)MAX_RAYCAST_DEPTH);
            if (isHitVertical) {
                brightness *= 0.7f;
            }
            sf::Color color = sf::Color(255 * brightness, 255 * brightness, 255 * brightness);

            walls.append(sf::Vertex { sf::Vector2f { (float)i, wallStart }, color, sf::Vector2f { textureX, 0.0f } });
            walls.append(sf::Vertex { sf::Vector2f { (float)i, wallEnd }, color, sf::Vector2f { textureX, textureSize } });

            sf::RenderStates states { &wallTexture };
            target.draw(walls, states);
        }
    }

    /*for (size_t i = 0; i < NUM_RAYS; i++, angle += angleIncrement) {

        Ray ray = castRay(player.position, angle, map);

        if (ray.hit) {
            // take the Perpendicular not the Ipotenusa to avoil fisheye
            ray.distance *= std::cos((player.angle - angle) * PI / 180.0f);

            float wallHeight = (map.getCellSize() * SCREEN_H) / ray.distance;
            float wallOffset = SCREEN_H / 2.0f - wallHeight / 2.0f; // ( center in screen )

            // map texture
            float textureX;
            if (ray.isHitVertical) {
                textureX = ray.hitPosition.y - wallTexture.getSize().x * std::floor(ray.hitPosition.y / wallTexture.getSize().x);
            } else {
                textureX = wallTexture.getSize().x * std::ceil(ray.hitPosition.x / wallTexture.getSize().x) - ray.hitPosition.x;
            }

            wallSprite->setPosition(sf::Vector2f { i * COLUMN_WIDTH, wallOffset });
            wallSprite->setTextureRect(sf::IntRect(
                { (int)textureX, 0 },
                { (int)wallTexture.getSize().x / (int)map.getCellSize(), (int)wallTexture.getSize().y }));
            wallSprite->setScale(sf::Vector2f {
                COLUMN_WIDTH, wallHeight / wallTexture.getSize().y });

            if (wallHeight > SCREEN_H) {
                wallHeight = SCREEN_H;
            }

            float brightness = 1.0f - (ray.distance / maxRenderDistance);
            if (brightness < 0.0f) {
                brightness = 0.0f;
            }

            float shade = (ray.isHitVertical ? 0.8f : 1.0f) * brightness;

            float fogPercentage = (ray.distance / maxFogfDistance);
            if (fogPercentage > 1.0f) {
                fogPercentage = 1.0f;
            }

            // fog effect
            column.setPosition(sf::Vector2f { i * COLUMN_WIDTH, wallOffset });
            column.setScale(sf::Vector2f { COLUMN_WIDTH, wallHeight });
            column.setFillColor(sf::Color(fogColor.r, fogColor.g, fogColor.b, fogPercentage * 255));

            wallSprite->setColor(sf::Color(255 * shade, 255 * shade, 255 * shade));
            target.draw(*wallSprite);
            target.draw(column);
        }
    }*/
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
