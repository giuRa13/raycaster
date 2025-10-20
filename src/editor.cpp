#include "editor.h"
#include "resources.h"
#include <ImGuiFileDialog.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <imgui-SFML.h>
#include <imgui.h>

#define ICON_IGFD_FOLDER "\uf07b" // folder in FA
#define ICON_IGFD_FILE "\uf15b" // file in FA
#define ICON_IGFD_FILE_IMAGE "\uf1c5" // Unicode for image file
#define ICON_IGFD_FILE_CODE "\uf121"
#define ICON_IGFD_FILE_TEXT "\uf15c"
#define ICON_IGFD_FILE_AUDIO "\uf1c7" // Audio File
#define ICON_IGFD_FILE_VIDEO "\uf1c8" // Video File
#define ICON_IGFD_FILE_ZIP "\uf1c6" // ZIP File
#define ICON_IGFD_FILE_MARKDOWN "\uf60f" // Markdown File
#define ICON_IGFD_FILE_JSON "\uf1c9"
#define ICON_IGFD_FILE_CSV "\uf1c3"
#define ICON_IGFD_FILE_PDF "\uf1c1"
#define ICON_IGFD_FOLDER_OPEN "\uf07c" // Open Folder
#define ICON_IGFD_FOLDER_TREE "\uf114" // Folder Tree
#define ICON_IGFD_FILE_ARCHIVE "\uf1c0"
#define ICON_IGFD_FILE_FONT "\uf031"

#define ICON_IGFD_OK "\uf00c" // Check (OK)
#define ICON_IGFD_CANCEL "\uf00d" // Times / Cancel
#define ICON_IGFD_EDIT "\uf303"
#define ICON_IGFD_RESET "\uf045"
#define ICON_IGFD_BACK "\uf060" // Arrow Left (Back)
#define ICON_IGFD_FORWARD "\uf061" // Arrow Right (Forward)
#define ICON_IGFD_UP "\uf062" // Arrow Up (Parent Dir)
#define ICON_IGFD_REFRESH "\uf021" // Refresh
#define ICON_IGFD_HOME "\uf015" // Home
#define ICON_IGFD_SAVE "\uf0c7" // Save
#define ICON_IGFD_UPLOAD "\uf093" // Upload
#define ICON_IGFD_DOWNLOAD "\uf019" // Download
#define ICON_IGFD_SEARCH "\uf002" // Search
#define ICON_IGFD_INFO "\uf129" // Info Circle
#define ICON_IGFD_WARNING "\uf071" // Exclamation Triangle

void Editor::init(sf::RenderWindow& window)
{
    currentLayer = Map::LAYER_WALLS;

    view = window.getView();
    cell.setFillColor(sf::Color::Green);
}

void Editor::run(sf::RenderWindow& window, Map& map)
{
    auto dlg = ImGuiFileDialog::Instance();
    dlg->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(1.0f, 0.85f, 0.2f, 0.9f), ICON_IGFD_FOLDER);
    dlg->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(0.3f, 0.6f, 1.0f, 0.9f), ICON_IGFD_FILE);
    dlg->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.3f, 0.8f, 0.6f, 0.9f), ICON_IGFD_FILE_IMAGE);
    dlg->SetFileStyle(IGFD_FileStyleByExtention, ".cpp,.hpp,.c,.h", ImVec4(0.3f, 0.8f, 0.6f, 0.9f), ICON_IGFD_FILE_CODE);
    dlg->SetFileStyle(IGFD_FileStyleByExtention, ".ttf", ImVec4(1.0f, 0.3f, 0.3f, 0.9f), ICON_IGFD_FILE_FONT);
    dlg->SetFileStyle(IGFD_FileStyleByExtention, ".map", ImVec4(0.7f, 0.4f, 1.0f, 0.9f), ICON_IGFD_FILE);
    dlg->SetFileStyle(IGFD_FileStyleByContainedInFullName, "CMakeLists", ImVec4(1.0f, 0.4f, 0.8f, 0.9f), ICON_IGFD_FILE);
    dlg->SetFileStyle(IGFD_FileStyleByExtention, ".txt", ImVec4(0.8f, 0.8f, 0.8f, 0.9f), ICON_IGFD_FILE_TEXT);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("Open")) {
                dlg->OpenDialog("OpenDialog", "Chose Map", ".*");
            }
            if (ImGui::MenuItem("Save")) {
                if (savedFileName.empty()) {
                    dlg->OpenDialog("SaveDialog", "Save Map", ".*");
                } else {
                    map.save(savedFileName);
                }
            }
            if (ImGui::MenuItem("Save As")) {
                dlg->OpenDialog("SaveDialog", "Save Map As", ".*");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        if (dlg->Display("OpenDialog")) {
            if (dlg->IsOk()) {
                savedFileName = dlg->GetFilePathName();
                map.load(savedFileName);
            }
            dlg->Close();
        }

        if (dlg->Display("SaveDialog")) {
            if (dlg->IsOk()) {
                savedFileName = dlg->GetFilePathName();
                map.save(savedFileName);
            }
            dlg->Close();
        }
    }

    ImGui::Begin("Editing Options");
    ImGui::Text("Layer: ");
    if (ImGui::BeginCombo("##layers", Map::LAYER_NAMES[currentLayer])) {

        for (size_t i = 0; i < Map::NUM_LAYERS; i++) {
            if (ImGui::Selectable(Map::LAYER_NAMES[i], currentLayer == i)) {
                currentLayer = i;
            }
        }

        ImGui::EndCombo();
    }

    ImGui::Text("Set N° ");
    ImGui::SameLine();
    ImGui::InputInt("##set_n", &setN);
    ImGui::Text("Texture N° ");
    ImGui::SameLine();
    ImGui::InputInt("##tex_n", &textureN);

    ImGui::Text("Preview: ");

    setN = std::clamp(setN, 0, 4);
    textureN = std::clamp(textureN, 0, 11);

    const auto& texture = Resources::texturesAtlas; // <-- your 12x12 combined texture
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

    if (ImGui::Button("Fill")) {
        map.fill(currentLayer, setN * 12 + textureN + 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        map.fill(currentLayer, 0);
    }

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
            map.setMapCell(mapPos.x, mapPos.y, currentLayer, id);
        }
    }

    map.draw(window, currentLayer, currentLayer);
    window.setView(view);
}

void Editor::handleEvents(const sf::Event& event)
{
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        float zoom = 1.0f - 0.1f * wheel->delta;
        view.zoom(zoom);
    }
}
