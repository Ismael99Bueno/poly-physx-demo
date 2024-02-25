#pragma once

#include "ppx/entities/body2D.hpp"
#include "ppx/entities/collider2D.hpp"
#include "ppx/joints/spring2D.hpp"
#include "ppx/joints/distance_joint2D.hpp"

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
    void remove_group(const std::string &name);

    bool ongoing_group() const;

    YAML::Node encode() const;
    void decode(const YAML::Node &node);

  private:
    struct collider_proxy
    {
        lynx::color color;
        collider2D::specs specs;
    };
    struct body_proxy
    {
        body2D::specs specs;
        std::vector<collider_proxy> cproxies;
    };
    struct spring_proxy
    {
        std::size_t bprox_index1;
        std::size_t bprox_index2;
        lynx::color color;
        spring2D::specs specs;
    };
    struct dist_joint_proxy
    {
        std::size_t bprox_index1;
        std::size_t bprox_index2;
        lynx::color color;
        distance_joint2D::specs specs;
    };
    struct group
    {
        glm::vec2 mean_position{0.f};
        std::vector<body_proxy> bproxies;
        std::vector<spring_proxy> sproxies;
        std::vector<dist_joint_proxy> djproxies;

        YAML::Node encode(world2D &world) const;
        void decode(const YAML::Node &node, world2D &world);
    };

  public:
    const std::unordered_map<std::string, group> &groups() const;

  private:
    demo_app &m_app;
    lynx::window2D *m_window;

    group m_current_group;

    std::unordered_map<std::string, group> m_groups;

    kit::transform2D<float> m_preview_transform;
    std::vector<kit::transform2D<float>> m_bodies_preview_transforms;
    std::vector<kit::scope<lynx::shape2D>> m_group_shapes_preview;
    std::vector<spring_line> m_group_springs_preview;
    std::vector<thick_line> m_group_dist_joints_preview;

    bool m_ongoing_group = false;

    void update_preview_from_current_group();
    group create_group_from_selected();
};
} // namespace ppx::demo
