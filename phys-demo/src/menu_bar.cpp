#include "menu_bar.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "demo_app.hpp"
#include "constants.hpp"
#include <filesystem>

namespace phys_demo
{
    void menu_bar::on_render()
    {
        if (ImGui::BeginMainMenuBar())
        {
            demo_app &app = demo_app::get();
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                {
                    app.load(DEFAULT_SAVE);
                    app.add_borders();
                }
                if (app.has_session())
                {
                    save_item();
                    load_item();
                }
                save_as_item();
                load_as_item();

                if (ImGui::MenuItem("Exit"))
                    app.window().close();
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    std::optional<std::string> menu_bar::chosen_savefile() const
    {
        std::optional<std::string> res = std::nullopt;
        for (const auto &entry : std::filesystem::directory_iterator(SAVES_DIR))
        {
            const std::string path = entry.path().string(),
                              filename = path.substr(path.find("/") + 1, path.size() - 1);
            if (filename == DEFAULT_SAVE)
                continue;
            if (ImGui::MenuItem(filename.c_str()))
                res = filename;
        }
        return res;
    }

    void menu_bar::save_item() const
    {
        if (ImGui::MenuItem("Save"))
            demo_app::get().save();
    }

    void menu_bar::save_as_item() const
    {
        if (ImGui::BeginMenu("Save as..."))
        {
            static char buff[24];
            ImGui::PushItemWidth(400);
            if (ImGui::InputText("Save file", buff, IM_ARRAYSIZE(buff), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                const std::string savefile(buff);
                demo_app &app = demo_app::get();
                app.session(savefile + ".ini");
                app.save(savefile + ".ini");
                buff[0] = '\0';
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }
    }

    void menu_bar::load_item() const
    {
        if (ImGui::MenuItem("Load"))
            demo_app::get().load();
    }

    void menu_bar::load_as_item() const
    {
        if (ImGui::BeginMenu("Load as..."))
        {
            std::optional<std::string> filename = chosen_savefile();
            if (filename)
                demo_app::get().load(filename.value());
            ImGui::EndMenu();
        }
    }
}