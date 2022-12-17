#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "engine2D.hpp"
#include "actions_panel.hpp"
#include "engine_panel.hpp"
#include "perf_panel.hpp"
#include "grabber.hpp"
#include "selector.hpp"

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>

namespace phys_env
{
    class environment : public phys::engine2D
    {
    public:
        environment(const rk::butcher_tableau &table,
                    float dt = 0.001f,
                    std::size_t allocations = 100,
                    const std::string &wname = "Physics engine");

        phys::entity_ptr add_entity(const phys::body2D &body, const std::vector<alg::vec2> &vertices = geo::polygon2D::box(1.f)) override;
        phys::entity_ptr add_entity(const alg::vec2 &pos = {0.f, 0.f},
                                    const alg::vec2 &vel = {0.f, 0.f},
                                    float angpos = 0.f, float angvel = 0.f,
                                    float mass = 1.f, float charge = 1.f,
                                    const std::vector<alg::vec2> &vertices = geo::polygon2D::box(1.f)) override;

        void remove_entity(std::size_t index) override;
        void remove_entity(const phys::const_entity_ptr &e) override;

        using forward_fun = std::function<bool(engine2D &, float &)>;
        void run(const forward_fun &forward = &engine2D::raw_forward,
                 const std::string &wname = "Physics engine");

    private:
        std::vector<sf::ConvexShape> m_shapes;
        sf::RenderWindow m_window;

        alg::vec2 m_mouse_press;
        grabber m_grabber;
        selector m_selector;

        actions_panel m_actions;
        engine_panel m_eng_panel;
        perf_panel m_perf;
        int m_integrations_per_frame = 10;
        float m_dt;
        bool m_visualize_qt;

        sf::Clock m_clock;

        void add_shape(const geo::polygon2D &poly, sf::Color = sf::Color::Green);
        void add_entity_template();
        void handle_events();
        void draw_quad_tree(const phys::quad_tree2D &qt);
        void draw_entities();
        alg::vec2 world_mouse() const;
        alg::vec2 world_mouse_delta() const;
    };
}

#endif