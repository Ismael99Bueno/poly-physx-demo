#include "demo_app.hpp"
#include "constants.hpp"
#include "flat_line_strip.hpp"
#include "flat_line.hpp"

namespace phys_demo
{
    demo_app::demo_app() : app()
    {
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
        if (m_adder.adding())
        {
            const auto [pos, vel] = m_adder.pos_vel_upon_addition();
            m_previewer.preview(pos, vel);
        }
        if (m_attacher.has_first())
        {
            m_attacher.rotate_joint();
            m_attacher.draw_unattached_joint();
        }
#ifdef DEBUG
        ImGui::ShowDemoWindow();
#endif
    }

    void demo_app::on_entity_draw(const phys::entity_ptr &e, sf::ConvexShape &shape)
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
                m_adder.setup();
                m_previewer.setup(&m_actions_panel.templ());
                break;
            case actions_panel::GRAB:
                m_grabber.try_grab_entity();
                break;
            case actions_panel::ATTACH:
                if (m_attacher.has_first())
                    m_attacher.try_attach_second();
                else
                    m_attacher.try_attach_first();
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
                m_adder.add(m_actions_panel.templ());
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
                const auto selected = m_selector.get();
                for (phys::const_entity_ptr e : selected)
                    if (e.try_validate())
                        engine().remove_entity(e);
                m_attacher.cancel();
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
            prm::flat_line_strip fls({alg::vec2(mm.x, mx.y) * WORLD_TO_PIXEL,
                                      mx * WORLD_TO_PIXEL,
                                      alg::vec2(mx.x, mm.y) * WORLD_TO_PIXEL,
                                      mm * WORLD_TO_PIXEL,
                                      alg::vec2(mm.x, mx.y) * WORLD_TO_PIXEL});
            window().draw(fls);
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
                            sf::Color c1 = shapes()[e1.index()].getFillColor(),
                                      c2 = shapes()[e2.index()].getFillColor();
                            c1.a = 120;
                            c2.a = 120;
                            prm::flat_line fl(e1->pos() * WORLD_TO_PIXEL,
                                              e2->pos() * WORLD_TO_PIXEL,
                                              c1, c2);
                            window().draw(fl);
                        }
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
}