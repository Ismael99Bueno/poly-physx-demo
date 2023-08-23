#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/utility/group_manager.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
group_manager::group_manager(demo_app &app) : m_app(app)
{
    m_window = m_app.window();
}

void group_manager::update()
{
    if (!m_ongoing_group)
        return;
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_position;
    for (std::size_t i = 0; i < m_group_shapes_preview.size(); i++)
    {
        m_group_shapes_preview[i]->transform.position = m_current_group.body_templates[i].specs.position + offset_pos;
        m_group_shapes_preview[i]->transform.rotation = m_current_group.body_templates[i].specs.rotation;
    }
}
void group_manager::render() const
{
    if (!m_ongoing_group)
        return;
    for (const auto &shape : m_group_shapes_preview)
        m_window->draw(*shape);
}

void group_manager::begin_group_from_selected()
{
    m_current_group = create_group_from_selected();
    update_preview_from_current_group();
    m_ongoing_group = true;
}

void group_manager::update_preview_from_current_group()
{
    m_group_shapes_preview.clear();
    for (const body_template &btemplate : m_current_group.body_templates)
        if (btemplate.specs.shape == body2D::shape_type::POLYGON)
            m_group_shapes_preview
                .emplace_back(kit::make_scope<lynx::polygon2D>(btemplate.specs.vertices, btemplate.color))
                .get();
        else
            m_group_shapes_preview
                .emplace_back(kit::make_scope<lynx::ellipse2D>(btemplate.specs.radius, btemplate.color))
                .get();
}

void group_manager::cancel_group()
{
    m_ongoing_group = false;
}

void group_manager::paste_group()
{
    if (!m_ongoing_group)
        return;
    std::unordered_map<kit::uuid, body2D::ptr> old_id_to_new_body;
    const glm::vec2 offset_pos = m_app.world_mouse_position() - m_current_group.mean_position;

    for (const body_template &btemplate : m_current_group.body_templates)
    {
        body2D::specs specs = btemplate.specs;
        specs.position += offset_pos;
        old_id_to_new_body[btemplate.id] = m_app.world.add_body(specs);
    }

    for (spring_template &sptemplate : m_current_group.spring_templates)
    {
        sptemplate.specs.body1 = old_id_to_new_body[sptemplate.id1];
        sptemplate.specs.body2 = old_id_to_new_body[sptemplate.id2];
        m_app.world.add_spring(sptemplate.specs);
    }

    for (revolute_template &rjtemplate : m_current_group.revolute_templates)
    {
        rjtemplate.specs.body1 = old_id_to_new_body[rjtemplate.id1];
        rjtemplate.specs.body2 = old_id_to_new_body[rjtemplate.id2];
        m_app.world.add_constraint<revolute_joint2D>(rjtemplate.specs);
    }
}

group_manager::group group_manager::create_group_from_selected()
{
    group created_group{};
    const auto &selected = m_app.selector.selected_bodies();
    constexpr std::uint32_t alpha = 120;

    for (const body2D::ptr &body : selected)
    {
        const lynx::color color{m_app.shapes()[body->index]->color(), alpha};
        created_group.body_templates.emplace_back(body->id, color, body2D::specs::from_body(*body));
        created_group.mean_position += body->transform().position;
    }
    created_group.mean_position /= created_group.body_templates.size();

    const auto &body_templates = created_group.body_templates;

    for (std::size_t i = 0; i < body_templates.size(); i++)
    {
        const kit::uuid id1 = body_templates[i].id;
        for (std::size_t j = i + 1; j < body_templates.size(); j++)
        {
            const kit::uuid id2 = body_templates[j].id;

            for (const spring2D::ptr &sp : m_app.world.springs_from_ids(id1, id2))
            {
                const lynx::color color{m_app.spring_lines()[sp->index].color(), alpha};
                created_group.spring_templates.emplace_back(id1, id2, color, spring2D::specs::from_spring(*sp));
            }

            for (const constraint2D *ctr : m_app.world.constraints_from_ids({id1, id2}))
            {
                const revolute_joint2D *rj = dynamic_cast<const revolute_joint2D *>(ctr);
                const lynx::color color{m_app.revolute_lines().at(rj).color(), alpha};
                created_group.revolute_templates.emplace_back(id1, id2, color,
                                                              revolute_joint2D::specs::from_revolute_joint(*rj));
            }
        }
    }
    return created_group;
}
} // namespace ppx::demo