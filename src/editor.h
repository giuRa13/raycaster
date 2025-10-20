#pragma once

#include "map.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class Editor {

public:
    void init(sf::RenderWindow& window);
    void run(sf::RenderWindow& window, Map& map);

    void handleEvents(const sf::Event& event);

    std::string savedFileName;

private:
    sf::RectangleShape cell;

    bool isFirstMouse {};
    sf::Vector2i lastMousePos;
    sf::View view;

    int textureN;
    int setN;

    int currentLayer;
};
