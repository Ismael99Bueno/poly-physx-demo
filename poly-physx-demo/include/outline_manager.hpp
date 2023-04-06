#ifndef OUTLINE_MANAGER_HPP
#define OUTLINE_MANAGER_HPP

#include "engine2D.hpp"
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class outline_manager
    {
    public:
        outline_manager(std::size_t allocations = 100);

        void start();
        void update();
        void load_outline(std::size_t index, const sf::Color &color, std::uint32_t priority);

    private:
        std::vector<std::pair<std::uint32_t, sf::Color>> m_outline_colors;
        sf::Clock m_clock;

        void paint_outlines(utils::vector_view<sf::ConvexShape> shapes) const;
        void reset_priorities();
    };
}

#endif