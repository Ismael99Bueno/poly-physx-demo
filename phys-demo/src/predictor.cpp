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
        if (!p_enabled || m_paths.empty())
            return;

        PERF_PRETTY_FUNCTION()
        demo_app &papp = demo_app::get();
        phys::engine2D &eng = papp.engine();
        for (auto &[e, path] : m_paths)
        {
            path.clear();
            path.append(e->pos() * WORLD_TO_PIXEL);
            path.thickness(p_line_thickness);
        }

        const bool collisions = eng.collider().enabled();
        eng.checkpoint();
        if (!p_with_collisions)
            eng.collider().enabled(false);

        for (std::size_t i = 0; i < p_steps; i++)
        {
            eng.raw_forward(p_dt);
            for (auto &[e, path] : m_paths)
            {
                const float alpha = 1.f - (float)i / (float)(p_steps - 1);
                path.append(e->pos() * WORLD_TO_PIXEL);
                path.alpha(alpha);
            }
        }
        if (!p_with_collisions)
            eng.collider().enabled(collisions);
        eng.revert();
    }

    void predictor::render()
    {
        if (!p_enabled)
            return;
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
        prm::thick_line_strip path(papp.shapes()[e.index()].getFillColor(), p_line_thickness);
        path.append(e.pos() * WORLD_TO_PIXEL);

        const bool collisions = eng.collider().enabled();
        eng.checkpoint();
        if (!p_with_collisions)
            eng.collider().enabled(false);

        for (std::size_t i = 0; i < p_steps; i++)
        {
            eng.raw_forward(p_dt);
            path.alpha(1.f - (float)i / (float)(p_steps - 1));
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

    void predictor::write(ini::output &out) const
    {
        out.write("enabled", p_enabled);
        out.write("timestep", p_dt);
        out.write("thickness", p_line_thickness);
        out.write("steps", p_steps);
        out.write("with_collisions", p_with_collisions);

        const std::string key = "entity";
        for (const auto &[e, path] : m_paths)
            out.write(key + std::to_string(e.index()), e.index());
    }

    void predictor::read(ini::input &in)
    {
        p_enabled = (bool)in.readi("enabled");
        p_dt = in.readf("timestep");
        p_line_thickness = in.readf("thickness");
        p_steps = in.readi("steps");
        p_with_collisions = (bool)in.readi("with_collisions");

        const std::string key = "entity";
        demo_app &papp = demo_app::get();
        for (std::size_t i = 0; i < papp.engine().size(); i++)
        {
            const phys::entity2D_ptr e = papp.engine()[i];
            if (in.contains_key(key + std::to_string(e.index())))
                predict(e);
        }
    }
}