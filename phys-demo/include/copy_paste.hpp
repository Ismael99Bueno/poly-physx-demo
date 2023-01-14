#ifndef COPY_PASTE_HPP
#define COPY_PASTE_HPP

#include "entity_template.hpp"
#include "spring2D.hpp"
#include "rigid_bar2D.hpp"
#include "selector.hpp"
#include <unordered_map>
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    class demo_app;
    class copy_paste
    {
    public:
        copy_paste(demo_app *papp, const selector &s);

        void copy();
        void paste();
        void delete_copy();
        void preview();

        bool has_copy() const;

    private:
        demo_app *m_app;
        const selector &m_selector;
        alg::vec2 m_ref_pos;
        std::unordered_map<std::size_t, std::pair<entity_template, sf::ConvexShape>> m_entities;
        std::vector<phys::spring2D> m_springs;
        std::vector<phys::rigid_bar2D> m_rbars;
        bool m_has_copy = false;
    };
}

#endif