#ifndef PPX_ACTIONS_PANEL_HPP
#define PPX_ACTIONS_PANEL_HPP

#include "lynx/app/layer.hpp"

namespace ppx::demo
{
class actions_panel : public lynx::layer
{
  public:
    actions_panel();

  private:
    void on_render(float ts) override;
};
} // namespace ppx::demo

#endif