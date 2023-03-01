#ifndef TRAIL_MANAGER_HPP
#define TRAIL_MANAGER_HPP

#include "thick_line_strip.hpp"
#include "entity2D_ptr.hpp"

namespace phys_demo
{
    class trail_manager : public ini::saveable
    {
    public:
        trail_manager() = default;

        void start();
        void update();
        void render() const;

        void include(const phys::const_entity2D_ptr &e);
        void exclude(const phys::entity2D &e);
        bool contains(const phys::entity2D &e) const;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        std::size_t p_steps = 150, p_length = 5;
        float p_line_thickness = 6.f;
        bool p_enabled = true, p_auto_include = true;

    private:
        std::vector<std::pair<phys::const_entity2D_ptr, prm::thick_line_strip>> m_trails;
    };
}

#endif