#include "demo_app.hpp"
#include "constants.hpp"

namespace phys_demo
{
    demo_app::demo_app() : app()
    {
        const auto validate = [this](const std::size_t index)
        {m_grabber.validate();
        m_selector.validate(); };
        engine().on_entity_removal(validate);

        push_layer(&m_phys_panel);
        push_layer(&m_perf_panel);
        push_layer(&m_engine_panel);
        push_layer(&m_actions_panel);

        add_borders();
    }

    void demo_app::on_update()
    {
        draw_interaction_lines();
        if (m_grabber)
            m_grabber.move_grabbed_entity();
        m_selector.draw_select_box();
        if (m_engine_panel.visualize_quad_tree())
            draw_quad_tree(engine().collider().quad_tree());
        if (m_adding)
        {
            const auto [pos, vel] = pos_vel_upon_addition();
            m_previewer.preview(pos, vel);
        }
#ifdef DEBUG
        ImGui::ShowDemoWindow();
#endif
    }

    void demo_app::on_entity_draw(const phys::const_entity_ptr &e, sf::ConvexShape &shape)
    {
        const float ampl = 1.5f, freq = 2.5f;

        if (m_selector.is_selecting(e))
            shape.setOutlineThickness(ampl * (2.0f + std::sinf(freq * m_clock.getElapsedTime().asSeconds())));
        else
            shape.setOutlineThickness(0);
    }

    void demo_app::on_event(sf::Event &event)
    {
        switch (event.type)
        {
        case sf::Event::MouseButtonPressed:
            switch (m_actions_panel.action())
            {
            case actions_panel::ADD:
                m_mouse_add = world_mouse();
                m_adding = true;
                m_previewer.setup(&m_actions_panel.templ());
                break;
            case actions_panel::GRAB:
                m_grabber.try_grab_entity();
                break;
            case actions_panel::ENTITIES:
                m_selector.begin_select(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                break;
            default:
                break;
            }
            break;

        case sf::Event::MouseButtonReleased:
        {
            switch (m_actions_panel.action())
            {
            case actions_panel::ADD:
                add_entity_template();
                m_adding = false;
                break;
            case actions_panel::GRAB:
                m_grabber.null();
                break;
            case actions_panel::ENTITIES:
                m_selector.end_select();
                break;
            default:
                break;
            }
            break;
        }
        case sf::Event::KeyPressed:
            switch (event.key.code)
            {
            case sf::Keyboard::Backspace:
            {
                const std::unordered_set<phys::const_entity_ptr> selected = m_selector.get();
                for (phys::const_entity_ptr e : selected)
                    if (e.try_validate())
                        engine().remove_entity(e);
                break;
            }
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    void demo_app::draw_quad_tree(const phys::quad_tree2D &qt)
    {
        if (qt.partitioned())
            for (const auto &child : qt.children())
                draw_quad_tree(*child);
        else
        {
            const alg::vec2 &mm = qt.aabb().min(),
                            &mx = qt.aabb().max();
            sf::Vertex vertices[5];
            vertices[0].position = alg::vec2(mm.x, mx.y) * WORLD_TO_PIXEL;
            vertices[1].position = mx * WORLD_TO_PIXEL;
            vertices[2].position = alg::vec2(mx.x, mm.y) * WORLD_TO_PIXEL;
            vertices[3].position = mm * WORLD_TO_PIXEL;
            vertices[4].position = vertices[0].position;
            window().draw(vertices, 5, sf::LineStrip);
        }
    }

    void demo_app::draw_interaction_lines()
    {
        const phys::const_entity_ptr e1 = engine()[world_mouse()];
        if (e1)
            for (const auto &inter : engine().interactions())
                if (inter->contains(e1))
                    for (const auto &e2 : inter->entities())
                        if (e1 != e2)
                        {
                            sf::Vertex line[2];
                            sf::Color c1 = shapes()[e1.index()].getFillColor(),
                                      c2 = shapes()[e2.index()].getFillColor();
                            c1.a = 120;
                            c2.a = 120;
                            line[0].position = e1->pos() * WORLD_TO_PIXEL;
                            line[0].color = c1;
                            line[1].position = e2->pos() * WORLD_TO_PIXEL;
                            line[1].color = c2;
                            window().draw(line, 2, sf::Lines);
                        }
    }

    void demo_app::add_entity_template()
    {
        const auto [pos, vel] = pos_vel_upon_addition();
        const entity_template &templ = m_actions_panel.templ();
        engine().add_entity(pos, templ.dynamic ? vel : alg::vec2(),
                            std::atan2f(vel.y, vel.x),
                            0.f,
                            templ.mass,
                            templ.charge,
                            templ.vertices,
                            templ.dynamic);
    }

    void demo_app::add_borders()
    {
        const float w = 0.5f * WIDTH * PIXEL_TO_WORLD, h = 0.5f * HEIGHT * PIXEL_TO_WORLD;
        const float thck = 20.f;
        phys::engine2D &eng = engine();

        const phys::entity_ptr e1 = eng.add_entity({-w - 0.4f * thck, 0.f}),
                               e2 = eng.add_entity({w + 0.4f * thck, 0.f}),
                               e3 = eng.add_entity({0.f, -h - 0.4f * thck}),
                               e4 = eng.add_entity({0.f, h + 0.4f * thck});

        e1->shape(geo::polygon2D(geo::polygon2D::rect(thck, 2.f * h - 0.6f * thck)));
        e2->shape(geo::polygon2D(geo::polygon2D::rect(thck, 2.f * h - 0.6f * thck)));
        e3->shape(geo::polygon2D(geo::polygon2D::rect(2.f * w, thck)));
        e4->shape(geo::polygon2D(geo::polygon2D::rect(2.f * w, thck)));

        e1->dynamic(false);
        e2->dynamic(false);
        e3->dynamic(false);
        e4->dynamic(false);
    }

    std::pair<alg::vec2, alg::vec2> demo_app::pos_vel_upon_addition() const
    {
        const float speed_mult = 0.5f;
        const alg::vec2 pos = m_mouse_add,
                        vel = speed_mult * (m_mouse_add - world_mouse());
        return std::make_pair(pos, vel);
    }
}