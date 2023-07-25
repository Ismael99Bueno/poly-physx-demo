#ifndef PPX_PERFORMANCE_PANEL_HPP
#define PPX_PERFORMANCE_PANEL_HPP

#include "lynx/app/layer.hpp"

namespace ppx::demo
{
class performance_panel : public lynx::layer
{
  public:
    performance_panel();

  private:
    void on_render(float ts) override;
};
} // namespace ppx::demo

#endif