#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "entity2D_ptr.hpp"
#include <unordered_set>
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class demo_app;
    class selector
    {
    public:
        selector(demo_app *papp);

        void render();

        void begin_select(bool clear_previous = true);
        void end_select();

        bool is_selecting(const phys::entity2D_ptr &e) const;
        bool is_selected(const phys::entity2D_ptr &e) const;

        void select(const phys::entity2D_ptr &e);
        void deselect(const phys::entity2D_ptr &e);

        const std::unordered_set<phys::entity2D_ptr> &get() const;

    private:
        demo_app *m_app;
        std::unordered_set<phys::entity2D_ptr> m_selected;
        alg::vec2 m_mpos_start;
        bool m_selecting = false;

        void draw_select_box() const;
        geo::aabb2D select_box() const;
    };
}

#endif