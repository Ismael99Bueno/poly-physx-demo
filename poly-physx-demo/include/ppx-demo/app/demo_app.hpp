#ifndef PPX_DEMO_APP_HPP
#define PPX_DEMO_APP_HPP

#include "ppx-app/app/app.hpp"
#include "ppx-demo/utility/selection_manager.hpp"

namespace ppx::demo
{
class demo_app : public app
{
  public:
    demo_app();

    selection_manager selector;

  private:
    void on_late_start() override;
    void on_late_shutdown() override;

    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event &event) override;

    void add_walls();
    void remove_selected_bodies();
};
} // namespace ppx::demo

#endif