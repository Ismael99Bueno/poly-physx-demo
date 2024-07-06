#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "kit/profiling/instrumentor.hpp"

namespace ppx::demo
{
class performance_panel final : public demo_layer
{
  public:
    performance_panel();

    enum class time_unit
    {
        NANOSECONDS,
        MICROSECONDS,
        MILLISECONDS,
        SECONDS
    };

    struct detailed_metrics
    {
        kit::perf::time per_call;
        kit::perf::time max_per_call;

        kit::perf::time over_calls;
        kit::perf::time max_over_calls;

        float call_load_over_parent;
        float relative_percent_over_calls;
        float total_percent_over_calls;
    };

    struct recording
    {
        bool recording = false;
        bool dump_hot_path_only = false;

        std::uint32_t frame_count = 0;

        std::array<kit::perf::time, 4> time_measurements;
        std::array<kit::perf::time, 4> max_time_measurements;
        std::unordered_map<std::string, detailed_metrics> cum_metrics;
    };

  private:
    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_fps();

    void start_recording();
    void record();
    void record_hierarchy_recursive(const kit::perf::node &node, std::size_t parent_calls = 1);
    void stop_recording();

    void render_ongoing_recording();
    template <typename TimeUnit, typename T> void render_ongoing_recording(const char *format) const;

    recording generate_average_recording(bool include_hierarchy = false) const;

    void dump_recording(const std::string &filename) const;
    template <typename TimeUnit, typename T>
    void dump_recording(const std::string &filename, const recording &record, const char *unit) const;

    template <typename TimeUnit, typename T> YAML::Node encode_summary_recording(const recording &record) const;
    template <typename TimeUnit, typename T>
    void encode_hierarchy_recursive(const recording &record, const std::string &name_hash, YAML::Node &node) const;

    void render_measurements_summary();
    template <typename TimeUnit, typename T> void render_measurements_summary(const char *format) const;
    template <typename TimeUnit> void render_time_plot(const std::string &unit);

    void render_profile_hierarchy();

    detailed_metrics generate_detailed_metrics(const kit::perf::node &node,
                                               const kit::perf::measurement::metrics &metrics,
                                               std::size_t parent_calls);
    template <typename TimeUnit>
    void render_hierarchy_recursive(const kit::perf::node &node, const char *unit, std::size_t parent_calls = 1);

    void render_performance_pie_plot(const kit::perf::node &parent, const std::unordered_set<std::string> &children);
    kit::perf::measurement::metrics smooth_out_average_metrics(const kit::perf::node &node);

    kit::perf::time take_max_hierarchy_elapsed(const char *name, kit::perf::time elapsed);

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;

    time_unit m_time_unit = time_unit::MILLISECONDS;
    float m_smoothness = 0.f;
    float m_time_plot_speed = 0.02f;
    bool m_expand_hot_path = false;
    bool m_limit_fps = true;
    std::uint32_t m_fps_cap = 60;

    recording m_record;

    std::array<kit::perf::time, 4> m_raw_time_measurements;
    std::array<kit::perf::time, 4> m_time_measurements;
    std::array<kit::perf::time, 4> m_max_time_measurements;

    std::unordered_map<std::string, kit::perf::time> m_hierarchy_max_elapsed;
    std::unordered_map<std::string, kit::perf::measurement::metrics> m_hierarchy_metrics;
    std::unordered_map<std::string, std::pair<const char *, float>> m_current_hotpath;
    std::unordered_map<std::string, std::pair<const char *, float>> m_last_hotpath;
};
} // namespace ppx::demo
