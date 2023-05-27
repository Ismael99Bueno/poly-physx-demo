#include "ppxdpch.hpp"
#include "menu_bar.hpp"
#include "demo_app.hpp"
#include "globals.hpp"

namespace ppx_demo
{
menu_bar::menu_bar() : ppx::layer("menu_bar")
{
}
void menu_bar::on_render()
{
    if (ImGui::BeginMainMenuBar())
    {
        demo_app &papp = demo_app::get();
        render_file_menu();
        render_windows_menu();
        render_settings_menu();

        ImGui::BeginMenu(papp.has_session() ? ("Current session: " + papp.session()).c_str()
                                            : "No current session. Select 'Save as...' to create one",
                         false);
        ImGui::EndMainMenuBar();
    }
}

void menu_bar::render_file_menu() const
{
    demo_app &papp = demo_app::get();
    if (ImGui::BeginMenu("File")) // TODO: Menu de examples
    {
        if (ImGui::MenuItem("New", "LCtrl + N"))
        {
            papp.read_save(DEFAULT_SAVE);
            papp.add_borders();
        }

        save_item();
        load_item();
        save_as_item();
        load_as_item();
        ImGui::EndMenu();
    }
}

void menu_bar::render_windows_menu() const
{
    demo_app &papp = demo_app::get();
    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Actions", nullptr, &papp.p_actions_panel->p_visible);
        ImGui::MenuItem("Engine", nullptr, &papp.p_engine_panel->p_visible);
        ImGui::MenuItem("Physics", nullptr, &papp.p_phys_panel->p_visible);
        ImGui::MenuItem("Performance", nullptr, &papp.p_perf_panel->p_visible);
        ImGui::EndMenu();
    }
}

void menu_bar::render_settings_menu() const
{
    if (ImGui::BeginMenu("Settings"))
    {
        render_fonts();
        ImGui::EndMenu();
    }
}

void menu_bar::render_fullscreen_option() const
{
    demo_app &papp = demo_app::get();
    bool fullscreen = (bool)(papp.style() & sf::Style::Fullscreen);
    if (ImGui::MenuItem("Fullscreen", "F10", &fullscreen))
        papp.recreate_window(fullscreen ? sf::Style::Fullscreen : sf::Style::Default);
}

void menu_bar::render_fonts() const
{
    if (ImGui::BeginMenu("Fonts"))
    {
        ImGuiIO &io = ImGui::GetIO();
        ImFont *current = ImGui::GetFont();
        for (int i = 0; i < io.Fonts->Fonts.Size; i++)
        {
            ImFont *font = io.Fonts->Fonts[i];
            if (ImGui::MenuItem(font->GetDebugName(), nullptr, font == current))
                io.FontDefault = font;
        }
        ImGui::EndMenu();
    }
}

std::optional<std::string> menu_bar::chosen_savefile() const
{
    std::optional<std::string> res = std::nullopt;
    std::string to_delete;

    for (const auto &entry : std::filesystem::directory_iterator(SAVES_DIR))
    {
        const std::string &path = entry.path().string(),
                          filename = path.substr(path.find_last_of("/") + 1, path.size() - 1);
        if (filename == DEFAULT_SAVE || filename == LAST_SAVE)
            continue;

        if (ImGui::Button("X"))
            to_delete = path;

        ImGui::SameLine();
        if (ImGui::MenuItem(filename.c_str()))
            res = filename;
    }
    if (!to_delete.empty())
    {
        std::filesystem::remove(to_delete);
        demo_app::get().validate_session();
    }
    return res;
}

void menu_bar::save_item() const
{
    if (ImGui::MenuItem("Save", "LCtrl + S", nullptr, demo_app::get().has_session()))
        demo_app::get().write_save();
}

void menu_bar::save_as_item() const
{
    if (ImGui::BeginMenu("Save as..."))
    {
        static char buffer[24] = "\0";
        ImGui::PushItemWidth(400);
        if (ImGui::InputTextWithHint("##", "File name", buffer, IM_ARRAYSIZE(buffer),
                                     ImGuiInputTextFlags_EnterReturnsTrue) &&
            buffer[0] != '\0')
        {
            SUBSTITUTE(buffer, ' ', '-')
            const std::string savefile(buffer);
            demo_app &papp = demo_app::get();
            papp.session(savefile + ".yaml");
            papp.write_save(savefile + ".yaml");
            buffer[0] = '\0';
        }
        ImGui::PopItemWidth();
        ImGui::EndMenu();
    }
}

void menu_bar::load_item() const
{
    demo_app &papp = demo_app::get();

    if (ImGui::MenuItem("Load", "LCtrl + L", nullptr, papp.has_session()))
        papp.read_save();
}

void menu_bar::load_as_item() const
{
    if (ImGui::BeginMenu("Load as..."))
    {
        std::optional<std::string> filename = chosen_savefile();
        if (filename)
            demo_app::get().read_save(filename.value());
        ImGui::EndMenu();
    }
}
} // namespace ppx_demo