#pragma once

#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/scenarios/scenario.hpp"
#include "kit/memory/ptr/scope.hpp"

namespace ppx::demo
{
class scenarios_panel final : public demo_layer
{
    enum class scenario_type
    {
        TUMBLER
    };

  public:
    scenarios_panel();

    const scenario *current_scenario() const;

  private:
    void on_attach() override;
    void on_update(float ts) override;
    void on_render(float ts) override;

    void render_dropdown_and_scenario_info();
    void update_scenario_type();

    kit::scope<scenario> m_current_scenario = nullptr;
    scenario_type m_sctype = scenario_type::TUMBLER;
};
} // namespace ppx::demo