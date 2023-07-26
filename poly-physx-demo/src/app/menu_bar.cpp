#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
menu_bar::menu_bar() : demo_layer("Menu bar")
{
}

void menu_bar::on_render(const float ts)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save", "Ctrl + S"))
            {
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void menu_bar::serialize_to_saves(const std::string &filename) const
{
    const std::string filepath = SAVES_DIRECTORY + filename;
    m_app->serialize(filepath);
}
void menu_bar::serialize_to_examples(const std::string &filename) const
{
    const std::string filepath = EXAMPLES_DIRECTORY + filename;
    m_app->serialize(filepath);
}

void menu_bar::deserialize_from_saves(const std::string &filename) const
{
    const std::string filepath = SAVES_DIRECTORY + filename;
    m_app->deserialize(filepath);
}
void menu_bar::deserialize_from_examples(const std::string &filename) const
{
    const std::string filepath = EXAMPLES_DIRECTORY + filename;
    m_app->deserialize(filepath);
}
} // namespace ppx::demo