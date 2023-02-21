#ifndef PREDICTOR_HPP
#define PREDICTOR_HPP

#include "flat_line_strip.hpp"
#include "entity2D_ptr.hpp"
#include <utility>

namespace phys_demo
{
    class predictor
    {
    public:
        predictor() = default;

        void start();
        void update();
        void render();

        void predict(const phys::const_entity2D_ptr &e);
        void predict_and_render(const phys::const_entity2D_ptr &e);

        float p_dt = 5e-2f;
        std::uint32_t p_steps = 100;

    private:
        std::vector<std::pair<phys::const_entity2D_ptr, prm::flat_line_strip>> m_paths;
    };
}

#endif