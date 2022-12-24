#include "actions_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"

namespace phys_demo
{
    actions_panel::actions_panel(grabber &g, selector &s) : m_grabber(g), m_selector(s) {}

    void actions_panel::on_attach(phys::app *papp) { m_app = papp; }

    void actions_panel::on_update()
    {
        ImGui::Begin("Actions");
        ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
        render_tabs();
        ImGui::End();
    }

    void actions_panel::render_tabs()
    {
        ImGui::BeginTabBar("Hey");
        ImGui::PushItemWidth(200);
        if (ImGui::BeginTabItem("Add"))
        {
            m_action = ADD;
            render_shapes_list();
            render_entity_inputs();
            render_color_picker();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Grab"))
        {
            m_action = GRAB;
            render_grab_parameters();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Select"))
        {
            m_action = SELECT;
            render_selected_options();
            ImGui::EndTabItem();
        }
        ImGui::PopItemWidth();
        ImGui::EndTabBar();
    }

    void actions_panel::render_shapes_list()
    {
        static const char *shapes[] = {"Box", "Rectangle", "NGon"};
        ImGui::ListBox("Shapes", (int *)&m_selected_shape, shapes, IM_ARRAYSIZE(shapes));
        const sf::Color col = sf::Color(m_color[0] * 255.f, m_color[1] * 255.f, m_color[2] * 255.f);
        switch (m_selected_shape)
        {
        case BOX:
        {
            const alg::vec2 size = alg::vec2(m_templ.size(), m_templ.size()) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), col);
            break;
        }
        case RECT:
        {
            const alg::vec2 size = alg::vec2(m_templ.width(), m_templ.height()) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), col);
            break;
        }
        case NGON:
        {
            const ImVec4 col = {m_color[0], m_color[1], m_color[2], 1.f};
            const float radius = m_templ.radius() * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(col), m_templ.sides());
            break;
        }
        default:
            break;
        }
    }

    void actions_panel::render_entity_inputs()
    {
        ImGui::DragFloat("Mass", &m_templ.m_mass, 0.2f, 1.f, 100.f);
        ImGui::DragFloat("Charge", &m_templ.m_charge, 0.2f, 1.f, 100.f);
        switch (m_selected_shape)
        {
        case BOX:
            ImGui::DragFloat("Size", &m_templ.m_size, 0.2f, 1.f, 100.f);
            break;
        case RECT:
            ImGui::DragFloat("Width", &m_templ.m_width, 0.2f, 1.f, 100.f);
            ImGui::DragFloat("Height", &m_templ.m_height, 0.2f, 1.f, 100.f);
            break;
        case NGON:
            ImGui::DragFloat("Radius", &m_templ.m_radius, 0.2f, 1.f, 100.f);
            ImGui::SliderInt("Sides", (int *)&m_templ.m_sides, 3, 30);
            break;
        default:
            break;
        }
    }

    void actions_panel::render_color_picker()
    {
        if (ImGui::ColorPicker3("Entity color", m_color, ImGuiColorEditFlags_NoTooltip))
            m_app->entity_color(sf::Color(m_color[0] * 255.f, m_color[1] * 255.f, m_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", m_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            m_app->entity_color(sf::Color(m_color[0] * 255.f, m_color[1] * 255.f, m_color[2] * 255.f));
    }

    void actions_panel::render_grab_parameters()
    {
        ImGui::DragFloat("Stiffness", &m_grabber.m_stiffness, 0.2f, 50.f, 1000.f, "%.1f");
        ImGui::DragFloat("Dampening", &m_grabber.m_dampening, 0.2f, 5.f, 100.f, "%.2f");
    }

    void actions_panel::render_selected_options() const
    {
        ImGui::Text("Press DEL to remove selected entities");
        phys::const_entity_ptr to_deselect = nullptr, to_remove = nullptr;
        for (const auto &e : m_selector.get())
            if (ImGui::TreeNodeEx((void *)(intptr_t)e->id(), ImGuiTreeNodeFlags_CollapsingHeader, "Entity %llu", e->id()))
            {
                if (ImGui::IsItemHovered())
                    m_app->shapes()[e.index()].setOutlineColor(sf::Color::Magenta);
                else
                    m_app->shapes()[e.index()].setOutlineColor(sf::Color::Red);
                ImGui::Text("Position - x: %f, y: %f", e->pos().x, e->pos().y);
                ImGui::Text("Velocity - x: %f, y: %f", e->vel().x, e->vel().y);
                ImGui::Text("Force - x: %f, y: %f", e->force().x, e->force().y);
                ImGui::Text("Angular position - %f", e->angpos());
                ImGui::Text("Angular velocity - %f", e->angvel());
                ImGui::Text("Torque - %f", e->torque());
                ImGui::Text("Area - %f", e->shape().area());
                ImGui::Text("Inertia - %f", e->inertia());

                if (ImGui::Button("Deselect"))
                    to_deselect = e;
                ImGui::SameLine();
                if (ImGui::Button("Remove"))
                    to_remove = e;
            }
            else if (ImGui::IsItemHovered())
                m_app->shapes()[e.index()].setOutlineColor(sf::Color::Magenta);
            else
                m_app->shapes()[e.index()].setOutlineColor(sf::Color::Red);
        if (to_deselect)
            m_selector.deselect(to_deselect);
        if (to_remove)
            m_app->engine().remove_entity(to_remove.index());
    }

    void actions_panel::update_template()
    {
        switch (m_selected_shape)
        {
        case BOX:
            m_templ.box();
            break;
        case RECT:
            m_templ.rect();
            break;
        case NGON:
            m_templ.ngon();
        default:
            break;
        }
    }

    const entity_template &actions_panel::templ()
    {
        update_template();
        return m_templ;
    }
    actions_panel::actions actions_panel::action() const
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return NONE;
        return m_action;
    }
}