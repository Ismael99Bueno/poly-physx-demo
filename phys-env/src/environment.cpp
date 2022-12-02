#include "environment.hpp"
#include "force2D.hpp"
#include "perf.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include <iostream>

class gravity : public phys::force2D
{
    std::pair<alg::vec2, float> force(const phys::entity2D &e) const override { return {{0.f, -100.f}, 0.f}; }
};
gravity g;

namespace phys_env
{
    environment::environment(const rk::tableau &table,
                             const float dt,
                             const std::size_t allocations,
                             const std::string &wname) : engine2D(table, allocations),
                                                         m_dt(dt),
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
        const phys::entity_ptr e = engine2D::add_entity(pos, vel, angpos, angvel, mass, charge, vertices);
        // Base class will already call derived overridden method
        return e;
    }

    void environment::add_shape(const geo::polygon2D &poly, sf::Color color)
    {
        sf::ConvexShape &shape = m_shapes.emplace_back(sf::ConvexShape());
        shape.setPointCount(poly.size());
        for (std::size_t i = 0; i < poly.size(); i++)
            shape.setPoint(i, poly[i] * WORLD_TO_PIXEL);
        shape.setFillColor(sf::Color::Green);
    }

    void environment::run(bool (engine2D::*forward)(float &),
                          const std::string &wname)
    {
        PERF_FUNCTION()
        m_window.setFramerateLimit(60);
        assert(ImGui::SFML::Init(m_window) && "ImGui initialization failed.");

        sf::Clock dclock;
        while (m_window.isOpen())
        {
            PERF_SCOPE("FRAME")
            handle_events();

            ImGui::SFML::Update(m_window, dclock.restart());
            m_window.clear();

            {
                PERF_SCOPE("PHYSICS")
                for (std::size_t i = 0; i < 30; i++)
                    (this->*forward)(m_dt);
            }

            {
                PERF_SCOPE("DRAWING")
                draw_entities();
                m_gui.actions().render();
#ifdef DEBUG
                ImGui::ShowDemoWindow();
#endif
                ImGui::SFML::Render(m_window);
                m_window.display();
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
                if (event.key.code == sf::Keyboard::Escape)
                    m_window.close();
                break;

            case sf::Event::MouseButtonPressed:
                m_grab = cartesian_mouse();
                break;

            case sf::Event::MouseButtonReleased:
            {
                const alg::vec2 release = cartesian_mouse();
                if (m_gui.actions().adding_entity())
                {
                    const alg::vec2 pos = m_grab * PIXEL_TO_WORLD,
                                    vel = (0.3f * PIXEL_TO_WORLD) * (m_grab - release);
                    add_entity(phys::body2D(pos, vel, 0.f, 0.f,
                                            m_gui.actions().templ().mass(),
                                            m_gui.actions().templ().charge()),
                               m_gui.actions().templ().vertices());
                }
                break;
            }

            default:
                break;
            }
        }
    }

    void environment::draw_entities()
    {
        retrieve();
        for (std::size_t i = 0; i < m_shapes.size(); i++)
        {
            for (std::size_t j = 0; j < m_shapes[i].getPointCount(); j++)
                m_shapes[i].setPoint(j, m_entities[i].shape()[j] * WORLD_TO_PIXEL);
            m_window.draw(m_shapes[i]);
        }
    }

    alg::vec2 environment::cartesian_mouse() const
    {
        const sf::Vector2i mpos = sf::Mouse::getPosition(m_window);
        const sf::Vector2f wpos = m_window.mapPixelToCoords(mpos);
        return {wpos.x, wpos.y}; //{x - WIDTH / 2.f, HEIGHT / 2.f - y};
    }
}