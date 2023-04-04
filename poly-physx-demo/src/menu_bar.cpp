#include "menu_bar.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "demo_app.hpp"
#include "constants.hpp"
#include <filesystem>

namespace ppx_demo
{
    void menu_bar::on_render()
    {
        if (ImGui::BeginMainMenuBar())
        {
            demo_app &app = demo_app::get();
            render_file_menu();
            render_windows_menu();
            ImGui::BeginMenu(app.has_session() ? ("Current session: " + app.session()).c_str() : "No current session. Select 'Save as...' to create one", false);
            ImGui::EndMainMenuBar();
        }
    }

    void menu_bar::render_file_menu() const
    {
        demo_app &app = demo_app::get();
        if (ImGui::BeginMenu("File")) // TODO: Menu de examples
        {
            if (ImGui::MenuItem("New"))
            {
                app.read_save(DEFAULT_SAVE);
                app.add_borders();
            }

            save_item();
            load_item();
            save_as_item();
            load_as_item();

            if (ImGui::MenuItem("Exit"))
                app.window().close();
            ImGui::EndMenu();
        }
    }

    void menu_bar::render_windows_menu() const
    {
        demo_app &app = demo_app::get();
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("Actions", nullptr, &app.p_actions_panel.p_enabled);
            ImGui::MenuItem("Engine", nullptr, &app.p_engine_panel.p_enabled);
            ImGui::MenuItem("Physics", nullptr, &app.p_phys_panel.p_enabled);
            ImGui::MenuItem("Performance", nullptr, &app.p_perf_panel.p_enabled);
            ImGui::EndMenu();
        }
    }

    std::optional<std::string> menu_bar::chosen_savefile() const
    {
        std::optional<std::string> res = std::nullopt;
        std::string to_delete;

        for (const auto &entry : std::filesystem::directory_iterator(SAVES_DIR))
        {
            const std::string &path = entry.path(),
                              filename = path.substr(path.find("/") + 1, path.size() - 1);
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
        if (ImGui::MenuItem("Save", nullptr, nullptr, demo_app::get().has_session()))
            demo_app::get().write_save();
    }

    void menu_bar::save_as_item() const
    {
        if (ImGui::BeginMenu("Save as..."))
        {
            static char buffer[24] = "\0";
            ImGui::PushItemWidth(400);
            if (ImGui::InputTextWithHint("##", "File name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue) && buffer[0] != '\0')
            {
                SUBSTITUTE(buffer, ' ', '-')
                const std::string savefile(buffer);
                demo_app &app = demo_app::get();
                app.session(savefile + ".ini");
                app.write_save(savefile + ".ini");
                buffer[0] = '\0';
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }
    }

    void menu_bar::load_item() const
    {
        demo_app &papp = demo_app::get();

        if (ImGui::MenuItem("Load", nullptr, nullptr, papp.has_session()))
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
}