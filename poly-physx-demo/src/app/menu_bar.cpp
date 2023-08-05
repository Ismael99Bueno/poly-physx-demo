#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "ppx-demo/app/demo_app.hpp"

#define EXTENSION ".yaml"

namespace ppx::demo
{
menu_bar::menu_bar() : demo_layer("Menu bar")
{
}

static bool any_user_file_present()
{
    for (const auto &entry : std::filesystem::directory_iterator(menu_bar::SAVES_DIRECTORY))
    {
        const std::string filename = entry.path().filename().string();
        if (filename != PPX_DEMO_LAST_SAVE_FILENAME && filename != PPX_DEMO_DEFAULT_SAVE_FILENAME)
            return true;
    }
    return false;
}

void menu_bar::on_render(const float ts)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            const bool active_session = has_active_session();
            if (ImGui::MenuItem("Save", "Ctrl + S", nullptr, active_session))
                m_app->serialize(SAVES_DIRECTORY + m_session + EXTENSION);
            if (ImGui::MenuItem("Load", "Ctrl + L", nullptr, active_session))
                m_app->deserialize(SAVES_DIRECTORY + m_session + EXTENSION);

            if (ImGui::BeginMenu("Save as..."))
            {
                render_save_prompt();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load as...", any_user_file_present()))
            {
                render_load_and_removal_prompts();
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void menu_bar::render_save_prompt()
{
    static char buffer[24] = "\0";
    if (ImGui::InputTextWithHint("##", "File name", buffer, 24, ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        std::string name = buffer;
        if (name == PPX_DEMO_LAST_SAVE_FILENAME || name == PPX_DEMO_DEFAULT_SAVE_FILENAME)
            return;

        std::replace(name.begin(), name.end(), ' ', '-');
        m_session = name;

        name += EXTENSION;
        m_app->serialize(SAVES_DIRECTORY + name);
        buffer[0] = '\0';
    }
}
void menu_bar::render_load_and_removal_prompts()
{
    for (const auto &entry : std::filesystem::directory_iterator(SAVES_DIRECTORY))
    {
        const std::string path = entry.path().string();
        const std::string filename = entry.path().filename().string();
        if (filename == PPX_DEMO_LAST_SAVE_FILENAME || filename == PPX_DEMO_DEFAULT_SAVE_FILENAME)
            continue;

        if (ImGui::Button("X"))
        {
            std::filesystem::remove(path);
            if (filename == m_session + EXTENSION)
                m_session.clear();
            return;
        }
        ImGui::SameLine();
        if (ImGui::MenuItem(filename.c_str()))
        {
            m_app->deserialize(path);
            return;
        }
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

bool menu_bar::has_active_session() const
{
    return !m_session.empty();
}

YAML::Node menu_bar::encode() const
{
    YAML::Node node;
    node["Session"] = m_session;
    return node;
}
bool menu_bar::decode(const YAML::Node &node)
{
    m_session = node["Session"].as<std::string>();
    return true;
}
} // namespace ppx::demo