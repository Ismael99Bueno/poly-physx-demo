#ifndef PREDICTOR_HPP
#define PREDICTOR_HPP

#include "thick_line_strip.hpp"
#include "entity2D_ptr.hpp"
#include <utility>

namespace phys_demo
{
    class predictor : ini::saveable
    {
    public:
        predictor() = default;

        void start();
        void update();
        void render();

        void predict(const phys::const_entity2D_ptr &e);
        void stop_predicting(const phys::entity2D &e);

        void predict_and_render(const phys::entity2D &e);
        bool is_predicting(const phys::entity2D &e) const;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_dt = 5e-2f;
        std::uint32_t p_steps = 100;
        float p_line_thickness = 6.f;
        bool p_with_collisions = false, p_enabled = true;

    private:
        std::vector<std::pair<phys::const_entity2D_ptr, prm::thick_line_strip>> m_paths;
    };
}

#endif