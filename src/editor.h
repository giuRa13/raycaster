#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class Editor {

public:
    void init(sf::RenderWindow& window);
    void run(sf::RenderWindow& window);

    void handleEvents(const sf::Event& event);

private:
    bool isFirstMouse {};
    sf::Vector2i lastMousePos;
    sf::View view;
};
