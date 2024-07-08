#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx/collision/broad/broad_phase2D.hpp"
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

    struct report
    {
        bool recording = false;
        bool dump_hot_path_only = false;
        bool append_datetime = false;
        bool include_per_frame_data = true;

        std::uint32_t frame_count = 0;

        std::array<kit::perf::time, 4> avg_time_measurements;
        std::array<kit::perf::time, 4> max_time_measurements;
        std::unordered_map<std::string, detailed_metrics> avg_metrics;

        struct entry
        {
            float timestep;
            std::array<kit::perf::time, 4> time_measurements;
            std::size_t body_count;
            std::size_t collider_count;
            std::size_t joint_count;
            std::size_t collision_count;
            std::size_t total_contact_count;
            std::size_t active_contact_count;
            broad_phase2D::metrics broad_metrics;
        };

        kit::ref<std::vector<entry>> entries;
    };
    void start_recording();
    void stop_recording();

  private:
    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_fps();

    void record();
    void record_hierarchy_recursive(const kit::perf::node &node, std::size_t parent_calls = 1);

    report generate_average_report() const;

    void dump_report(const std::string &foldername) const;
    template <typename TimeUnit, typename T>
    void dump_report(const std::string &foldername, const report &rep, const char *unit) const;

    template <typename TimeUnit, typename T> YAML::Node encode_summary_report(const report &rep) const;
    template <typename TimeUnit, typename T>
    void encode_hierarchy_recursive(const report &rep, const std::string &name_hash, YAML::Node &node) const;

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

    time_unit m_time_unit = time_unit::MICROSECONDS;
    float m_smoothness = 0.f;
    float m_time_plot_speed = 0.02f;
    bool m_expand_hot_path = false;
    bool m_limit_fps = true;
    std::uint32_t m_fps_cap = 60;

    report m_report;

    std::array<kit::perf::time, 4> m_raw_time_measurements;
    std::array<kit::perf::time, 4> m_time_measurements;
    std::array<kit::perf::time, 4> m_max_time_measurements;

    std::unordered_map<std::string, kit::perf::time> m_hierarchy_max_elapsed;
    std::unordered_map<std::string, kit::perf::measurement::metrics> m_hierarchy_metrics;
    std::unordered_map<std::string, std::pair<const char *, float>> m_current_hotpath;
    std::unordered_map<std::string, std::pair<const char *, float>> m_last_hotpath;
};
} // namespace ppx::demo
