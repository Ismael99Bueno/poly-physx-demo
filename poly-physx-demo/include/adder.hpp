#ifndef ADDER_HPP
#define ADDER_HPP

#include "templates.hpp"
#include "ppx/app.hpp"
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class adder : public ini::saveable
    {
    public:
        enum shape_type
        {
            RECT = 0,
            NGON = 1,
            CUSTOM = 2
        };

    private:
        struct add_template : private ini::saveable
        {
            std::string name;
            entity_template entity_templ;
            shape_type shape = RECT;
            float width = DEFAULT_SIZE, height = DEFAULT_SIZE, radius = 0.6f * DEFAULT_SIZE;
            std::uint32_t sides = 3;
            sf::Color color = DEFAULT_ENTITY_COLOR;

            void write(ini::output &out) const override;
            void read(ini::input &in) override;
        };

    public:
        adder() = default;

        void render();
        void setup();
        void cancel();
        ppx::entity2D_ptr add(bool definitive = true);

        void save_template(const std::string &name);
        void load_template(const std::string &name);
        void erase_template(const std::string &name);
        void save_template();
        void load_template();
        void erase_template();
        void update_template_vertices();

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        const std::map<std::string, add_template> &templates() const;
        bool has_saved_entity() const;

        add_template p_current_templ;
        bool p_predict_path = true;

    private:
        std::map<std::string, add_template> m_templates;

        glm::vec2 m_start_pos{0.f};
        bool m_adding = false;

        sf::ConvexShape m_preview;

        std::pair<glm::vec2, glm::vec2> pos_vel_upon_addition() const;

        void setup_preview();
        void preview();

        void draw_preview();
        void draw_velocity_arrow() const;
    };
}

#endif