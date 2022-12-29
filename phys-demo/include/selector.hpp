#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "app.hpp"
#include <unordered_set>
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class selector
    {
    public:
        selector(phys::app *papp);

        void begin_select(bool clear_previous = true);
        void end_select();

        bool is_selecting(const phys::const_entity_ptr &e) const;
        bool is_selected(const phys::const_entity_ptr &e) const;

        void select(const phys::const_entity_ptr &e);
        void deselect(const phys::const_entity_ptr &e);

        bool validate();

        void draw_select_box() const;
        const std::unordered_set<phys::const_entity_ptr> &get() const;

    private:
        phys::app *m_app;
        std::unordered_set<phys::const_entity_ptr> m_selected;
        alg::vec2 m_mpos_start;
        bool m_selecting = false;

        geo::aabb2D select_box() const;
    };
}

#endif