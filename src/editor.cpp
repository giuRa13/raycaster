#include "editor.h"
#include "resources.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>
#include <imgui-SFML.h>
#include <imgui.h>

void Editor::init(sf::RenderWindow& window)
{
    view = window.getView();
    cell.setFillColor(sf::Color::Green);
}

void Editor::run(sf::RenderWindow& window, Map& map)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save")) {
                map.save(RESOURCES_PATH "test.map");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Editing Options");
    ImGui::Text("Set N° ");
    ImGui::SameLine();
    ImGui::InputInt("##set_n", &setN);
    ImGui::Text("Texture N° ");
    ImGui::SameLine();
    ImGui::InputInt("##tex_n", &textureN);

    ImGui::Text("Preview: ");
    // ImGui::Image(
    // sf::SpriteShape {
    // Resources::wallTexture,
    // sf::IntRect(textureN * textureSize, 0, textureSize, textureSize) },
    // sf::Vector2f(100.f, 100.f));
    /*int textureSize = Resources::wallTexture.getSize().y;
    ImGui::Image(
        reinterpret_cast<ImTextureID>(Resources::wallTexture.getNativeHandle()),
        ImVec2(100.f, 100.f),
        ImVec2((textureN * textureSize) / static_cast<float>(Resources::wallTexture.getSize().x), 0.f),
        ImVec2((textureN + 1) * textureSize / static_cast<float>(Resources::wallTexture.getSize().x), 1.f));*/
    setN = std::clamp(setN, 0, 4);
    textureN = std::clamp(textureN, 0, 11);

    const auto& texture = Resources::wallAtlas; // <-- your 12x12 combined texture
    const int cols = 12;
    const int rows = 5;
    const sf::Vector2u texSize = texture.getSize();
    const float tileWidth = static_cast<float>(texSize.x) / cols;
    const float tileHeight = static_cast<float>(texSize.y) / rows;

    ImVec2 uv0(
        textureN * tileWidth / texSize.x,
        setN * tileHeight / texSize.y);
    ImVec2 uv1(
        (textureN + 1) * tileWidth / texSize.x,
        (setN + 1) * tileHeight / texSize.y);

    ImGui::Image(
        reinterpret_cast<ImTextureID>(texture.getNativeHandle()),
        ImVec2(100.f, 100.f),
        uv0, uv1);
    ImGui::End();

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

        /*if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            map.setMapCell(mapPos.x, mapPos.y,
                sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::N) ? 0 : textureN + 1);
        }*/
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            int id = setN * 12 + textureN + 1; // atlasCols = 12
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::N))
                id = 0; // erase
            map.setMapCell(mapPos.x, mapPos.y, id);
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
