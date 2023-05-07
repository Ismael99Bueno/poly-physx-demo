#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "ppx/engine2D.hpp"
#include <unordered_set>
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class selector
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

        void update_selected_springs_rbars();

        const std::unordered_set<ppx::entity2D_ptr> &entities() const;
        const std::vector<std::pair<ppx::uuid, ppx::uuid>> &spring_pairs() const;
        const std::vector<std::pair<ppx::uuid, ppx::uuid>> &rbar_pairs() const;

    private:
        std::unordered_set<ppx::entity2D_ptr> m_entities;
        std::vector<std::pair<ppx::uuid, ppx::uuid>> m_springs, m_rbars;
        glm::vec2 m_mpos_start{0.f};
        bool m_selecting = false;

        void draw_select_box() const;
        geo::aabb2D select_box() const;

        friend YAML::Emitter &operator<<(YAML::Emitter &, const selector &);
        friend struct YAML::convert<selector>;
    };

    YAML::Emitter &operator<<(YAML::Emitter &out, const selector &slct);
}

namespace YAML
{
    template <>
    struct convert<ppx_demo::selector>
    {
        static Node encode(const ppx_demo::selector &slct);
        static bool decode(const Node &node, ppx_demo::selector &slct);
    };
}

#endif