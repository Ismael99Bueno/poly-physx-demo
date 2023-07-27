#ifndef PPX_ACTIONS_PANEL_HPP
#define PPX_ACTIONS_PANEL_HPP

#include "ppx-demo/app/demo_layer.hpp"

namespace ppx::demo
{
class actions_panel : public demo_layer
{
  public:
    actions_panel();

  private:
    void on_render(float ts) override;
    bool on_event(const lynx::event &event) override;
};
} // namespace ppx::demo

#endif