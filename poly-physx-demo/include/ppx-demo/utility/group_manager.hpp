#ifndef PPX_GROUP_MANAGER_HPP
#define PPX_GROUP_MANAGER_HPP

#include "ppx/body2D.hpp"
#include "ppx/joints/spring2D.hpp"
#include "ppx/joints/revolute_joint2D.hpp"

#include "lynx/drawing/shape.hpp"

#include "ppx-app/lines/spring_line.hpp"
#include "ppx-app/lines/thick_line.hpp"

namespace ppx::demo
{
class demo_app;
class group_manager
{
  public:
    group_manager(demo_app &app);

    void update();
    void render() const;

    void begin_group_from_selected();
    void paste_group();
    void cancel_group();

    void save_group_from_selected(const std::string &name);
    void load_group(const std::string &name);

    bool ongoing_group() const;

  private:
    struct body_template
    {
        body_template(const kit::uuid id, const lynx::color &color, const body2D::specs &specs)
            : id(id), color(color), specs(specs)
        {
        }
        kit::uuid id;
        lynx::color color;
        body2D::specs specs;
    };
    struct spring_template
    {
        spring_template(const std::size_t btemplate_index1, const std::size_t btemplate_index2,
                        const lynx::color &color, const spring2D::specs &specs)
            : btemplate_index1(btemplate_index1), btemplate_index2(btemplate_index2), color(color), specs(specs)
        {
        }
        std::size_t btemplate_index1;
        std::size_t btemplate_index2;
        lynx::color color;
        spring2D::specs specs;
    };
    struct revolute_template
    {
        revolute_template(const std::size_t btemplate_index1, const std::size_t btemplate_index2,
                          const lynx::color &color, const revolute_joint2D::specs &specs)
            : btemplate_index1(btemplate_index1), btemplate_index2(btemplate_index2), color(color), specs(specs)
        {
        }
        std::size_t btemplate_index1;
        std::size_t btemplate_index2;
        lynx::color color;
        revolute_joint2D::specs specs;
    };
    struct group
    {
        std::string name;
        glm::vec2 mean_position{0.f};
        std::vector<body_template> body_templates;
        std::vector<spring_template> spring_templates;
        std::vector<revolute_template> revolute_templates;
    };

    demo_app &m_app;
    lynx::window2D *m_window;

    group m_current_group;

    std::vector<group> m_groups;
    std::vector<kit::scope<lynx::shape2D>> m_group_shapes_preview;
    std::vector<spring_line> m_group_springs_preview;
    std::vector<thick_line> m_group_revolutes_preview;

    bool m_ongoing_group = false;

    void update_preview_from_current_group();
    group create_group_from_selected();
};
} // namespace ppx::demo

#endif