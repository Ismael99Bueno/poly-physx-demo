#pragma once

#include "lynx/drawing/shape.hpp"
#include "lynx/app/window.hpp"
#include "ppx/entities/body2D.hpp"

namespace ppx::demo
{
class demo_app;
class spawn_tab
{
  public:
    spawn_tab() = default;
    spawn_tab(demo_app *app);

    void update();
    void render();
    void render_imgui_tab();

    void begin_body_spawn();
    void end_body_spawn();
    void cancel_body_spawn();

    void increase_body_type();
    void decrease_body_type();

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    enum class shape_type
    {
        RECT,
        CIRCLE,
        NGON,
        CUSTOM,
        SIZE
    };
    struct body_template
    {
        body2D::specs specs;
        std::string name;
        float width = 5.f, height = 5.f, ngon_radius = 3.f;
        int ngon_sides = 3;
        shape_type type = shape_type::RECT;
        lynx::color color;
    };

    demo_app *m_app;
    lynx::window2D *m_window;
    kit::scope<lynx::shape2D> m_preview;

    glm::vec2 m_starting_mouse_pos{0.f};
    float m_speed_spawn_multiplier = 0.6f;
    body_template m_current_body_template{};

    bool m_spawning = false;
    bool m_bulk_spawn = false;

    body2D::ptr m_last_added = nullptr;

    std::unordered_map<std::string, body_template> m_templates;

    bool is_current_template_registered() const;

    void render_save_template_prompt();
    void render_load_template_and_removal_prompts();

    void render_body_shape_types_and_properties();
    void render_menu_bar();
    void render_and_update_custom_shape_canvas();

    void update_shape_from_type();

    static YAML::Node encode_template(const body_template &btemplate);
    static body_template decode_template(const YAML::Node &node);
};
} // namespace ppx::demo
