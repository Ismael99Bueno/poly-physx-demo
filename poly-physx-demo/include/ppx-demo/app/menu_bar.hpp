#ifndef PPX_MENU_BAR_HPP
#define PPX_MENU_BAR_HPP

#include "ppx-demo/app/demo_layer.hpp"

namespace ppx::demo
{
class menu_bar : public demo_layer
{
  public:
    menu_bar();

  private:
    void on_render(float ts) override;
};
} // namespace ppx::demo

#endif