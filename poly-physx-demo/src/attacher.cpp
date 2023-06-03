#include "ppxdpch.hpp"
#include "attacher.hpp"
#include "demo_app.hpp"
#include "prm/spring_line.hpp"
#include "prm/thick_line.hpp"
#include "globals.hpp"

namespace ppx_demo
{
    void attacher::update(const bool snap_e2_to_center)
    {
        PERF_PRETTY_FUNCTION()
        if (!m_e1)
            return;
        if (!m_snap_e1_to_center)
            rotate_anchor();

        demo_app &papp = demo_app::get();
        if (p_auto_length)
        {
            float length = glm::distance(papp.world_mouse(), m_e1->pos() + m_anchor1);
            if (snap_e2_to_center)
            {
                const glm::vec2 mpos = papp.world_mouse();
                const auto e2 = papp.engine()[mpos];
                if (e2)
                    length = glm::distance(e2->pos(), m_e1->pos() + m_anchor1);
            }
            p_sp_length = length;
        }
    }
    void attacher::render(const bool snap_e2_to_center) const
    {
        PERF_PRETTY_FUNCTION()
        if (m_e1)
            draw_unattached_anchor(snap_e2_to_center);
    }

    void attacher::try_attach_first(const bool snap_e1_to_center)
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        const auto e1 = papp.engine()[mpos];
        if (!e1)
            return;
        m_e1 = e1;
        m_anchor1 = snap_e1_to_center ? glm::vec2(0.f) : (mpos - e1->pos());
        if (!snap_e1_to_center)
            m_last_angle = e1->angpos();
        m_snap_e1_to_center = snap_e1_to_center;
    }

    void attacher::try_attach_second(const bool snap_e2_to_center)
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        const auto e2 = papp.engine()[mpos];
        if (!e2 || e2 == m_e1)
            return;
        const glm::vec2 anchor2 = snap_e2_to_center ? glm::vec2(0.f) : (mpos - e2->pos());

        const bool no_anchors = m_snap_e1_to_center && snap_e2_to_center;
        switch (p_attach)
        {
        case SPRING:
        {
            if (no_anchors)
                papp.engine().add_spring(m_e1, e2, p_sp_stiffness, p_sp_dampening, p_sp_length);
            else
                papp.engine().add_spring(m_e1, e2, m_anchor1, anchor2, p_sp_stiffness, p_sp_dampening, p_sp_length);
            break;
        }
        case RIGID_BAR:
        {
            if (no_anchors)
                papp.engine().compeller().add_constraint<ppx::rigid_bar2D>(m_e1, e2, p_rb_stiffness, p_rb_dampening);
            else
                papp.engine().compeller().add_constraint<ppx::rigid_bar2D>(m_e1, e2, m_anchor1, anchor2, p_rb_stiffness, p_rb_dampening);
            break;
        }
        }
        m_e1 = nullptr;
    }

    void attacher::rotate_anchor()
    {
        m_anchor1 = glm::rotate(m_anchor1, m_e1->angpos() - m_last_angle);
        m_last_angle = m_e1->angpos();
    }
    void attacher::draw_unattached_anchor(const bool snap_e2_to_center) const
    {
        demo_app &papp = demo_app::get();

        const glm::vec2 mpos = papp.world_mouse();
        const auto e2 = papp.engine()[mpos];
        const glm::vec2 anchor2 = (snap_e2_to_center && e2) ? (e2->pos() * PPX_WORLD_TO_PIXEL) : papp.pixel_mouse();
        switch (p_attach)
        {
        case SPRING:
            papp.draw_spring((m_e1->pos() + m_anchor1) * PPX_WORLD_TO_PIXEL, anchor2);
            break;
        case RIGID_BAR:
            papp.draw_rigid_bar((m_e1->pos() + m_anchor1) * PPX_WORLD_TO_PIXEL, anchor2);
            break;
        }
    }

    void attacher::cancel() { m_e1 = nullptr; }

    bool attacher::has_first() const { return (bool)m_e1; }

    YAML::Emitter &operator<<(YAML::Emitter &out, const attacher &attch)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Spring stiffness" << YAML::Value << attch.p_sp_stiffness;
        out << YAML::Key << "Spring dampening" << YAML::Value << attch.p_sp_dampening;
        out << YAML::Key << "Spring length" << YAML::Value << attch.p_sp_length;
        out << YAML::Key << "Rigid bar stiffness" << YAML::Value << attch.p_rb_stiffness;
        out << YAML::Key << "Rigid bar dampening" << YAML::Value << attch.p_rb_dampening;
        out << YAML::Key << "Auto length" << YAML::Value << attch.p_auto_length;
        out << YAML::Key << "Attach type" << YAML::Value << (int)attch.p_attach;
        out << YAML::EndMap;
        return out;
    }
}

namespace YAML
{
    Node convert<ppx_demo::attacher>::encode(const ppx_demo::attacher &attch)
    {
        Node node;
        node["Spring stiffness"] = attch.p_sp_stiffness;
        node["Spring dampening"] = attch.p_sp_dampening;
        node["Spring length"] = attch.p_sp_length;
        node["Rigid bar stiffness"] = attch.p_rb_stiffness;
        node["Rigid bar dampening"] = attch.p_rb_dampening;
        node["Auto length"] = attch.p_auto_length;
        node["Attach type"] = (int)attch.p_attach;
        return node;
    }
    bool convert<ppx_demo::attacher>::decode(const Node &node, ppx_demo::attacher &attch)
    {
        if (!node.IsMap() || node.size() != 7)
            return false;
        attch.p_sp_stiffness = node["Spring stiffness"].as<float>();
        attch.p_sp_dampening = node["Spring dampening"].as<float>();
        attch.p_sp_length = node["Spring length"].as<float>();
        attch.p_rb_stiffness = node["Rigid bar stiffness"].as<float>();
        attch.p_rb_dampening = node["Rigid bar dampening"].as<float>();
        attch.p_auto_length = node["Auto length"].as<bool>();
        attch.p_attach = (ppx_demo::attacher::attach_type)node["Attach type"].as<int>();
        return true;
    }
}