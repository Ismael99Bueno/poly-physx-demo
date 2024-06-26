#pragma once

#include "lynx/app/window.hpp"

#include "ppx/collision/broad/brute_force_broad2D.hpp"
#include "ppx/collision/broad/quad_tree_broad2D.hpp"
#include "ppx/collision/broad/sort_sweep_broad2D.hpp"
#include "ppx/collision/contacts/contact_solver2D.hpp"
#include "ppx/collision/contacts/nonpen_contact2D.hpp"
#include "ppx/collision/contacts/spring_contact2D.hpp"

#include "lynx/drawing/line.hpp"
#include "ppx-app/drawables/lines/thick_line2D.hpp"

namespace ppx::demo
{
class demo_app;
class collision_tab
{
  public:
    collision_tab() = default;
    collision_tab(demo_app *app);

    void render_imgui_tab();
    void update();
    void render();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    demo_app *m_app;
    lynx::window2D *m_window;

    bool m_draw_bounding_boxes = false;
    bool m_visualize_qtree = false;

    std::vector<lynx::line_strip2D> m_bbox_lines;
    std::vector<lynx::line_strip2D> m_qt_lines;
    std::size_t m_qt_active_partitions = 0;

    bool m_draw_contacts = false;

    struct collision_repr
    {
        lynx::ellipse2D point;
        thick_line2D normal;
    };
    using contact_key = collision_contacts2D::contact_key;
    using nonpen_cmap = contact_manager2D<nonpen_contact2D>::contact_map;
    using spring_cmap = contact_manager2D<spring_contact2D>::contact_map;

    std::unordered_map<contact_key, collision_repr> m_contact_lines;

    void render_quad_tree_parameters(quad_tree_broad2D &qtbroad);
    void render_quad_tree_node(const quad_tree::node &node);
    void render_nonpen_contact_solver_parameters();
    void render_spring_contact_solver_parameters();

    void update_bounding_boxes();
    void update_quad_tree_lines(const quad_tree::node &qtnode);

    template <typename CMap> void update_contact_lines(const CMap &contacts)
    {
        for (const auto &[hash, cnt] : contacts)
        {
            auto repr = m_contact_lines.find(hash);
            const auto &point = cnt->point();
            if (repr == m_contact_lines.end())
            {
                collision_repr new_repr;
                new_repr.point = lynx::ellipse2D{.3f, contact_color()};
                new_repr.point.transform.position = point.point;
                new_repr.normal = thick_line2D{normal_color(), .1f};

                repr = m_contact_lines.emplace(hash, new_repr).first;
            }

            const float length = 0.5f * std::clamp(100.f * glm::abs(point.penetration), 0.5f, 1.2f);
            const glm::vec2 dir = length * cnt->normal();

            repr->second.point.transform.position = point.point;
            repr->second.normal.p1(point.point);
            repr->second.normal.p2(point.point + dir);
            repr->second.point.color(contact_color(!cnt->enabled || cnt->asleep()));
            repr->second.normal.color(normal_color(!cnt->enabled || cnt->asleep()));
        }
    }

    lynx::color contact_color(bool faded = false) const;
    lynx::color normal_color(bool faded = false) const;

    template <typename CMap> void render_contact_lines(const CMap &contacts)
    {
        for (auto it = m_contact_lines.begin(); it != m_contact_lines.end();)
        {
            if (!contacts.contains(it->first))
            {
                it = m_contact_lines.erase(it);
                continue;
            }
            const auto &repr = it->second;
            m_window->draw(repr.point);
            m_window->draw(repr.normal);
            ++it;
        }
    }

    void render_broad_metrics() const;
    void render_broad_methods_list() const;
    void render_collisions_and_contacts_list() const;
    void render_contact_solvers_list() const;

    void render_cp_narrow_list() const;
    void render_pp_narrow_list() const;

    void render_bounding_boxes() const;
    void render_quad_tree_lines() const;

    friend class demo_app;
};
} // namespace ppx::demo