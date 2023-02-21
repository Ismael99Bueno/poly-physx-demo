#include "predictor.hpp"
#include "demo_app.hpp"
#include "constants.hpp"

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
        PERF_PRETTY_FUNCTION()
        if (m_paths.empty())
            return;
        demo_app &papp = demo_app::get();
        phys::engine2D &eng = papp.engine();
        for (auto &[e, path] : m_paths)
        {
            path.clear();
            path.append(e->pos() * WORLD_TO_PIXEL);
        }

        const bool collisions = eng.collider().enabled();
        eng.checkpoint();
        if (!p_with_collisions)
            eng.collider().enabled(false);

        for (std::size_t i = 0; i < p_steps; i++)
        {
            eng.raw_forward(p_dt);
            for (auto &[e, path] : m_paths)
                path.append(e->pos() * WORLD_TO_PIXEL);
        }
        if (!p_with_collisions)
            eng.collider().enabled(collisions);
        eng.revert();
    }

    void predictor::render()
    {
        PERF_PRETTY_FUNCTION()
        for (auto &[e, path] : m_paths)
            demo_app::get().window().draw(path);
    }

    void predictor::predict(const phys::const_entity2D_ptr &e) { m_paths.emplace_back(e, demo_app::get().shapes()[e.index()].getFillColor()); }
    void predictor::stop_predicting(const phys::entity2D &e)
    {
        for (auto it = m_paths.begin(); it != m_paths.end(); ++it)
            if (*(it->first) == e)
            {
                m_paths.erase(it);
                break;
            }
    }

    void predictor::predict_and_render(const phys::entity2D &e)
    {
        PERF_FUNCTION()
        demo_app &papp = demo_app::get();
        phys::engine2D &eng = papp.engine();
        prm::flat_line_strip path(papp.shapes()[e.index()].getFillColor());

        const bool collisions = eng.collider().enabled();
        eng.checkpoint();
        if (!p_with_collisions)
            eng.collider().enabled(false);

        for (std::size_t i = 0; i < p_steps; i++)
        {
            eng.raw_forward(p_dt);
            path.append(e.pos() * WORLD_TO_PIXEL);
        }
        papp.window().draw(path);
        if (!p_with_collisions)
            eng.collider().enabled(collisions);
        eng.revert();
    }

    bool predictor::is_predicting(const phys::entity2D &e) const
    {
        for (const auto &[entt, path] : m_paths)
            if (e == *entt)
                return true;
        return false;
    }
}