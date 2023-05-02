#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "ppx/engine2D.hpp"
#include <unordered_set>
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class selector : public ini::serializable
    {
    public:
        selector(std::size_t allocations = 100);

        void start();
        void render() const;

        void begin_select();
        void end_select(bool clear_previous);

        bool is_selecting(const ppx::entity2D_ptr &e) const;
        bool is_selected(const ppx::entity2D_ptr &e) const;

        void select(const ppx::entity2D_ptr &e);
        void deselect(const ppx::entity2D_ptr &e);

        void serialize(ini::serializer &out) const override;
        void deserialize(ini::deserializer &in) override;

        void update_selected_springs_rbars();

        const std::unordered_set<ppx::entity2D_ptr> &entities() const;
        const std::vector<std::pair<ppx::const_entity2D_ptr, ppx::const_entity2D_ptr>> &springs() const;
        const std::vector<std::pair<ppx::const_entity2D_ptr, ppx::const_entity2D_ptr>> &rbars() const;

    private:
        std::unordered_set<ppx::entity2D_ptr> m_entities;
        std::vector<std::pair<ppx::const_entity2D_ptr, ppx::const_entity2D_ptr>> m_springs, m_rbars;
        glm::vec2 m_mpos_start{0.f};
        bool m_selecting = false;

        void draw_select_box() const;
        geo::aabb2D select_box() const;
    };
}

#endif