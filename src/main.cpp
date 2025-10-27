#include "editor.h"
#include "map.h"
#include "player.h"
#include "renderer.h"
#include "resources.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include <string>

int main(int argc, const char** argv)
{
    // sf::RenderWindow window { sf::VideoMode { { (int)SCREEN_W, (int)SCREEN_H } }, "Raycaster", sf::Style::Close | sf::Style::Titlebar };
    auto window = sf::RenderWindow { sf::VideoMode { { (int)SCREEN_W, (int)SCREEN_H } }, "Raycaster", sf::Style::Close | sf::Style::Titlebar };
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window)) {
        std::cout << "Failed to init ImGui\n";
        return -1;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear(); // clear default fonts
    // Add default font first
    io.Fonts->AddFontDefault();
    // Configure for Font Awesome
    ImFontConfig faConfig;
    faConfig.MergeMode = true; // merge into existing font
    faConfig.PixelSnapH = true;
    // Load Font Awesome glyphs
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 }; // Font Awesome unicode range
    io.Fonts->AddFontFromFileTTF(RESOURCES_PATH "fa-solid-900.ttf", 16.0f, &faConfig, icons_ranges);
    // Rebuild the font texture
    (void)ImGui::SFML::UpdateFontTexture();

    if (!Resources::texturesImage.loadFromFile(RESOURCES_PATH "raycaster_textures.png")) {
        std::cerr << "Failed to load raycaster_textures.png\n";
    }
    Resources::texturesAtlas.loadFromImage(Resources::texturesImage);

    Player player {};
    player.position = sf::Vector2f(1.2f, 1.2f);

    Renderer renderer {};
    renderer.init();

    Editor editor {};
    editor.init(window);

    Map map {};
    if (argc > 1) {
        editor.savedFileName = argv[1];
    } else {
        editor.savedFileName = "../resources/test2.map";
    }
    map.load(editor.savedFileName);

    enum class State {
        Editor,
        Game
    } state
        = State::Game;

    sf::Clock gameClock;

    while (window.isOpen()) {
        float deltaTime = gameClock.restart().asSeconds();

        while (auto event = window.pollEvent()) {

            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape)) {
                window.close();
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::M) {
                    state = state == State::Game ? State::Editor : State::Game;
                }
            }

            if (state == State::Editor) {
                editor.handleEvents(*event);
            }
        }

        ImGui::SFML::Update(window, sf::seconds(deltaTime));

        ImGui::ShowDemoWindow();

        window.clear();

        if (state == State::Game) {
            window.setView(window.getDefaultView());
            player.update(deltaTime, map);
            renderer.draw3dView(window, player, map);
        } else {
            editor.run(window, map);
        }

        ImGui::SFML::Render(window);
        window.display();
        window.setTitle("Raycaster | " + std::to_string(1.0f / deltaTime));
    }

    ImGui::SFML::Shutdown();

    return 0;
}
