#include "outline_manager.hpp"
#include "entity2D_ptr.hpp"
#include "debug.hpp"
#include "demo_app.hpp"
#include <cmath>

namespace phys_demo
{
    outline_manager::outline_manager(const std::size_t allocations)
    {
        m_outline_colors.reserve(allocations);
    }

    void outline_manager::start()
    {

        const auto on_addition = [this](phys::entity2D_ptr e)
        { m_outline_colors.emplace_back(0, sf::Color::Black); };

        const auto on_removal = [this](const std::size_t index)
        {
            m_outline_colors[index] = m_outline_colors.back();
            m_outline_colors.pop_back();
        };

        demo_app &papp = demo_app::get();
        papp.engine().on_entity_addition(on_addition);
        papp.engine().on_entity_removal(on_removal);
    }

    void outline_manager::update()
    {
        paint_outlines(demo_app::get().shapes());
        reset_priorities();
    }
    void outline_manager::paint_outlines(utils::vector_view<sf::ConvexShape> shapes) const
    {
        for (std::size_t i = 0; i < shapes.unwrap().size(); i++)
        {
            const auto &[priority, color] = m_outline_colors[i];
            shapes[i].setOutlineColor(color);
            if (priority)
            {
                const float ampl = 1.5f, freq = 2.5f;
                shapes[i].setOutlineThickness(ampl * (2.0f + std::sinf(freq * m_clock.getElapsedTime().asSeconds())));
            }
            else
                shapes[i].setOutlineThickness(0.f);
        }
    }

    void outline_manager::load_outline(const std::size_t index,
                                       const sf::Color &color,
                                       const std::uint32_t priority)
    {
        auto &[prev_priority, prev_color] = m_outline_colors[index];
        if (priority > prev_priority)
        {
            prev_color = color;
            prev_priority = priority;
        }
    }

    void outline_manager::reset_priorities()
    {
        for (auto &elm : m_outline_colors)
            elm.first = 0;
    }
}