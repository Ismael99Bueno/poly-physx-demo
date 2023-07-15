#include "ppxdpch.hpp"
#include "predictor.hpp"
#include "demo_app.hpp"
#include "globals.hpp"

namespace ppx_demo
{
void predictor::start()
{
    const auto on_addition = [this](const ppx::entity2D_ptr &e) {
        if (p_enabled && p_auto_predict)
            predict(e);
    };
    const auto on_removal = [this](const std::size_t index) {
        for (auto it = m_paths.begin(); it != m_paths.end();)
            if (!it->first.validate())
                it = m_paths.erase(it);
            else
                ++it;
    };

    demo_app &papp = demo_app::get();
    papp.engine().events().on_entity_addition += on_addition;
    papp.engine().events().on_late_entity_removal += on_removal;
    m_paths.reserve(p_steps);
}

void predictor::update()
{
    if (!p_enabled || m_paths.empty())
        return;

    KIT_PERF_PRETTY_FUNCTION()
    demo_app &papp = demo_app::get();
    ppx::engine2D &eng = papp.engine();
    for (auto &[e, path] : m_paths)
    {
        path.clear();
        path.append(e->pos() * PPX_WORLD_TO_PIXEL);
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
            path.append(e->pos() * PPX_WORLD_TO_PIXEL);
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
    KIT_PERF_PRETTY_FUNCTION()
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
    KIT_PERF_FUNCTION()
    demo_app &papp = demo_app::get();
    ppx::engine2D &eng = papp.engine();
    prm::thick_line_strip path(papp[e.index()].getFillColor(), p_line_thickness);
    path.append(e.pos() * PPX_WORLD_TO_PIXEL);

    const bool collisions = eng.collider().enabled();
    eng.checkpoint();
    if (!p_with_collisions)
        eng.collider().enabled(false);
    eng.integrator().limited_timestep(false);

    for (std::size_t i = 0; i < p_steps; i++)
    {
        eng.raw_forward(p_dt);
        path.alpha(1.f - (float)i / (float)(p_steps - 1));
        path.append(e.pos() * PPX_WORLD_TO_PIXEL);
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

YAML::Emitter &operator<<(YAML::Emitter &out, const predictor &pred)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Enabled" << YAML::Value << pred.p_enabled;
    out << YAML::Key << "Auto predict" << YAML::Value << pred.p_auto_predict;
    out << YAML::Key << "With collisions" << YAML::Value << pred.p_with_collisions;
    out << YAML::Key << "Timestep" << YAML::Value << pred.p_dt;
    out << YAML::Key << "Steps" << YAML::Value << pred.p_steps;
    out << YAML::Key << "Line thickness" << YAML::Value << pred.p_line_thickness;
    out << YAML::Key << "Paths" << YAML::Value << YAML::Flow << YAML::BeginSeq;
    for (const auto &[e, line] : pred.m_paths)
        out << e.index();
    out << YAML::EndSeq;
    out << YAML::EndMap;
    return out;
}
} // namespace ppx_demo

namespace YAML
{
Node convert<ppx_demo::predictor>::encode(const ppx_demo::predictor &pred)
{
    Node node;
    node["Enabled"] = pred.p_enabled;
    node["Auto predict"] = pred.p_auto_predict;
    node["With collisions"] = pred.p_with_collisions;
    node["Timestep"] = pred.p_dt;
    node["Steps"] = pred.p_steps;
    node["Line thickness"] = pred.p_line_thickness;
    for (const auto &[e, line] : pred.m_paths)
        node["Paths"].push_back(e.index());
    node["Paths"].SetStyle(YAML::EmitterStyle::Flow);
    return node;
}
bool convert<ppx_demo::predictor>::decode(const Node &node, ppx_demo::predictor &pred)
{
    if (!node.IsMap() || node.size() != 7)
        return false;
    pred.p_enabled = node["Enabled"].as<bool>();
    pred.p_auto_predict = node["Auto predict"].as<bool>();
    pred.p_with_collisions = node["With collisions"].as<bool>();
    pred.p_dt = node["Timestep"].as<float>();
    pred.p_steps = node["Steps"].as<std::uint32_t>();
    pred.p_line_thickness = node["Line thickness"].as<float>();
    pred.m_paths.clear();
    for (const Node &n : node["Paths"])
        pred.predict(ppx_demo::demo_app::get().engine()[n.as<std::size_t>()]);
    return true;
}
} // namespace YAML