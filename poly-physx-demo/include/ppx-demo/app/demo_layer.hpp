#ifndef PPX_DEMO_LAYER_HPP
#define PPX_DEMO_LAYER_HPP

#include "lynx/app/layer.hpp"

namespace ppx::demo
{
class demo_app;
class demo_layer : public lynx::layer
{
  public:
    using lynx::layer::layer;

    virtual void on_attach() override;

  protected:
    demo_app *m_app;
};
} // namespace ppx::demo

#endif