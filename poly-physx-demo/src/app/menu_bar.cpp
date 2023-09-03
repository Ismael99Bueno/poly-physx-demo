#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/menu_bar.hpp"
#include "ppx-demo/app/demo_app.hpp"

#define EXTENSION ".yaml"

namespace ppx::demo
{
menu_bar::menu_bar() : demo_layer("Menu bar")
{
}

static bool any_user_file_present(const char *path)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        const std::string filename = entry.path().filename().string();
        if (filename != PPX_DEMO_LAST_SAVE_FILENAME && filename != PPX_DEMO_DEFAULT_SAVE_FILENAME)
            return true;
    }
    return false;
}

void menu_bar::on_update(float ts)
{
    if (!m_serialize_path.empty())
    {
        m_app->serialize(m_serialize_path);
        m_serialize_path.clear();
    }
    if (!m_deserialize_path.empty())
    {
        m_app->deserialize(m_deserialize_path);
        m_deserialize_path.clear();
    }
}

void menu_bar::on_render(const float ts)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            const bool active_session = has_active_session();
            if (ImGui::MenuItem("Save", "Ctrl + S", nullptr, active_session))
                m_serialize_path = SAVES_DIRECTORY + m_session + EXTENSION;
            if (ImGui::MenuItem("Load", "Ctrl + L", nullptr, active_session))
                m_deserialize_path = SAVES_DIRECTORY + m_session + EXTENSION;

            if (ImGui::BeginMenu("Save as..."))
            {
                render_save_prompt();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load as...", any_user_file_present(SAVES_DIRECTORY)))
            {
                render_load_and_removal_prompts(SAVES_DIRECTORY);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Examples", any_user_file_present(EXAMPLES_DIRECTORY)))
            {
                render_load_prompts(EXAMPLES_DIRECTORY);
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
    if (ImGui::InputTextWithHint("##Body name input", "Session name", buffer, 24,
                                 ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        std::string name = buffer;
        std::replace(name.begin(), name.end(), ' ', '-');

        const std::string filename = name + EXTENSION;
        if (filename == PPX_DEMO_LAST_SAVE_FILENAME || filename == PPX_DEMO_DEFAULT_SAVE_FILENAME)
            return;
        m_session = name;

        m_serialize_path = SAVES_DIRECTORY + filename;
        buffer[0] = '\0';
    }
}
void menu_bar::render_load_prompts(const char *path)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        const std::string path = entry.path().string();
        const std::string filename = entry.path().filename().string();
        if (ImGui::MenuItem(filename.c_str()))
        {
            m_deserialize_path = path;
            return;
        }
    }
}
void menu_bar::render_load_and_removal_prompts(const char *path)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
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
            m_deserialize_path = path;
            return;
        }
    }
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