#ifndef PPX_DEMO_APP_HPP
#define PPX_DEMO_APP_HPP

#include "ppx-app/app.hpp"

namespace ppx::demo
{
class demo_app : public app
{
  public:
    demo_app();

  private:
    void on_late_start() override;
    void on_late_shutdown() override;

#ifdef DEBUG
    void on_render(float ts) override;
#endif

    void add_walls();
};
} // namespace ppx::demo

#endif