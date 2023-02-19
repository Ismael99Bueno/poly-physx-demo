#ifndef ADDER_HPP
#define ADDER_HPP

#include "templates.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class adder : ini::saveable
    {
    public:
        enum shape_type
        {
            BOX = 0,
            RECT = 1,
            NGON = 2
        };

        struct add_template : private ini::saveable
        {
            std::string name;
            entity_template entity_templ;
            shape_type shape = BOX;
            float size = 5.f, width = 5.f, height = 5.f, radius = 0.6f * 5.f;
            std::uint32_t sides = 3;
            sf::Color color;

            void write(ini::output &out) const override;
            void read(ini::input &in) override;
        };

        adder() = default;

        void render();
        void setup();
        void add(bool definitive = true);

        void save_template(const std::string &name);
        void load_template(const std::string &name);
        void erase_template(const std::string &name);
        void save_template();
        void load_template();
        void erase_template();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        const std::map<std::string, add_template> &templates() const;
        bool has_saved_entity() const;

        add_template p_current_templ;

    private:
        std::map<std::string, add_template> m_templates;

        alg::vec2 m_start_pos;
        bool m_adding = false;

        sf::ConvexShape m_preview;

        std::pair<alg::vec2, alg::vec2> pos_vel_upon_addition() const;
        void update_template();
        void setup_preview();
        void preview();

        void draw_velocity_arrow();
    };
}

#endif