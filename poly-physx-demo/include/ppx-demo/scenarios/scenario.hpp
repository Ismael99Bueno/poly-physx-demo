#pragma once

#include "kit/serialization/yaml/codec.hpp"

namespace ppx::demo
{
class demo_app;
class scenario : public kit::yaml::encodeable
{
  public:
    scenario(demo_app *app);
    virtual ~scenario() = default;

    virtual void start();
    virtual void stop();
    virtual void update(float ts) = 0;
    virtual void render()
    {
    }
    virtual void on_imgui_window_render();

    bool stopped() const;
    virtual bool expired() const
    {
        return false;
    }

  protected:
    demo_app *m_app;
    bool m_stopped = true;
    bool m_start_performance_report = true;
};
} // namespace ppx::demo