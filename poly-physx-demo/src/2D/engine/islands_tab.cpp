#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/2D/engine/islands_tab.hpp"
#include "ppx-demo/2D/app/demo_app.hpp"
#include "ppx-demo/2D/actions/actions_panel.hpp"

namespace ppx::demo
{
islands_tab::islands_tab(demo_app *app) : m_app(app)
{
    m_window = m_app->window();
}

static std::vector<glm::vec2> get_bbox_points(const aabb2D &aabb)
{
    const glm::vec2 &mm = aabb.min;
    const glm::vec2 &mx = aabb.max;
    return {glm::vec2(mm.x, mx.y), mx, glm::vec2(mx.x, mm.y), mm, glm::vec2(mm.x, mx.y)};
}

void islands_tab::update()
{
    if (!m_draw_islands || !m_app->world.islands.enabled())
        return;

    std::size_t index = 0;
    static std::size_t color_index = 0;

    for (const island2D *island : m_app->world.islands)
    {
        geo::aabb2D aabb;
        std::size_t colindex = 0;
        for (const body2D *body : island->bodies())
            for (const collider2D *collider : *body)
                if (colindex++ == 0)
                    aabb = collider->tight_bbox();
                else
                    aabb += collider->tight_bbox();
        const auto points = get_bbox_points(aabb);

        if (index < m_island_lines.size())
            for (std::size_t i = 0; i < points.size(); i++)
                m_island_lines[index][i].position = points[i];
        else
            m_island_lines.emplace_back(points, m_app->style.island_colors[color_index]);
        index++;
        color_index = (color_index + 1) % m_app->style.island_colors.size();
    }
}

void islands_tab::render()
{
    if (!m_draw_islands || !m_app->world.islands.enabled())
        return;
    const std::size_t end = glm::min(m_app->world.islands.size(), m_island_lines.size());
    for (std::size_t i = 0; i < end; i++)
        m_window->draw(m_island_lines[i]);
}

void islands_tab::render_imgui_tab()
{

    bool enabled = m_app->world.islands.enabled();
    if (ImGui::Checkbox("Enabled", &enabled))
        m_app->world.islands.enabled(enabled);

    ImGui::Checkbox("Multithreading", &m_app->world.islands.params.multithreading);
    ImGui::Checkbox("Sleep", &m_app->world.islands.params.enable_sleep);

    ImGui::SliderFloat("Lower sleep energy threshold", &m_app->world.islands.params.lower_sleep_energy_threshold,
                       0.0001f, m_app->world.islands.params.upper_sleep_energy_threshold, "%.4f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Upper sleep energy threshold", &m_app->world.islands.params.upper_sleep_energy_threshold,
                       m_app->world.islands.params.lower_sleep_energy_threshold, 1.f, "%.4f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::SliderInt("Body count mid threshold reference",
                     (int *)&m_app->world.islands.params.body_count_mid_threshold_reference, 1, 6000);

    ImGui::SliderFloat("Sleep time threshold", &m_app->world.islands.params.sleep_time_threshold, 0.0001f, 5.f, "%.4f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::Checkbox("Draw islands", &m_draw_islands);

    if (enabled && ImGui::TreeNode(&m_app, "Islands (%zu)", m_app->world.islands.size()))
    {
        for (const island2D *island : m_app->world.islands)
            if (ImGui::TreeNode(island, "%s (%zu)", kit::uuid::name_from_ptr(island).c_str(), island->bodies().size()))
            {
                ImGui::Text("Energy: %.4f", island->energy());
                ImGui::Text("Time still: %.4f", island->time_still());
                ImGui::Text("Solved positions: %s", island->solved_positions() ? "true" : "false");
                ImGui::Text("Asleep: %s", island->asleep() ? "true" : "false");
                ImGui::Text("About to sleep: %s", island->about_to_sleep() ? "true" : "false");
                ImGui::Text("Sleep energy threshold: %.4f", m_app->world.islands.sleep_energy_threshold(island));
                ImGui::Text("Actuators count: %zu", island->actuators().size());
                ImGui::Text("Constraints count: %zu", island->constraints().size());

                if (ImGui::TreeNode(island, "Bodies (%zu)", island->bodies().size()))
                {
                    for (body2D *body : island->bodies())
                        if (ImGui::TreeNode(body, "%s", kit::uuid::name_from_ptr(body).c_str()))
                        {
                            m_app->actions->entities.render_single_body_properties(body);
                            ImGui::TreePop();
                        }
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
        ImGui::TreePop();
    }
}

YAML::Node islands_tab::encode() const
{
    YAML::Node node;
    node["Draw islands"] = m_draw_islands;
    return node;
}
void islands_tab::decode(const YAML::Node &node)
{
    m_draw_islands = node["Draw islands"].as<bool>();
}

} // namespace ppx::demo