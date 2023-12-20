#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "kit/profile/time.hpp"
#include "kit/profile/measurement.hpp"

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

    std::array<kit::time, 4> m_time_measurements;
    std::array<kit::time, 4> m_max_time_measurements;
    std::unordered_map<const char *, kit::time> m_max_time_hierarchy_measurements;

    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_unit_slider();
    void render_smoothness_slider();
    void render_fps() const;

    void render_summary();
    template <typename TimeUnit, typename T> void render_measurements_summary(const char *format);
    template <typename TimeUnit> void render_time_plot(const std::string &unit);

    void render_measurements_hierarchy();
    template <typename TimeUnit> void render_hierarchy_recursive(const kit::measurement &measure, const char *unit);

    kit::time evaluate_max_hierarchy_measurement(const char *name, kit::time duration);

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo
