#include "outline_manager.hpp"
#include "ppx/entity2D_ptr.hpp"
#include "ppxdpch.hpp"

#include "demo_app.hpp"

namespace ppx_demo
{
outline_manager::outline_manager(const std::size_t allocations)
{
    m_outline_colors.reserve(allocations);
}

void outline_manager::start()
{
    const auto on_addition = [this](const ppx::entity2D_ptr &e) { m_outline_colors.emplace_back(0, sf::Color::Black); };

    const auto on_removal = [this](const std::size_t index) {
        m_outline_colors[index] = m_outline_colors.back();
        m_outline_colors.pop_back();
    };

    demo_app &papp = demo_app::get();
    papp.engine().events().on_entity_addition += on_addition;
    papp.engine().events().on_late_entity_removal += on_removal;
}

void outline_manager::update()
{
    paint_outlines();
    reset_priorities();
}
void outline_manager::paint_outlines() const
{
    demo_app &papp = demo_app::get();
    for (std::size_t i = 0; i < m_outline_colors.size(); i++)
    {
        const auto &[priority, color] = m_outline_colors[i];
        sf::Shape &shape = papp[i];
        if (priority)
        {
            const float ampl = 3.f, freq = 3.5f;
            shape.setOutlineColor(color);
            shape.setOutlineThickness(ampl * (2.0f + sinf(freq * m_clock.getElapsedTime().asSeconds())));
        }
        else if (shape.getOutlineThickness() != 0.f)
            shape.setOutlineThickness(0.f);
    }
}

void outline_manager::load_outline(const std::size_t index, const sf::Color &color, const std::uint32_t priority)
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
} // namespace ppx_demo