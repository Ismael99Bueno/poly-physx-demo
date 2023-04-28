#include "predictor.hpp"
#include "demo_app.hpp"
#include "globals.hpp"

namespace ppx_demo
{
    void predictor::start()
    {
        const auto on_addition = [this](const ppx::entity2D_ptr &e)
        {
            if (p_enabled && p_auto_predict)
                predict(e);
        };
        const auto on_removal = [this](const std::size_t index)
        {
            for (auto it = m_paths.begin(); it != m_paths.end();)
                if (!it->first.try_validate())
                    it = m_paths.erase(it);
                else
                    ++it;
        };

        demo_app &papp = demo_app::get();
        papp.engine().events().on_entity_addition(on_addition);
        papp.engine().events().on_late_entity_removal(on_removal);
        m_paths.reserve(p_steps);
    }

    void predictor::update()
    {
        if (!p_enabled || m_paths.empty())
            return;

        PERF_PRETTY_FUNCTION()
        demo_app &papp = demo_app::get();
        ppx::engine2D &eng = papp.engine();
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
        eng.integrator().limited_timestep(false);

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
        eng.integrator().limited_timestep(true);
    }

    void predictor::render() const
    {
        if (!p_enabled)
            return;
        PERF_PRETTY_FUNCTION()
        for (const auto &[e, path] : m_paths)
            demo_app::get().draw(path);
    }

    void predictor::predict(const ppx::const_entity2D_ptr &e)
    {
        if (!is_predicting(*e))
            m_paths.emplace_back(e, demo_app::get()[e.index()].getFillColor());
    }
    void predictor::stop_predicting(const ppx::entity2D &e)
    {
        for (auto it = m_paths.begin(); it != m_paths.end(); ++it)
            if (*(it->first) == e)
            {
                m_paths.erase(it);
                break;
            }
    }

    void predictor::predict_and_render(const ppx::entity2D &e)
    {
        PERF_FUNCTION()
        demo_app &papp = demo_app::get();
        ppx::engine2D &eng = papp.engine();
        prm::thick_line_strip path(papp[e.index()].getFillColor(), p_line_thickness);
        path.append(e.pos() * WORLD_TO_PIXEL);

        const bool collisions = eng.collider().enabled();
        eng.checkpoint();
        if (!p_with_collisions)
            eng.collider().enabled(false);
        eng.integrator().limited_timestep(false);

        for (std::size_t i = 0; i < p_steps; i++)
        {
            eng.raw_forward(p_dt);
            path.alpha(1.f - (float)i / (float)(p_steps - 1));
            path.append(e.pos() * WORLD_TO_PIXEL);
        }
        papp.draw(path);
        if (!p_with_collisions)
            eng.collider().enabled(collisions);
        eng.revert();
        eng.integrator().limited_timestep(true);
    }

    bool predictor::is_predicting(const ppx::entity2D &e) const
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
        out.write("auto_predict", p_auto_predict);

        const std::string key = "entity";
        for (const auto &[e, path] : m_paths)
            out.write(key + std::to_string(e.index()), e.index());
    }

    void predictor::read(ini::input &in)
    {
        p_enabled = (bool)in.readi16("enabled");
        p_dt = in.readf32("timestep");
        p_line_thickness = in.readf32("thickness");
        p_steps = in.readui32("steps");
        p_with_collisions = (bool)in.readi16("with_collisions");
        p_auto_predict = (bool)in.readi16("auto_predict");
        m_paths.clear();

        const std::string key = "entity";
        demo_app &papp = demo_app::get();
        for (std::size_t i = 0; i < papp.engine().size(); i++)
        {
            const ppx::entity2D_ptr e = papp.engine()[i];
            if (in.contains_key(key + std::to_string(e.index())))
                predict(e);
        }
    }
}