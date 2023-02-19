#include "predictor.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    void predictor::start()
    {
        const auto on_removal = [this](const std::size_t index)
        {
            for (auto it = m_paths.begin(); it != m_paths.end();)
                if (!it->first.try_validate())
                    it = m_paths.erase(it);
                else
                    ++it;
        };
        demo_app::get().engine().on_entity_removal(on_removal);
        m_paths.reserve(p_steps);
    }

    void predictor::update()
    {
        demo_app &papp = demo_app::get();
        phys::engine2D &eng = papp.engine();
        for (auto &[e, path] : m_paths)
        {
            path.clear();
            path.append(e->pos());
        }

        eng.checkpoint();
        for (std::size_t i = 0; i < p_steps; i++)
        {
            eng.raw_forward(p_dt);
            for (auto &[e, path] : m_paths)
                path.append(e->pos());
        }
        eng.revert();
    }

    void predictor::render()
    {
        for (auto &[e, path] : m_paths)
            demo_app::get().window().draw(path);
    }

    void predictor::predict(const phys::const_entity2D_ptr &e) { m_paths.emplace_back(e, demo_app::get().shapes()[e.index()].getFillColor()); }
    void predictor::predict_and_render(const phys::const_entity2D_ptr &e)
    {
        demo_app &papp = demo_app::get();
        phys::engine2D &eng = papp.engine();
        prm::flat_line_strip path(papp.shapes()[e.index()].getFillColor());

        eng.checkpoint();
        for (std::size_t i = 0; i < p_steps; i++)
        {
            eng.raw_forward(p_dt);
            path.append(e->pos());
        }
        papp.window().draw(path);
        eng.revert();
    }
}