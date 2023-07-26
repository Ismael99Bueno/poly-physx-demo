#ifndef PPX_PERFORMANCE_PANEL_HPP
#define PPX_PERFORMANCE_PANEL_HPP

#include "lynx/app/layer.hpp"
#include "kit/profile/time.hpp"

namespace ppx::demo
{
class performance_panel : public lynx::layer
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

    std::array<kit::time, 4> m_time_measurements;
    inline static std::array<kit::time, 4> s_max_time_measurements;

    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_unit_slider();
    void render_smoothness_slider();
    void render_fps() const;
    template <typename TimeUnit, typename T> void render_time_summary(const char *format) const;
#ifdef KIT_PERF
#else
#endif
};
} // namespace ppx::demo

#endif