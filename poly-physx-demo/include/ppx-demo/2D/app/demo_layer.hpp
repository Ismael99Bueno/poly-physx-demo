#pragma once

#include "lynx/app/layer.hpp"

namespace ppx::demo
{
class demo_app;
class demo_layer : public lynx::layer2D
{
  public:
    using lynx::layer2D::layer2D;

    virtual void on_attach() override;

    bool window_toggle = true;

  protected:
    demo_app *m_app;
};
} // namespace ppx::demo
