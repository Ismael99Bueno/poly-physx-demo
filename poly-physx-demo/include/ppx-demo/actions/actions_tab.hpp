#ifndef PPX_ACTIONS_TAB_HPP
#define PPX_ACTIONS_TAB_HPP

#include "lynx/app/layer.hpp"

namespace ppx::demo
{
class actions_tab : public lynx::layer
{
  public:
    actions_tab();

  private:
    void on_render(float ts) override;
};
} // namespace ppx::demo

#endif