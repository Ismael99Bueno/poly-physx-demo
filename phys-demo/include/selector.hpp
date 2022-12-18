#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "entity_ptr.hpp"
#include <unordered_set>
#include <utility>
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class selector
    {
    public:
        selector(sf::RenderWindow &window, std::vector<phys::entity2D> &entities);

        void begin_select(const alg::vec2 &mpos, bool clear_previous = true);
        void draw_select_box(const alg::vec2 &mpos) const;
        bool is_selecting(const phys::const_entity_ptr &e, const alg::vec2 &mpos) const;
        void select(const alg::vec2 &mpos);
        bool validate();

        const std::unordered_set<phys::const_entity_ptr> &get() const;

    private:
        sf::RenderWindow &m_window;
        std::vector<phys::entity2D> &m_entities;
        std::unordered_set<phys::const_entity_ptr> m_selected;
        alg::vec2 m_mpos_start;
        bool m_selecting = false;

        std::pair<alg::vec2, alg::vec2> minmax(const alg::vec2 &mpos) const;
    };
}

#endif