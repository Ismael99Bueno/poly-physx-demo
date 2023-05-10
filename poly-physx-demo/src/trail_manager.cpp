#include "ppxdpch.hpp"
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
                if (!it->first.validate())
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

    YAML::Emitter &operator<<(YAML::Emitter &out, const trail_manager &tm)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Enabled" << YAML::Value << tm.p_enabled;
        out << YAML::Key << "Auto include" << YAML::Value << tm.p_auto_include;
        out << YAML::Key << "Steps" << YAML::Value << tm.p_steps;
        out << YAML::Key << "Length" << YAML::Value << tm.p_length;
        out << YAML::Key << "Line thickness" << YAML::Value << tm.p_line_thickness;
        out << YAML::Key << "Trails" << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (const auto &[e, line] : tm.m_trails)
            out << e.index();
        out << YAML::EndSeq;
        out << YAML::EndMap;
        return out;
    }
}

namespace YAML
{
    Node convert<ppx_demo::trail_manager>::encode(const ppx_demo::trail_manager &tm)
    {
        Node node;
        node["Enabled"] = tm.p_enabled;
        node["Auto include"] = tm.p_auto_include;
        node["Steps"] = tm.p_steps;
        node["Length"] = tm.p_length;
        node["Line thickness"] = tm.p_line_thickness;
        for (const auto &[e, line] : tm.m_trails)
            node["Trails"].push_back(e.index());
        node["Trails"].SetStyle(YAML::EmitterStyle::Flow);
        return node;
    }
    bool convert<ppx_demo::trail_manager>::decode(const Node &node, ppx_demo::trail_manager &tm)
    {
        if (!node.IsMap() || node.size() != 6)
            return false;
        tm.p_enabled = node["Enabled"].as<bool>();
        tm.p_steps = node["Steps"].as<std::uint32_t>();
        tm.p_line_thickness = node["Line thickness"].as<float>();
        tm.p_auto_include = node["Auto include"].as<bool>();
        tm.p_length = node["Length"].as<std::uint32_t>();
        tm.m_trails.clear();
        for (const Node &n : node["Trails"])
            tm.include(ppx_demo::demo_app::get().engine()[n.as<std::size_t>()]);
        return true;
    }
}