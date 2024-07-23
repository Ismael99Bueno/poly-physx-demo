#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/2D/app/menu_bar.hpp"
#include "ppx-demo/2D/app/demo_app.hpp"

#include "ppx-demo/2D/actions/actions_panel.hpp"
#include "ppx-demo/2D/engine/engine_panel.hpp"
#include "ppx-demo/2D/performance/performance_panel.hpp"
#include "ppx-demo/2D/physics/physics_panel.hpp"

namespace ppx::demo
{
menu_bar::menu_bar() : demo_layer("Menu bar")
{
}

static bool any_user_file_present(const char *path)
{
    if (!std::filesystem::exists(path))
        return false;
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
            if (ImGui::MenuItem("New", "Ctrl + N"))
                load_new_session();
            if (ImGui::BeginMenu("Examples", any_user_file_present(EXAMPLES_DIRECTORY)))
            {
                render_load_prompts(EXAMPLES_DIRECTORY);
                ImGui::EndMenu();
            }

            const bool active_session = has_active_session();
            if (ImGui::MenuItem("Save", "Ctrl + S", nullptr, active_session))
                save();
            if (ImGui::MenuItem("Load", "Ctrl + L", nullptr, active_session))
                load();

            if (ImGui::BeginMenu("Save as..."))
            {
                save_as();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Load as...", any_user_file_present(SAVES_DIRECTORY)))
            {
                load_as(SAVES_DIRECTORY);
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Reload config file"))
                m_app->reload_config_file();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("Actions", nullptr, &m_app->actions->window_toggle);
            ImGui::MenuItem("Engine", nullptr, &m_app->engine->window_toggle);
            ImGui::MenuItem("Performance", nullptr, &m_app->performance->window_toggle);
            ImGui::MenuItem("Physics", nullptr, &m_app->physics->window_toggle);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

bool menu_bar::on_event(const lynx::event2D &event)
{
    switch (event.type)
    {
    case lynx::event2D::KEY_PRESSED:
        switch (event.key)
        {
        case lynx::input2D::key::S:
            if (lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL) && has_active_session())
            {
                save();
                return true;
            }
        case lynx::input2D::key::L:
            if (lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL) && has_active_session())
            {
                load();
                return true;
            }
        case lynx::input2D::key::N:
            if (lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL))
            {
                load_new_session();
                return true;
            }
        default:
            break;
        }
    default:
        break;
    }
    return false;
}

void menu_bar::load_new_session()
{
    m_deserialize_path = DEFAULT_SAVE_FILEPATH;
}
void menu_bar::save()
{
    m_serialize_path = SAVES_DIRECTORY + m_session + PPX_DEMO_YAML_EXTENSION;
}
void menu_bar::load()
{
    m_deserialize_path = SAVES_DIRECTORY + m_session + PPX_DEMO_YAML_EXTENSION;
}

void menu_bar::save_as()
{
    static char buffer[24] = "\0";
    if (ImGui::InputTextWithHint("##Body name input", "Session name", buffer, 24,
                                 ImGuiInputTextFlags_EnterReturnsTrue) &&
        buffer[0] != '\0')
    {
        std::string name = buffer;
        std::replace(name.begin(), name.end(), ' ', '-');

        const std::string filename = name + PPX_DEMO_YAML_EXTENSION;
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
void menu_bar::load_as(const char *path)
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
            if (filename == m_session + PPX_DEMO_YAML_EXTENSION)
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