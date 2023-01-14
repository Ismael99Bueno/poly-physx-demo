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
        if (m_copy_paste.has_copy())
            m_copy_paste.preview();
#ifdef DEBUG
        ImGui::ShowDemoWindow();
#endif
    }

    void demo_app::on_late_update()
    {
        if (m_attacher.has_first())
        {
            m_attacher.rotate_joint();
            m_attacher.draw_unattached_joint();
        }
        m_outline_manager.reset_priorities();
    }

    void demo_app::on_entity_draw(const phys::entity_ptr &e, sf::ConvexShape &shape)
    {
        if (m_selector.is_selecting(e))
            m_outline_manager.load_outline(e.index(), sf::Color::Red, 3);
        m_outline_manager.paint_outline(e.index(), shape);
    }

    void demo_app::on_event(sf::Event &event)
    {
        switch (event.type)
        {
        case sf::Event::MouseButtonPressed:
            m_copy_paste.delete_copy();
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
            case sf::Keyboard::C:
                m_copy_paste.copy();
                break;
            case sf::Keyboard::V:
                m_copy_paste.paste();
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
}