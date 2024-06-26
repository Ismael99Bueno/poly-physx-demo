#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "kit/profiling/instrumentor.hpp"

namespace ppx::demo
{
class performance_panel : public demo_layer
{
  public:
    performance_panel();

  private:
    enum class time_unit
    {
        NANOSECONDS,
        MICROSECONDS,
        MILLISECONDS,
        SECONDS
    };

    time_unit m_time_unit = time_unit::MILLISECONDS;
    float m_smoothness = 0.f;
    float m_time_plot_speed = 0.02f;
    bool m_expand_hot_path = false;
    bool m_limit_fps = true;
    std::uint32_t m_fps_cap = 60;

    std::array<kit::perf::time, 4> m_time_measurements;
    std::array<kit::perf::time, 4> m_max_time_measurements;

    std::unordered_map<std::string, kit::perf::time> m_hierarchy_max_elapsed;
    std::unordered_map<std::string, kit::perf::measurement::metrics> m_hierarchy_metrics;
    std::unordered_map<std::string, std::pair<const char *, float>> m_current_hotpath;
    std::unordered_map<std::string, std::pair<const char *, float>> m_last_hotpath;

    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_fps();

    void render_summary();
    template <typename TimeUnit, typename T> void render_measurements_summary(const char *format);
    template <typename TimeUnit> void render_time_plot(const std::string &unit);

    void render_profile_hierarchy();

    template <typename TimeUnit>
    void render_hierarchy_recursive(const kit::perf::node &node, const char *unit, std::size_t parent_calls = 1);

    void render_performance_pie_plot(const kit::perf::node &parent, const std::unordered_set<std::string> &children);
    kit::perf::measurement::metrics smooth_out_average_metrics(const kit::perf::node &node);

    kit::perf::time take_max_hierarchy_elapsed(const char *name, kit::perf::time elapsed);

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo
