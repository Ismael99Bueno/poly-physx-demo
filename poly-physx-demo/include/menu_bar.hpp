#ifndef MENU_BAR_HPP
#define MENU_BAR_HPP

#include "ppx/layer.hpp"
#include <optional>

namespace ppx_demo
{
    class menu_bar : public ppx::layer
    {
    public:
        menu_bar();

    private:
        void on_render() override;

        void render_file_menu() const;
        void render_windows_menu() const;
        void render_settings_menu() const;
        void render_fullscreen_option() const;
        void render_fonts() const;

        std::optional<std::string> chosen_savefile() const;
        void save_item() const;
        void save_as_item() const;
        void load_item() const;
        void load_as_item() const;
    };
}

#endif