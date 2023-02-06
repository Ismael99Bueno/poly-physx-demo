#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "engine2D.hpp"
#include <unordered_set>
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class selector : ini::saveable
    {
    public:
        selector(phys::engine2D &engine, std::size_t allocations = 100);

        void render();

        void begin_select(bool clear_previous = true);
        void end_select();

        bool is_selecting(const phys::entity2D_ptr &e) const;
        bool is_selected(const phys::entity2D_ptr &e) const;

        void select(const phys::entity2D_ptr &e);
        void deselect(const phys::entity2D_ptr &e);

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        const std::unordered_set<phys::entity2D_ptr> &get() const;

    private:
        std::unordered_set<phys::entity2D_ptr> m_selected;
        alg::vec2 m_mpos_start;
        bool m_selecting = false;

        void draw_select_box() const;
        geo::aabb2D select_box() const;
    };
}

#endif