#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/menu_bar.hpp"

#ifdef ROOT_PATH

#define SAVES_DIR ROOT_PATH "saves/"
#define EXAMPLES_DIR ROOT_PATH "examples/"

#else

#define SAVES_DIR "saves/"
#define EXAMPLES_DIR "examples/"

#endif

#define DEFAULT_SAVE "default.yaml"
#define LAST_SAVE "last.yaml"

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
} // namespace ppx::demo