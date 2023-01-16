#ifndef COPY_PASTE_HPP
#define COPY_PASTE_HPP

#include "templates.hpp"
#include "selector.hpp"
#include <unordered_map>
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class copy_paste
    {
    public:
        copy_paste(const selector &s);

        void render();
        void copy();
        void paste();
        void delete_copy();

    private:
        const selector &m_selector;
        alg::vec2 m_ref_pos;
        std::unordered_map<std::size_t, std::pair<entity_template, sf::ConvexShape>> m_entities;
        std::vector<spring_template> m_springs;
        std::vector<rigid_bar_template> m_rbars;
        bool m_has_copy = false;

        void preview();
    };
}

#endif