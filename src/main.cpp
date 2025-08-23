#include "SFML/System/Clock.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include "map.h"
#include "player.h"
#include "renderer.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

int main()
{
    sf::RenderWindow window { sf::VideoMode { { (int)SCREEN_W, (int)SCREEN_H } }, "Raycaster", sf::Style::Close | sf::Style::Titlebar };

    std::vector<std::vector<int>> grid = {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1 },
        { 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
        { 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1 },
        { 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1 },
        { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    };

    Map map(48.0f, RESOURCES_PATH "map.png");

    Player player;
    player.position = sf::Vector2f(50, 50);

    Renderer renderer;

    sf::Clock gameClock;

    while (window.isOpen()) {
        float deltaTime = gameClock.restart().asSeconds();

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>() | sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape))
                window.close();
        }

        player.update(deltaTime);

        window.clear();
        // map.draw(window);
        // renderer.drawRays(window, player, map);
        // player.draw(window);
        renderer.draw3dView(window, player, map);
        window.display();
    }

    return 0;
}
