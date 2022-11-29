#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>

int main()
{
    // create the window
    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");
    if (!ImGui::SFML::Init(window))
        exit(EXIT_FAILURE);

    sf::CircleShape shape(100);
    shape.setPosition(400, 300);
    bool exists = true;

    sf::Clock delta_clock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();
        }
        ImGui::SFML::Update(window, delta_clock.restart());

        ImGui::Begin("Hey!");
        ImGui::Text("Heyyyyy");
        ImGui::Checkbox("Circle", &exists);
        ImGui::End();

        window.clear(sf::Color::Black);
        if (exists)
            window.draw(shape);
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}