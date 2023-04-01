#ifndef PREDICTOR_HPP
#define PREDICTOR_HPP

#include "thick_line_strip.hpp"
#include "entity2D_ptr.hpp"
#include <utility>

namespace ppx_demo
{
    class predictor : public ini::saveable
    {
    public:
        predictor() = default;

        void start();
        void update();
        void render() const;

        void predict(ppx::const_entity2D_ptr e);
        void stop_predicting(const ppx::entity2D &e);

        void predict_and_render(const ppx::entity2D &e);
        bool is_predicting(const ppx::entity2D &e) const;

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

        float p_dt = 3e-2f;
        std::uint32_t p_steps = 100;
        float p_line_thickness = 6.f;
        bool p_with_collisions = false, p_enabled = true, p_auto_predict = false;

    private:
        std::vector<std::pair<ppx::const_entity2D_ptr, prm::thick_line_strip>> m_paths;
    };
}

#endif