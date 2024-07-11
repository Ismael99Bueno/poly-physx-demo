#pragma once

#include "kit/serialization/yaml/codec.hpp"

namespace ppx::demo
{
class demo_app;
class scenario : public kit::yaml::codecable
{
  public:
    scenario(demo_app *app);
    virtual ~scenario() = default;

    virtual void start();
    virtual void stop();
    virtual void update(float ts) = 0;
    virtual void cleanup() = 0;

    virtual const char *name() const = 0;
    virtual std::string format() const = 0;

    virtual void on_imgui_window_render() = 0;

    bool stopped() const;             // when the scenario will no longer be included in a perf report
    virtual bool expired() const = 0; // when the scenario has finished its doings

  protected:
    demo_app *m_app;
    bool m_stopped = true;
};
} // namespace ppx::demo