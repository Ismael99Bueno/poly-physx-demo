#ifndef MENU_BAR_HPP
#define MENU_BAR_HPP

#include "layer.hpp"
#include <optional>

namespace phys_demo
{
    class menu_bar : public phys::layer
    {
    public:
        menu_bar() = default;

    private:
        void on_render() override;

        void render_file_menu() const;
        void render_windows_menu() const;

        std::optional<std::string> chosen_savefile() const;
        void save_item() const;
        void save_as_item() const;
        void load_item() const;
        void load_as_item() const;
    };
}

#endif