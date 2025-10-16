#include "editor.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <imgui.h>

void Editor::init(sf::RenderWindow& window)
{
    view = window.getView();
    cell.setFillColor(sf::Color::Green);
}

void Editor::run(sf::RenderWindow& window, Map& map)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {

        if (isFirstMouse) {
            lastMousePos = mousePos;
            isFirstMouse = false;
        } else {
            sf::Vector2i mouseDelta = mousePos - lastMousePos;

            view.setCenter(view.getCenter() - (sf::Vector2f)mouseDelta);
            sf::Mouse::setPosition(lastMousePos, window);
        }
        window.setMouseCursorVisible(false);
    } else {

        isFirstMouse = true;
        window.setMouseCursorVisible(true);
    }

    if (!ImGui::GetIO().WantCaptureMouse) {

        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos); // PixelToCoords: convert a point from target coords to world coords using the current view
        sf::Vector2i mapPos = (sf::Vector2i)(worldPos / map.getCellSize());
        cell.setSize(sf::Vector2f(map.getCellSize(), map.getCellSize()));
        cell.setPosition((sf::Vector2f)mapPos * map.getCellSize());
        window.draw(cell);

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            map.setMapCell(mapPos.x, mapPos.y,
                sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::N) ? 0 : 1);
        }
    }

    window.setView(view);
}

void Editor::handleEvents(const sf::Event& event)
{
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        float zoom = 1.0f - 0.1f * wheel->delta;
        view.zoom(zoom);
    }
}
