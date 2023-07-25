#ifndef PPX_PERFORMANCE_TAB_HPP
#define PPX_PERFORMANCE_TAB_HPP

#include "lynx/app/layer.hpp"

namespace ppx::demo
{
class performance_tab : public lynx::layer
{
  public:
    performance_tab();

  private:
    void on_render(float ts) override;
};
} // namespace ppx::demo

#endif