#include "environment.hpp"
#include "force2D.hpp"
#include "perf.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include <iostream>
#include <cmath>

class gravity : public phys::force2D
{
    std::pair<alg::vec2, float> force(const phys::entity2D &e) const override { return {{0.f, -100.f * e.mass()}, 0.f}; }
};
gravity g;

namespace phys_env
{
    environment::environment(const rk::butcher_tableau &table,
                             const float dt,
                             const std::size_t allocations,
                             const std::string &wname) : engine2D(table, allocations),
                                                         m_dt(dt),
                                                         m_selector(m_window, entities()),
                                                         m_actions(m_grabber),
                                                         m_eng_panel(integrator(), collider(), m_dt),
                                                         m_perf(m_window),
                                                         m_window(sf::VideoMode::getFullscreenModes()[0], wname, sf::Style::Fullscreen)
    {
        m_window.setView(sf::View(sf::Vector2f(0.f, 0.f), sf::Vector2f(WIDTH, -HEIGHT)));
    }

    phys::entity_ptr environment::add_entity(const phys::body2D &body,
                                             const std::vector<alg::vec2> &vertices)
    {
        const phys::entity_ptr e = engine2D::add_entity(body, vertices);
        add_shape(e->shape());
        return e;
    }
    phys::entity_ptr environment::add_entity(const alg::vec2 &pos,
                                             const alg::vec2 &vel,
                                             float angpos, float angvel,
                                             float mass, float charge,
                                             const std::vector<alg::vec2> &vertices)
    {
        return engine2D::add_entity(pos, vel, angpos, angvel, mass, charge, vertices); // Should call method above
    }

    void environment::remove_entity(const std::size_t index)
    {
        engine2D::remove_entity(index);
        m_grabber.validate();
        m_selector.validate();
        m_shapes[index] = m_shapes.back();
        m_shapes.pop_back();
    }

    void environment::remove_entity(const phys::const_entity_ptr &e)
    {
        engine2D::remove_entity(e); // Should call method above
    }

    void environment::add_entity_template()
    {
        const alg::vec2 release = world_mouse();
        const alg::vec2 pos = m_mouse_press,
                        vel = 0.3f * (m_mouse_press - release);
        const entity_template &templ = m_actions.templ();
        add_entity(phys::body2D(pos, vel, 0.f, 0.f,
                                templ.mass(),
                                templ.charge()),
                   templ.vertices());
    }

    void environment::add_shape(const geo::polygon2D &poly, sf::Color color)
    {
        sf::ConvexShape &shape = m_shapes.emplace_back(sf::ConvexShape());
        shape.setPointCount(poly.size());
        for (std::size_t i = 0; i < poly.size(); i++)
            shape.setPoint(i, poly[i] * WORLD_TO_PIXEL);
        shape.setFillColor(sf::Color::Green);
        shape.setOutlineColor(sf::Color::Red);
    }

    void environment::run(const forward_fun &forward,
                          const std::string &wname)
    {
        m_window.setFramerateLimit(DEFAULT_FPS);
        if (!ImGui::SFML::Init(m_window))
        {
            perror("ImGui initialization failed\n");
            exit(EXIT_FAILURE);
        }

        sf::Clock dclock;
        while (m_window.isOpen())
        {
            PERF_SCOPE("-Frame-")
            handle_events();
#ifndef PERF
            static sf::Time phys_dur, draw_dur;
            {
                sf::Clock clock;
#endif
                for (std::size_t i = 0; i < m_integrations_per_frame; i++)
                    forward(*this, m_dt);
#ifndef PERF
                phys_dur = clock.getElapsedTime();
            }
            sf::Clock clock;
#endif
            {
                PERF_SCOPE("-Drawing-")
                ImGui::SFML::Update(m_window, dclock.restart());
                m_window.clear();
                draw_entities();

                const alg::vec2 mpos = world_mouse();
                if (m_grabber)
                    m_grabber.move_grabbed_entity(m_window, mpos, world_mouse_delta());
                m_selector.draw_select_box(mpos);

                m_actions.render();
                m_eng_panel.render(elapsed(), m_integrations_per_frame);
#ifdef PERF
                m_perf.render();
#else
                m_perf.render_simple(phys_dur, draw_dur);
#endif
                if (m_eng_panel.visualize_quad_tree())
                    draw_quad_tree(collider().quad_tree());
#ifdef DEBUG
                ImGui::ShowDemoWindow();
#endif
                ImGui::SFML::Render(m_window);
                m_window.display();
#ifndef PERF
                draw_dur = clock.getElapsedTime();
#endif
            }
        }
        ImGui::SFML::Shutdown(m_window);
    }

    void environment::handle_events()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type)
            {
            case sf::Event::Closed:
                m_window.close();
                break;

            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::Escape:
                    m_window.close();
                    break;
                case sf::Keyboard::Backspace:
                {
                    const std::unordered_set<phys::const_entity_ptr> selected = m_selector.get();
                    for (phys::const_entity_ptr e : selected)
                        if (e.try_validate())
                            remove_entity(e);
                    break;
                }
                default:
                    break;
                }
                break;

            case sf::Event::MouseButtonPressed:
                switch (m_actions.action())
                {
                case actions_panel::ADD:
                    m_mouse_press = world_mouse();
                    break;
                case actions_panel::GRAB:
                    m_grabber.try_grab_entity(entities(), world_mouse());
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
                switch (m_actions.action())
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

            default:
                break;
            }
        }
    }

    void environment::draw_quad_tree(const phys::quad_tree2D &qt)
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
            m_window.draw(vertices, 5, sf::LineStrip);
        }
    }

    void environment::draw_entities()
    {
        PERF_FUNCTION()
        retrieve();
        const std::vector<phys::entity2D> &etts = entities();
        const alg::vec2 mpos = world_mouse();
        for (std::size_t i = 0; i < m_shapes.size(); i++)
        {
            for (std::size_t j = 0; j < m_shapes[i].getPointCount(); j++)
                m_shapes[i].setPoint(j, etts[i].shape()[j] * WORLD_TO_PIXEL);

            const float ampl = 1.5f, freq = 2.5f;
            if (m_selector.is_selecting({&entities(), i}, mpos))
                m_shapes[i].setOutlineThickness(ampl * (2.0f + std::sinf(freq * m_clock.getElapsedTime().asSeconds())));
            else
                m_shapes[i].setOutlineThickness(0);
            m_window.draw(m_shapes[i]);
        }
    }

    alg::vec2 environment::world_mouse() const
    {
        const sf::Vector2i mpos = sf::Mouse::getPosition(m_window);
        const sf::Vector2f wpos = m_window.mapPixelToCoords(mpos);
        return alg::vec2(wpos.x, wpos.y) * PIXEL_TO_WORLD; //{x - WIDTH / 2.f, HEIGHT / 2.f - y};
    }

    alg::vec2 environment::world_mouse_delta() const
    {
        return alg::vec2(ImGui::GetIO().MouseDelta.x, -ImGui::GetIO().MouseDelta.y) * PIXEL_TO_WORLD;
    }
}