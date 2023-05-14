#ifndef ADDER_HPP
#define ADDER_HPP

#include "globals.hpp"
#include "ppx-app/app.hpp"
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class adder
    {
    public:
        enum shape_type
        {
            RECT = 0,
            NGON = 1,
            CIRCLE = 2,
            CUSTOM = 3
        };

    private:
        struct add_specs
        {
            std::string name;
            ppx::entity2D::specs entity_spec;
            shape_type shape = RECT;
            float width = DEFAULT_SIZE, height = DEFAULT_SIZE,
                  ngon_radius = 0.6f * DEFAULT_SIZE,
                  circle_radius = 0.6f * DEFAULT_SIZE,
                  sb_stiffness = 250.f, sb_dampening = 1.2f, sb_radius = 1.f;
            bool soft_body = false;
            std::uint32_t entities_between_vertices = 0;

            std::uint32_t sides = 3;
            sf::Color color = PPX_DEFAULT_ENTITY_COLOR;
        };

    public:
        adder() = default;

        void render();
        void setup();
        void cancel();

        void save_template(const std::string &name);
        void load_template(const std::string &name);
        void erase_template(const std::string &name);
        void save_template();
        void load_template();
        void erase_template();
        void update_template_vertices();

        void add();

        const std::map<std::string, add_specs> &templates() const;
        bool has_saved_entity() const;

        add_specs p_add_specs;
        bool p_predict_path = true;

    private:
        std::map<std::string, add_specs> m_templates;

        glm::vec2 m_start_pos{0.f};
        bool m_adding = false;

        std::unique_ptr<sf::Shape> m_preview;

        ppx::entity2D_ptr add_entity(bool definitive = true);
        void add_soft_body();

        glm::vec2 vel_upon_addition() const;

        void setup_entity_preview();
        void preview_entity();

        void draw_entity_preview();
        void draw_velocity_arrow() const;

        friend YAML::Emitter &operator<<(YAML::Emitter &, const adder &);
        friend struct YAML::convert<adder>;
        friend YAML::Emitter &operator<<(YAML::Emitter &, const add_specs &);
        friend struct YAML::convert<add_specs>;
    };

    YAML::Emitter &operator<<(YAML::Emitter &out, const adder &addr);
    YAML::Emitter &operator<<(YAML::Emitter &out, const adder::add_specs &add_tmpl);
}

namespace YAML
{
    template <>
    struct convert<ppx_demo::adder>
    {
        static Node encode(const ppx_demo::adder &addr);
        static bool decode(const Node &node, ppx_demo::adder &addr);
    };

    template <>
    struct convert<ppx_demo::adder::add_specs>
    {
        static Node encode(const ppx_demo::adder::add_specs &add_tmpl);
        static bool decode(const Node &node, ppx_demo::adder::add_specs &add_tmpl);
    };
}

#endif