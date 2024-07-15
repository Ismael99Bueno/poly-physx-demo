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

    struct report
    {
        struct data
        {
            float app_timestep;
            float physics_timestep;
#ifndef KIT_PROFILE
            std::array<kit::perf::time, 4> measurements;
#else
            std::vector<kit::perf::measurement> measurements;
#endif
            std::size_t body_count;
            std::size_t collider_count;
            std::size_t joint_count;
            std::size_t collision_count;
            std::size_t total_contact_count;
            std::size_t active_contact_count;
            broad_phase2D::metrics broad_metrics;
        };

        bool recording = false;
        bool append_datetime = false;

        std::uint32_t frame_count = 0;

#ifndef KIT_PROFILE
        std::array<kit::perf::time, 4> avg_measurements;
        std::array<kit::perf::time, 4> max_measurements;
#else
        std::unordered_map<const char *, kit::perf::measurement> avg_measurements;
        std::unordered_map<const char *, kit::perf::measurement> max_measurements;
#endif

        std::vector<data> per_frame_data;
    };
    void start_recording();
    void stop_recording();
    void dump_report(const std::string &relpath) const;

    const std::string &benchmark_data_folder() const;

  private:
    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_fps();

    void record(float ts);

    template <typename TimeUnit, typename T> void dump_report(const std::string &relpath, const char *unit) const;
    template <typename TimeUnit, typename T> YAML::Node encode_report(const char *unit) const;

    void render_measurements();
    template <typename TimeUnit, typename T> void render_measurements(const char *unit, const char *format);
    template <typename TimeUnit> void render_time_plot(const std::string &unit);

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;

    time_unit m_time_unit = time_unit::MICROSECONDS;
    float m_smoothness = 0.f;
    float m_time_plot_speed = 0.02f;
    bool m_limit_fps = true;
    std::uint32_t m_fps_cap = 60;

    report m_report;
    std::string m_benchmark_data_folder = std::string(PPX_DEMO_ROOT_PATH) + "output/benchmark/data/";

    std::array<kit::perf::time, 4> m_raw_measurements;
    std::array<kit::perf::time, 4> m_max_measurements;
    std::array<kit::perf::time, 4> m_measurements;
};
} // namespace ppx::demo
