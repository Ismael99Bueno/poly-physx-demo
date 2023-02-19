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
        push_layer(&p_phys_panel);
        push_layer(&p_perf_panel);
        push_layer(&p_engine_panel);
        push_layer(&p_actions_panel);
        push_layer(&p_menu_bar);
    }

    void demo_app::on_start()
    {
        p_grabber.start();
        p_selector.start();
        p_outline_manager.start();
        p_predictor.start();
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

        out.write("actions_enabled", p_actions_panel.p_enabled);
        out.write("engine_enabled", p_engine_panel.p_enabled);
        out.write("phys_enabled", p_phys_panel.p_enabled);
        out.write("perf_enabled", p_perf_panel.p_enabled);

        out.begin_section("adder");
        p_adder.write(out);
        out.end_section();
        out.begin_section("grabber");
        p_grabber.write(out);
        out.end_section();
        out.begin_section("selector");
        p_selector.write(out);
        out.end_section();
        out.begin_section("attacher");
        p_attacher.write(out);
        out.end_section();
        out.begin_section("engine_panel");
        p_engine_panel.write(out);
        out.end_section();
        out.begin_section("perf_panel");
        p_perf_panel.write(out);
        out.end_section();
        out.begin_section("phys_panel");
        p_phys_panel.write(out);
        out.end_section();
    }
    void demo_app::read(ini::input &in)
    {
        app::read(in);
        m_session = in.readstr("session");
        m_has_session = (bool)in.readi("has_session");

        p_actions_panel.p_enabled = (bool)in.readi("actions_enabled");
        p_engine_panel.p_enabled = (bool)in.readi("engine_enabled");
        p_phys_panel.p_enabled = (bool)in.readi("phys_enabled");
        p_perf_panel.p_enabled = (bool)in.readi("perf_enabled");

        in.begin_section("adder");
        p_adder.read(in);
        in.end_section();
        in.begin_section("grabber");
        p_grabber.read(in);
        in.end_section();
        in.begin_section("selector");
        p_selector.read(in);
        in.end_section();
        in.begin_section("attacher");
        p_attacher.read(in);
        in.end_section();
        in.begin_section("engine_panel");
        p_engine_panel.read(in);
        in.end_section();
        in.begin_section("perf_panel");
        p_perf_panel.read(in);
        in.end_section();
        in.begin_section("phys_panel");
        p_phys_panel.read(in);
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
        p_grabber.update();
        p_attacher.update(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
        p_predictor.update();
    }

    void demo_app::on_render()
    {
        PERF_FUNCTION()
        draw_interaction_lines();
        p_grabber.render();
        p_selector.render();
        p_adder.render();
        p_copy_paste.render();
        p_attacher.render(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
        p_predictor.render();
#ifdef DEBUG
        ImGui::ShowDemoWindow();
        ImPlot::ShowDemoWindow();
#endif
    }

    void demo_app::on_late_update()
    {
        PERF_FUNCTION()
        p_outline_manager.update();
    }

    void demo_app::on_entity_draw(const phys::entity2D_ptr &e, sf::ConvexShape &shape)
    {
        if (p_selector.is_selecting(e))
            p_outline_manager.load_outline(e.index(), sf::Color::Red, 3);
    }

    void demo_app::on_event(sf::Event &event)
    {
        switch (event.type)
        {
        case sf::Event::MouseButtonPressed:
            p_copy_paste.delete_copy();
            switch (p_actions_panel.action())
            {
            case actions_panel::ADD:
                p_adder.setup();
                break;
            case actions_panel::GRAB:
                p_grabber.try_grab_entity();
                break;
            case actions_panel::ATTACH:
                if (p_attacher.has_first())
                    p_attacher.try_attach_second(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                else
                    p_attacher.try_attach_first(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                break;
            case actions_panel::ENTITIES:
                p_selector.begin_select(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                break;
            default:
                break;
            }
            break;

        case sf::Event::MouseButtonReleased:
        {
            switch (p_actions_panel.action())
            {
            case actions_panel::ADD:
                p_adder.add();
                break;
            case actions_panel::GRAB:
                p_grabber.null();
                break;
            case actions_panel::ENTITIES:
                p_selector.end_select();
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
                const auto selected = p_selector.get();
                for (phys::const_entity2D_ptr e : selected)
                    if (e.try_validate())
                        engine().remove_entity(e);
                p_attacher.cancel();
                p_adder.cancel();
                break;
            }
            case sf::Keyboard::C:
                p_copy_paste.copy();
                break;
            case sf::Keyboard::V:
                p_copy_paste.paste();
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
        phys::engine2D &eng = engine();
        const phys::const_entity2D_ptr e1 = eng[world_mouse()];
        if (e1)
            for (const auto &inter : eng.interactions())
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
        const sf::Color prev_color = entity_color();
        entity_color(sf::Color(86, 113, 137));

        const float w = 0.5f * WIDTH * PIXEL_TO_WORLD, h = 0.5f * HEIGHT * PIXEL_TO_WORLD;
        const float thck = 20.f;

        phys::engine2D &eng = engine();
        const phys::entity2D_ptr e1 = eng.add_entity({-w - 0.5f * thck, 0.f}),
                                 e2 = eng.add_entity({w + 0.5f * thck, 0.f}),
                                 e3 = eng.add_entity({0.f, -h - 0.5f * thck}),
                                 e4 = eng.add_entity({0.f, h + 0.5f * thck});

        e1->shape(geo::polygon2D::rect(thck, 2.f * (h + thck)));
        e2->shape(geo::polygon2D::rect(thck, 2.f * (h + thck)));
        e3->shape(geo::polygon2D::rect(2.f * w, thck));
        e4->shape(geo::polygon2D::rect(2.f * w, thck));

        e1->dynamic(false);
        e2->dynamic(false);
        e3->dynamic(false);
        e4->dynamic(false);
        entity_color(prev_color);
    }

    const std::string &demo_app::session() const { return m_session; }
    void demo_app::session(const std::string &session)
    {
        m_session = session;
        m_has_session = true;
    }
    bool demo_app::has_session() const { return m_has_session; }
}