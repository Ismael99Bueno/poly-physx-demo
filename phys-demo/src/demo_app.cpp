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
        p_trails.start();
        p_follower.start();
        save(DEFAULT_SAVE);
        if (!load(LAST_SAVE))
            add_borders();
    }

    void demo_app::on_end() { save(LAST_SAVE); }

    void demo_app::write(ini::output &out) const
    {
        app::write(out);
        out.write("session", m_session);

        for (const auto &[section, saveable] : m_saveables)
        {
            out.begin_section(section);
            saveable->write(out);
            out.end_section();
        }
    }
    void demo_app::read(ini::input &in)
    {
        app::read(in);
        m_session = in.readstr("session");

        for (const auto &[section, saveable] : m_saveables)
        {
            in.begin_section(section);
            saveable->read(in);
            in.end_section();
        }
    }

    bool demo_app::validate_session()
    {
        const bool exists = std::filesystem::exists(SAVES_DIR + m_session);
        if (!exists)
            m_session.clear();
        return exists;
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
        validate_session(); // This may be redundant
        return true;
    }

    void demo_app::save() const
    {
        DBG_ASSERT(has_session(), "No current session active. Must specify a specific session name to save.\n")
        save(m_session);
    }
    bool demo_app::load()
    {
        DBG_ASSERT(has_session(), "No current session active. Must specify a specific session name to load.\n")
        return load(m_session);
    }

    void demo_app::on_update()
    {
        PERF_FUNCTION()
        p_grabber.update();
        p_attacher.update(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
        p_predictor.update();
        p_trails.update();
        p_follower.update();
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
        p_trails.render();
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
            if (ImGui::GetIO().WantCaptureKeyboard)
                break;
            switch (event.key.code)
            {
            case sf::Keyboard::Backspace:
                remove_selected();
                break;
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
        const phys::engine2D &eng = engine();
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

    void demo_app::remove_selected()
    {
        const auto selected = p_selector.get();
        for (phys::const_entity2D_ptr e : selected)
            if (e.try_validate())
                engine().remove_entity(*e);

        p_attacher.cancel();
        p_adder.cancel();
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

        e1->kynematic(false);
        e2->kynematic(false);
        e3->kynematic(false);
        e4->kynematic(false);
        entity_color(prev_color);
    }

    const std::string &demo_app::session() const { return m_session; }
    void demo_app::session(const std::string &session) { m_session = session; }
    bool demo_app::has_session() const { return !m_session.empty(); }
}