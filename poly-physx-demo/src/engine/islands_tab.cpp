#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/engine/islands_tab.hpp"
#include "ppx-demo/app/demo_app.hpp"
#include "ppx-demo/actions/actions_panel.hpp"

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
                    aabb = collider->bounding_box();
                else
                    aabb += collider->bounding_box();
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
    for (std::size_t i = 0; i < m_app->world.islands.size(); i++)
        m_window->draw(m_island_lines[i]);
}

void islands_tab::render_imgui_tab()
{

    bool enabled = m_app->world.islands.enabled();
    if (ImGui::Checkbox("Enabled", &enabled))
        m_app->world.islands.enabled(enabled);

    ImGui::Checkbox("Multithreaded", &m_app->world.islands.multithreaded);
    ImGui::Checkbox("Split", &m_app->world.islands.enable_split);
    ImGui::SliderFloat("Sleep energy threshold", &m_app->world.islands.sleep_energy_threshold, 0.0001f, 10.f, "%.4f",
                       ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Sleep time threshold", &m_app->world.islands.sleep_time_threshold, 0.0001f, 5.f, "%.4f",
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
                ImGui::Text("Wants to split: %s", island->may_split ? "true" : "false");
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
} // namespace ppx::demo