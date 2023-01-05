#include "actions_panel.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"

namespace phys_demo
{
    actions_panel::actions_panel(grabber &g,
                                 selector &s,
                                 attacher &a) : m_grabber(g),
                                                m_selector(s),
                                                m_attacher(a) {}

    void actions_panel::on_attach(phys::app *papp) { m_app = papp; }

    void actions_panel::on_update()
    {
        ImGui::Begin("Actions");
        // ImGui::SetWindowFontScale(WINDOW_FONT_SCALE);
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
            render_grab_options();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Attach"))
        {
            m_action = ATTACH;
            render_attach_options();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entities"))
        {
            m_action = ENTITIES;
            render_entities_options();
            ImGui::EndTabItem();
        }
        ImGui::PopItemWidth();
        ImGui::EndTabBar();
    }

    void actions_panel::render_shapes_list()
    {
        const char *shapes[3] = {"Box", "Rectangle", "NGon"};
        ImGui::ListBox("Shapes", (int *)&m_shape_type, shapes, IM_ARRAYSIZE(shapes));
        const sf::Color color = sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f);
        switch (m_shape_type)
        {
        case BOX:
        {
            const alg::vec2 size = alg::vec2(m_templ.size, m_templ.size) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case RECT:
        {
            const alg::vec2 size = alg::vec2(m_templ.width, m_templ.height) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case NGON:
        {
            const float radius = m_templ.radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(ImVec4(color)), m_templ.sides);
            break;
        }
        default:
            break;
        }
    }

    void actions_panel::render_entity_inputs()
    {
        ImGui::DragFloat("Mass", &m_templ.mass, 0.2f, 1.f, 100.f);
        ImGui::DragFloat("Charge", &m_templ.charge, 0.2f, 1.f, 100.f);
        switch (m_shape_type)
        {
        case BOX:
            ImGui::DragFloat("Size", &m_templ.size, 0.2f, 1.f, 100.f);
            break;
        case RECT:
            ImGui::DragFloat("Width", &m_templ.width, 0.2f, 1.f, 100.f);
            ImGui::DragFloat("Height", &m_templ.height, 0.2f, 1.f, 100.f);
            break;
        case NGON:
            ImGui::DragFloat("Radius", &m_templ.radius, 0.2f, 1.f, 100.f);
            ImGui::SliderInt("Sides", (int *)&m_templ.sides, 3, 30);
            break;
        default:
            break;
        }
        ImGui::Checkbox("Dynamic", &m_templ.dynamic);
    }

    void actions_panel::render_color_picker()
    {
        if (ImGui::ColorPicker3("Entity color", m_entity_color, ImGuiColorEditFlags_NoTooltip))
            m_app->entity_color(sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", m_entity_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            m_app->entity_color(sf::Color(m_entity_color[0] * 255.f, m_entity_color[1] * 255.f, m_entity_color[2] * 255.f));
    }

    void actions_panel::render_grab_options()
    {
        const sf::Color &color = m_grabber.spring_color();
        static float sp_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        ImGui::DragFloat("Stiffness", &m_grabber.m_stiffness, 0.2f, 0.f, 500.f, "%.1f");
        ImGui::DragFloat("Dampening", &m_grabber.m_dampening, 0.2f, 0.f, 50.f, "%.2f");
        if (ImGui::ColorPicker3("Spring color", sp_color, ImGuiColorEditFlags_NoTooltip))
            m_grabber.spring_color(sf::Color(sp_color[0] * 255.f, sp_color[1] * 255.f, sp_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Spring color", sp_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            m_grabber.spring_color(sf::Color(sp_color[0] * 255.f, sp_color[1] * 255.f, sp_color[2] * 255.f));
    }

    void actions_panel::render_attach_options()
    {
        const char *attach_types[2] = {"Spring", "Rigid bar"};
        static attacher::attach_type type = m_attacher.type();
        if (ImGui::ListBox("Attach type", (int *)&type, attach_types, IM_ARRAYSIZE(attach_types)))
            m_attacher.type(type);

        switch (type)
        {
        case attacher::SPRING:
            static float sp_stf = m_attacher.sp_stiffness(),
                         sp_dmp = m_attacher.sp_dampening(),
                         sp_len = m_attacher.sp_length();
            if (ImGui::DragFloat("Stiffness", &sp_stf, 0.3f, 0.f, 50.f))
                m_attacher.sp_stiffness(sp_stf);
            if (ImGui::DragFloat("Dampening", &sp_dmp, 0.3f, 0.f, 10.f))
                m_attacher.sp_dampening(sp_dmp);
            if (ImGui::DragFloat("Length", &sp_len, 0.3f, 0.f, 100.f))
                m_attacher.sp_length(sp_len);
            break;
        case attacher::RIGID_BAR:
            static float ctr_stf = m_attacher.ctr_stiffness(),
                         ctr_dmp = m_attacher.ctr_dampening();
            if (ImGui::DragFloat("Stiffness", &ctr_stf, 0.3f, 0.f, 2000.f, "%.1f"))
                m_attacher.ctr_stiffness(ctr_stf);
            if (ImGui::DragFloat("Dampening", &ctr_dmp, 0.3f, 0.f, 100.f, "%.2f"))
                m_attacher.ctr_dampening(ctr_dmp);
            break;
        }
        render_spring_constraint_list();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        render_attach_color_pickers();
    }

    void actions_panel::render_spring_constraint_list()
    {
        if (ImGui::CollapsingHeader("Springs"))
            for (std::size_t i = 0; i < m_app->engine().springs().size(); i++)
                if (ImGui::TreeNode((void *)(intptr_t)i, "Spring %zu", i))
                {
                    phys::spring2D &sp = m_app->engine().springs()[i];
                    static float stf = sp.stiffness(), dmp = sp.dampening(), len = sp.length();

                    ImGui::Text("Stress - %f", std::get<alg::vec2>(sp.force()).norm());
                    if (ImGui::DragFloat("Stiffness", &stf, 0.3f, 0.f, 50.f))
                        sp.stiffness(stf);
                    if (ImGui::DragFloat("Dampening", &dmp, 0.3f, 0.f, 10.f))
                        sp.dampening(dmp);
                    if (ImGui::DragFloat("Length", &len, 0.3f, 0.f, 100.f))
                        sp.length(len);
                    ImGui::TreePop();
                }

        if (ImGui::CollapsingHeader("Rigid bars"))
            for (std::size_t i = 0; i < m_attacher.rbars().size(); i++)
                if (ImGui::TreeNode((void *)(intptr_t)i, "Rigid bar %zu", i))
                {
                    phys::rigid_bar2D *rb = m_attacher.rbars()[i];
                    static float stf = rb->stiffness(), dmp = rb->dampening(), len = rb->length();

                    ImGui::Text("Stress - %f", rb->value());
                    if (ImGui::DragFloat("Stiffness", &stf, 0.3f, 0.f, 2000.f))
                        rb->stiffness(stf);
                    if (ImGui::DragFloat("Dampening", &dmp, 0.3f, 0.f, 100.f))
                        rb->dampening(dmp);
                    if (ImGui::DragFloat("Length", &len, 0.3f, 0.f, 100.f))
                        rb->length(len);
                    ImGui::TreePop();
                }
    }

    void actions_panel::render_attach_color_pickers()
    {
        const sf::Color &color = m_attacher.color();
        static float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
            m_attacher.color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            m_attacher.color(sf::Color(att_color[0] * 255.f, att_color[1] * 255.f, att_color[2] * 255.f));
    }

    void actions_panel::render_entities_options() const
    {
        if (ImGui::Button("Remove all"))
            m_app->engine().clear_entities();
        ImGui::SameLine();
        if (ImGui::Button("Add borders"))
            ((demo_app *)m_app)->add_borders();
        phys::const_entity_ptr to_deselect = nullptr, to_select = nullptr;
        const phys::entity2D *to_remove = nullptr;
        if (ImGui::CollapsingHeader("Selected entities"))
        {
            if (m_selector.get().empty())
                ImGui::Text("Select entities by dragging your mouse cursor!");
            else
                for (const auto &e : m_selector.get())
                {
                    if (!render_entity_data(*e))
                        ImGui::SameLine();
                    ImGui::PushID(e.id());
                    if (ImGui::Button("Deselect"))
                        to_deselect = e;
                    ImGui::PopID();
                    ImGui::SameLine();
                    ImGui::PushID(-e.id());
                    if (ImGui::Button("Remove"))
                        to_remove = e.raw();
                    ImGui::PopID();
                }
        }
        if (ImGui::CollapsingHeader("Entities"))
        {
            if (m_app->engine().entities().empty())
                ImGui::Text("Spawn entities by clicking with your mouse while on the 'Add' tab!");
            else
                for (const phys::entity2D &e : m_app->engine().entities())
                {
                    const phys::const_entity_ptr e_ptr = {&m_app->engine().entities(), e.index()};

                    if (!render_entity_data(e, -1))
                        ImGui::SameLine();
                    ImGui::PushID(e.id());
                    if (ImGui::Button(m_selector.is_selected(e_ptr) ? "Deselect" : "Select"))
                    {
                        if (m_selector.is_selected(e_ptr))
                            to_deselect = e_ptr;
                        else
                            to_select = e_ptr;
                    }
                    ImGui::PopID();
                    ImGui::SameLine();
                    ImGui::PushID(-e.id());
                    if (ImGui::Button("Remove"))
                        to_remove = &e;
                    ImGui::PopID();
                }
        }
        if (to_select)
            m_selector.select(to_select);
        if (to_deselect)
            m_selector.deselect(to_deselect);
        if (to_remove)
            m_app->engine().remove_entity(to_remove->index());
    }

    bool actions_panel::render_entity_data(const phys::entity2D &e, std::int8_t sign) const
    {
        const bool expanded = ImGui::TreeNode((void *)(intptr_t)(e.id() * sign), "Entity %llu", e.id());
        if (expanded)
        {
            ImGui::Text("Position - x: %f, y: %f", e.pos().x, e.pos().y);
            ImGui::Text("Velocity - x: %f, y: %f", e.vel().x, e.vel().y);
            ImGui::Text("Force - x: %f, y: %f", e.force().x, e.force().y);
            ImGui::Text("Angular position - %f", e.angpos());
            ImGui::Text("Angular velocity - %f", e.angvel());
            ImGui::Text("Torque - %f", e.torque());
            ImGui::Text("Mass - %f", e.mass());
            ImGui::Text("Charge - %f", e.charge());
            ImGui::Text("Area - %f", e.shape().area());
            ImGui::Text("Inertia - %f", e.inertia());
            ImGui::Text(e.dynamic() ? "Dynamic" : "Static");
            ImGui::TreePop();
        }
        return expanded;
    }

    void actions_panel::update_template()
    {
        switch (m_shape_type)
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