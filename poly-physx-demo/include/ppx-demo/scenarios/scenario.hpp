#pragma once

namespace ppx::demo
{
class demo_app;
class scenario
{
  public:
    scenario(demo_app *app);
    virtual ~scenario() = default;

    virtual void start();
    virtual void update() = 0;
    virtual void render()
    {
    }
    virtual void on_imgui_window_render()
    {
    }

    bool expired() const;

  protected:
    demo_app *m_app;
    bool m_expired = false;
};
} // namespace ppx::demo