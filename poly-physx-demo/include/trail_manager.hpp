#ifndef TRAIL_MANAGER_HPP
#define TRAIL_MANAGER_HPP

#include "thick_line_strip.hpp"
#include "entity2D_ptr.hpp"

namespace ppx_demo
{
    class trail_manager : public ini::saveable
    {
    public:
        trail_manager() = default;

        void start();
        void update();
        void render() const;

        void include(ppx::const_entity2D_ptr e);
        void exclude(const ppx::entity2D &e);
        bool contains(const ppx::entity2D &e) const;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        std::size_t p_steps = 150, p_length = 5;
        float p_line_thickness = 6.f;
        bool p_enabled = false, p_auto_include = true;

    private:
        std::vector<std::pair<ppx::const_entity2D_ptr, prm::thick_line_strip>> m_trails;
    };
}

#endif