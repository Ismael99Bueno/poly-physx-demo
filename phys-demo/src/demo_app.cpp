#include "demo_app.hpp"
#include "constants.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_demo
{
    demo_app::demo_app() : app()
    {
        const auto validate = [this](phys::entity2D &e)
        {m_grabber.validate();
        m_selector.validate(); };
        engine().on_entity_removal(validate);

        push_layer(&m_phys_panel);
        push_layer(&m_perf_panel);
        push_layer(&m_engine_panel);
        push_layer(&m_actions_panel);
    }

    void demo_app::on_update()
    {
        const alg::vec2 mpos = world_mouse(), mdelta = world_mouse_delta();
        if (m_grabber)
            m_grabber.move_grabbed_entity(mpos, mdelta);
        m_selector.draw_select_box(mpos);
        if (m_engine_panel.visualize_quad_tree())
            draw_quad_tree(engine().collider().quad_tree());
#ifdef DEBUG
        ImGui::ShowDemoWindow();
#endif
    }

    void demo_app::on_entity_draw(const phys::const_entity_ptr &e, sf::ConvexShape &shape)
    {
        const alg::vec2 mpos = world_mouse();
        const float ampl = 1.5f, freq = 2.5f;

        if (m_selector.is_selecting(e, mpos))
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
                break;
            case actions_panel::GRAB:
                m_grabber.try_grab_entity(engine().entities(), world_mouse());
                break;
            case actions_panel::SELECT:
                m_selector.begin_select(world_mouse(), !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
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
                break;
            case actions_panel::GRAB:
                m_grabber.null();
                break;
            case actions_panel::SELECT:
                m_selector.select(world_mouse());
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
            const alg::vec2 &pos = qt.pos(), &hdim = qt.dim() * 0.5f;
            sf::Vertex vertices[5];
            vertices[0].position = (pos + alg::vec2(-hdim.x, hdim.y)) * WORLD_TO_PIXEL;
            vertices[1].position = (pos + hdim) * WORLD_TO_PIXEL;
            vertices[2].position = (pos + alg::vec2(hdim.x, -hdim.y)) * WORLD_TO_PIXEL;
            vertices[3].position = (pos - hdim) * WORLD_TO_PIXEL;
            vertices[4].position = vertices[0].position;
            window().draw(vertices, 5, sf::LineStrip);
        }
    }

    void demo_app::add_entity_template()
    {
        const alg::vec2 release = world_mouse();
        const alg::vec2 pos = m_mouse_add,
                        vel = 0.3f * (m_mouse_add - release);
        const entity_template &templ = m_actions_panel.templ();
        engine().add_entity(phys::body2D(pos, vel, 0.f, 0.f,
                                         templ.mass(),
                                         templ.charge()),
                            templ.vertices());
    }
}