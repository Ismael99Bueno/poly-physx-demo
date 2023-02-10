#include "demo_app.hpp"
#include "constants.hpp"
#include "flat_line_strip.hpp"
#include "flat_line.hpp"
#include "implot.h"
#include "constants.hpp"
#include <filesystem>

namespace phys_demo
{
    demo_app::demo_app() : app()
    {
        push_layer(&m_phys_panel);
        push_layer(&m_perf_panel);
        push_layer(&m_engine_panel);
        push_layer(&m_actions_panel);
        push_layer(&m_menu_bar);
    }

    void demo_app::on_start()
    {
        m_grabber.start();
        m_selector.start();
        m_outline_manager.start();
        save(DEFAULT_SAVE);
        if (!load(LAST_SAVE))
            add_borders();
    }

    void demo_app::on_end() { save(LAST_SAVE); }

    void demo_app::write(ini::output &out) const
    {
        app::write(out);
        out.write("session", m_session);
        out.write("has_session", m_has_session);

        out.begin_section("adder");
        m_adder.write(out);
        out.end_section();
        out.begin_section("grabber");
        m_grabber.write(out);
        out.end_section();
        out.begin_section("selector");
        m_selector.write(out);
        out.end_section();
        out.begin_section("attacher");
        m_attacher.write(out);
        out.end_section();
        out.begin_section("engine_panel");
        m_engine_panel.write(out);
        out.end_section();
        out.begin_section("perf_panel");
        m_perf_panel.write(out);
        out.end_section();
        out.begin_section("phys_panel");
        m_phys_panel.write(out);
        out.end_section();
    }
    void demo_app::read(ini::input &in)
    {
        app::read(in);
        m_session = in.readstr("session");
        m_has_session = (bool)in.readi("has_session");

        in.begin_section("adder");
        m_adder.read(in);
        in.end_section();
        in.begin_section("grabber");
        m_grabber.read(in);
        in.end_section();
        in.begin_section("selector");
        m_selector.read(in);
        in.end_section();
        in.begin_section("attacher");
        m_attacher.read(in);
        in.end_section();
        in.begin_section("engine_panel");
        m_engine_panel.read(in);
        in.end_section();
        in.begin_section("perf_panel");
        m_perf_panel.read(in);
        in.end_section();
        in.begin_section("phys_panel");
        m_phys_panel.read(in);
        in.end_section();
    }

    void demo_app::save(const std::string &filename) const
    {
        if (!std::filesystem::exists(SAVES_DIR))
            std::filesystem::create_directory(SAVES_DIR);

        const std::string filepath = SAVES_DIR + filename;
        ini::output out(filepath.c_str());
        out.begin_section("demo_app");
        write(out);
        out.end_section();
        out.close();
    }

    bool demo_app::load(const std::string &filename)
    {
        if (!std::filesystem::exists(SAVES_DIR))
            std::filesystem::create_directory(SAVES_DIR);

        const std::string filepath = SAVES_DIR + filename;
        ini::input in(filepath.c_str());

        if (!in.is_open())
            return false;
        in.begin_section("demo_app");
        read(in);
        in.end_section();
        in.close();
        return true;
    }

    void demo_app::save() const
    {
        DBG_ASSERT(m_has_session, "No current session active. Must specify a specific session name to save.\n")
        save(m_session);
    }
    bool demo_app::load()
    {
        DBG_ASSERT(m_has_session, "No current session active. Must specify a specific session name to load.\n")
        return load(m_session);
    }

    void demo_app::on_update()
    {
        PERF_FUNCTION()
        m_grabber.update();
        m_attacher.update(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
    }

    void demo_app::on_render()
    {
        PERF_FUNCTION()
        draw_interaction_lines();
        m_grabber.render();
        m_selector.render();
        m_adder.render();
        m_copy_paste.render();
        m_attacher.render(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
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
                m_adder.setup();
                break;
            case actions_panel::GRAB:
                m_grabber.try_grab_entity();
                break;
            case actions_panel::ATTACH:
                if (m_attacher.has_first())
                    m_attacher.try_attach_second(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                else
                    m_attacher.try_attach_first(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
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

    void demo_app::add_borders()
    {
        const float w = 0.5f * WIDTH * PIXEL_TO_WORLD, h = 0.5f * HEIGHT * PIXEL_TO_WORLD;
        const float thck = 20.f;

        const phys::entity2D_ptr e1 = engine().add_entity({-w - 0.5f * thck, 0.f}),
                                 e2 = engine().add_entity({w + 0.5f * thck, 0.f}),
                                 e3 = engine().add_entity({0.f, -h - 0.5f * thck}),
                                 e4 = engine().add_entity({0.f, h + 0.5f * thck});

        e1->shape(geo::polygon2D::rect(thck, 2.f * (h + thck)));
        e2->shape(geo::polygon2D::rect(thck, 2.f * (h + thck)));
        e3->shape(geo::polygon2D::rect(2.f * w, thck));
        e4->shape(geo::polygon2D::rect(2.f * w, thck));

        e1->dynamic(false);
        e2->dynamic(false);
        e3->dynamic(false);
        e4->dynamic(false);
    }

    grabber &demo_app::grabber() { return m_grabber; }
    selector &demo_app::selector() { return m_selector; }
    adder &demo_app::adder() { return m_adder; }
    attacher &demo_app::attacher() { return m_attacher; }
    outline_manager &demo_app::outline_manager() { return m_outline_manager; }
    copy_paste &demo_app::copy_paste() { return m_copy_paste; }

    const std::string &demo_app::session() const { return m_session; }
    void demo_app::session(const std::string &session)
    {
        m_session = session;
        m_has_session = true;
    }
    bool demo_app::has_session() const { return m_has_session; }
}