#pragma once

#include "ppx-app/app/app.hpp"
#include "ppx-demo/utility/selection_manager.hpp"
#include "ppx-demo/utility/group_manager.hpp"

namespace ppx::demo
{
class demo_app : public app
{
  public:
    demo_app();

    selection_manager selector;
    group_manager grouper;

    void add_walls();

  private:
    void on_late_start() override;
    void on_late_shutdown() override;

    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event2D &event) override;

    void remove_selected();

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo
