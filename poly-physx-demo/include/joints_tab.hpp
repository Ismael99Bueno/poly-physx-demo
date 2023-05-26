#ifndef joints_tab_HPP
#define joints_tab_HPP

#include "attacher.hpp"
#include "outline_manager.hpp"

namespace ppx_demo
{
class joints_tab
{
  public:
    joints_tab(attacher &attch);
    void render() const;

  private:
    attacher &m_attacher;

    void render_springs_list() const;
    void render_rigid_bars_list() const;
    void render_selected_springs() const;
    void render_selected_rbars() const;
    void render_spring_color_pickers() const;
    void render_rb_color_pickers() const;
};
} // namespace ppx_demo

#endif