#ifndef OUTLINE_MANAGER_HPP
#define OUTLINE_MANAGER_HPP

#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class demo_app;
    class outline_manager
    {
    public:
        outline_manager(demo_app *papp, std::size_t allocations = 100);

        void update();
        void load_outline(std::size_t index, const sf::Color &color, std::uint32_t priority);

    private:
        demo_app *m_app;
        std::vector<std::pair<std::uint32_t, sf::Color>> m_outline_colors;
        sf::Clock m_clock;

        void paint_outlines(std::vector<sf::ConvexShape> &shapes) const;
        void reset_priorities();
    };
}

#endif