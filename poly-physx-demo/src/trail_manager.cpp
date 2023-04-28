#include "pch.hpp"
#include "trail_manager.hpp"
#include "prm/thick_line.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    void trail_manager::start()
    {
        const auto on_addition = [this](const ppx::entity2D_ptr &e)
        {
            if (p_enabled && p_auto_include)
                include(e);
        };
        const auto on_removal = [this](const std::size_t index)
        {
            for (auto it = m_trails.begin(); it != m_trails.end();)
                if (!it->first.try_validate())
                    it = m_trails.erase(it);
                else
                    ++it;
        };
        demo_app &papp = demo_app::get();
        papp.engine().events().on_entity_addition += on_addition;
        papp.engine().events().on_late_entity_removal += on_removal;
        m_trails.reserve(p_steps);
    }

    void trail_manager::update()
    {
        static std::size_t updates = 0;
        if (!p_enabled || updates++ < p_length)
            return;
        updates = 0;

        for (auto &[e, trail] : m_trails)
        {
            auto vertices = trail.vertices();
            if (vertices.unwrap().size() >= p_steps)
                trail.erase(0, vertices.unwrap().size() - p_steps + 1);

            trail.append(e->pos() * WORLD_TO_PIXEL);
            for (std::size_t i = 0; i < vertices.unwrap().size(); i++)
            {
                const float alpha = (float)i / (float)(p_steps - 1);
                vertices[i].second.a = (sf::Uint8)(255.f * alpha);
            }
            trail.thickness(p_line_thickness);
        }
    }

    void trail_manager::render() const
    {
        if (!p_enabled)
            return;

        for (const auto &[e, trail] : m_trails)
            if (trail.vertices().size() > 1)
            {
                demo_app &papp = demo_app::get();

                const auto &[last_pos, last_color] = trail.vertices().back();
                prm::thick_line tl(last_pos, e->pos() * WORLD_TO_PIXEL, p_line_thickness, last_color, true);

                papp.draw(tl);
                papp.draw(trail);
            }
    }

    void trail_manager::include(const ppx::const_entity2D_ptr &e)
    {
        if (!contains(*e))
            m_trails.emplace_back(e, demo_app::get()[e.index()].getFillColor());
    }
    void trail_manager::exclude(const ppx::entity2D &e)
    {
        for (auto it = m_trails.begin(); it != m_trails.end(); ++it)
            if (*(it->first) == e)
            {
                m_trails.erase(it);
                break;
            }
    }
    bool trail_manager::contains(const ppx::entity2D &e) const
    {
        for (const auto &[entt, path] : m_trails)
            if (e == *entt)
                return true;
        return false;
    }

    void trail_manager::write(ini::output &out) const
    {
        out.write("steps", p_steps);
        out.write("length", p_length);
        out.write("thickness", p_line_thickness);
        out.write("enabled", p_enabled);
        out.write("auto_include", p_auto_include);

        const std::string key = "entity";
        for (const auto &[e, trail] : m_trails)
            out.write(key + std::to_string(e.index()), e.index());
    }
    void trail_manager::read(ini::input &in)
    {
        p_steps = in.readui32("steps");
        p_length = in.readui32("length");
        p_line_thickness = in.readf32("thickness");
        p_enabled = (bool)in.readi16("enabled");
        p_auto_include = (bool)in.readi16("auto_include");
        m_trails.clear();

        const std::string key = "entity";
        demo_app &papp = demo_app::get();
        for (std::size_t i = 0; i < papp.engine().size(); i++)
        {
            const ppx::entity2D_ptr e = papp.engine()[i];
            if (in.contains_key(key + std::to_string(e.index())))
                include(e);
        }
    }
}