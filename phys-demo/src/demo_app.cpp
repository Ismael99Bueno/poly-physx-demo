#include "demo_app.hpp"
#include "constants.hpp"
#include "flat_line_strip.hpp"
#include "flat_line.hpp"
#include "implot.h"

namespace phys_demo
{
    demo_app::demo_app() : app(), m_grabber(engine()),
                           m_selector(engine()),
                           m_outline_manager(engine())
    {
        push_layer(&m_phys_panel);
        push_layer(&m_perf_panel);
        push_layer(&m_engine_panel);
        push_layer(&m_actions_panel);
    }

    void demo_app::on_update()
    {
        PERF_FUNCTION()
        m_grabber.update();
        m_attacher.update();
    }

    void demo_app::on_render()
    {
        PERF_FUNCTION()
        draw_interaction_lines();
        m_grabber.render();
        m_selector.render();
        m_adder.render();
        m_copy_paste.render();
        m_attacher.render();
#ifdef DEBUG
        ImGui::ShowDemoWindow();
        ImPlot::ShowDemoWindow();
#endif
    }

    void demo_app::on_late_update()
    {
        PERF_FUNCTION()
        m_outline_manager.update();
    }

    void demo_app::on_entity_draw(const phys::entity2D_ptr &e, sf::ConvexShape &shape)
    {
        if (m_selector.is_selecting(e))
            m_outline_manager.load_outline(e.index(), sf::Color::Red, 3);
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
                m_adder.setup(&m_actions_panel.templ());
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
                m_adder.add();
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
                for (phys::const_entity2D_ptr e : selected)
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

    demo_app &demo_app::get()
    {
        static demo_app app;
        return app;
    }

    void demo_app::draw_interaction_lines()
    {
        PERF_FUNCTION()
        const phys::const_entity2D_ptr e1 = engine()[world_mouse()];
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

    grabber &demo_app::grabber() { return m_grabber; }
    selector &demo_app::selector() { return m_selector; }
    adder &demo_app::adder() { return m_adder; }
    attacher &demo_app::attacher() { return m_attacher; }
    outline_manager &demo_app::outline_manager() { return m_outline_manager; }
    copy_paste &demo_app::copy_paste() { return m_copy_paste; }
}