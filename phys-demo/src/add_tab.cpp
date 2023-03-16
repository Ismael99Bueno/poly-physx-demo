#include "add_tab.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "constants.hpp"
#include "demo_app.hpp"
#include <SFML/Graphics.hpp>

namespace phys_demo
{
    void add_tab::render() const
    {
        ImGui::PushItemWidth(200);
        render_menu_bar();
        render_shape_list();
        render_entity_inputs();
        ImGui::Spacing();
        render_color_picker();
        ImGui::PopItemWidth();
    }

    void add_tab::render_menu_bar() const
    {
        adder &addr = demo_app::get().p_adder;
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Entities"))
            {
                if (ImGui::MenuItem("Save", nullptr, nullptr, addr.has_saved_entity()))
                    addr.save_template();
                if (ImGui::MenuItem("Load", nullptr, nullptr, addr.has_saved_entity()))
                    addr.load_template();
                if (ImGui::BeginMenu("Save as..."))
                {
                    static char buffer[24] = "\0";
                    if (ImGui::InputTextWithHint("##", "Entity name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue) && buffer[0] != '\0')
                    {
                        SUBSTITUTE(buffer, ' ', '-')
                        addr.save_template(buffer);
                        buffer[0] = '\0';
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Load as..."))
                {
                    std::string selected, to_remove;
                    for (const auto &[name, templ] : addr.templates())
                    {
                        if (ImGui::Button("X"))
                            to_remove = name;
                        ImGui::SameLine();
                        if (ImGui::MenuItem(name.c_str()))
                            selected = name;
                    }
                    if (!to_remove.empty())
                        addr.erase_template(to_remove);

                    if (!selected.empty())
                        addr.load_template(selected);
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }
            ImGui::BeginMenu(addr.has_saved_entity() ? ("Current entity: " + addr.p_current_templ.name).c_str() : "No entity template. Select 'Save as...' to create one", false);
            ImGui::EndMenuBar();
        }
    }

    void add_tab::render_shape_list() const
    {
        demo_app &papp = demo_app::get();
        adder &addr = papp.p_adder;
        adder::shape_type &shape = addr.p_current_templ.shape;

        const char *shapes[4] = {"Box", "Rectangle", "NGon", "Custom"};
        if (ImGui::ListBox("Shapes", (int *)&shape, shapes, IM_ARRAYSIZE(shapes)))
            addr.update_template_vertices();

        const sf::Color &color = papp.entity_color();
        switch (shape)
        {
        case adder::BOX:
        {
            const alg::vec2 size = alg::vec2(addr.p_current_templ.size, addr.p_current_templ.size) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::RECT:
        {
            const alg::vec2 size = alg::vec2(addr.p_current_templ.width, addr.p_current_templ.height) * WORLD_TO_PIXEL,
                            pos = alg::vec2(550.f, -30.f) - 0.5f * size;
            ImGui::DrawRectFilled(sf::FloatRect(pos, size), color);
            break;
        }
        case adder::NGON:
        {
            const float radius = addr.p_current_templ.radius * WORLD_TO_PIXEL;
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddNgonFilled({pos.x + 550.f, pos.y - 30.f}, radius, ImColor(color.r, color.g, color.b), addr.p_current_templ.sides);
            break;
        }
        default:
            break;
        }
    }

    void add_tab::render_entity_inputs() const
    {
        demo_app &papp = demo_app::get();
        adder &addr = papp.p_adder;

        ImGui::DragFloat("Mass", &addr.p_current_templ.entity_templ.mass, 0.2f, 1.f, 1000.f, "%.1f");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The mass of an entity represents how hard it is to move it.");

        ImGui::DragFloat("Charge", &addr.p_current_templ.entity_templ.charge, 0.2f, 1.f, 1000.f, "%.1f");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The charge of an entity represents how strongly\nit will react to electrical interactions.");
        switch (addr.p_current_templ.shape)
        {
        case adder::BOX:
            ImGui::DragFloat("Size", &addr.p_current_templ.size, 0.2f, 1.f, 100.f, "%.1f");
            break;
        case adder::RECT:
            ImGui::DragFloat("Width", &addr.p_current_templ.width, 0.2f, 1.f, 100.f, "%.1f");
            // if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            //     ImGui::SetTooltip("Together with the shape, it is directly proportional\nto the inertia that the entity will have.");
            ImGui::DragFloat("Height", &addr.p_current_templ.height, 0.2f, 1.f, 100.f, "%.1f");
            break;
        case adder::NGON:
            ImGui::SliderInt("Sides", (int *)&addr.p_current_templ.sides, 3, 30);
            ImGui::DragFloat("Radius", &addr.p_current_templ.radius, 0.2f, 1.f, 100.f, "%.1f");
            break;
        case adder::CUSTOM:
            render_canvas();
            break;
        default:
            break;
        }

        ImGui::Checkbox("Kinematic", &addr.p_current_templ.entity_templ.kinematic);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("If unchecked, the entity will not move by any means.");

        if (papp.p_predictor.p_enabled)
            ImGui::Checkbox("Predict path", &addr.p_predict_path);
        addr.update_template_vertices();
    }

    void add_tab::render_color_picker() const
    {
        demo_app &papp = demo_app::get();

        const sf::Color &color = papp.entity_color();
        float imcolor[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};

        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip))
            papp.entity_color({(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)});
        ImGui::SameLine();
        if (ImGui::ColorPicker3("Entity color", imcolor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_PickerHueWheel))
            papp.entity_color({(sf::Uint8)(imcolor[0] * 255.f), (sf::Uint8)(imcolor[1] * 255.f), (sf::Uint8)(imcolor[2] * 255.f)});
    }

    void add_tab::render_canvas() const
    {
        static alg::vec2 scrolling;
        demo_app &papp = demo_app::get();

        std::vector<alg::vec2> &vertices = papp.p_adder.p_current_templ.entity_templ.vertices;
        geo::polygon2D poly(vertices);

        const bool is_convex = poly.is_convex();
        if (!is_convex)
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - 575.f);
            ImGui::Text("The polygon is not convex!");
            ImGui::SameLine();
            ImGui::HelpMarker("WIP Why does this matter");
        }

        const ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(),
                     canvas_sz = ImGui::GetContentRegionAvail(),
                     canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        const alg::vec2 canvas_hdim = alg::vec2(canvas_sz.x, canvas_sz.y) * 0.5f;

        // Draw border and background color
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft);
        const bool is_hovered = ImGui::IsItemHovered();

        const float scale_factor = 3.f;
        ImGuiIO &io = ImGui::GetIO();
        const alg::vec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y), // Lock scrolled origin
            pixel_mouse = (alg::vec2(io.MousePos.x, io.MousePos.y) - canvas_hdim - origin) / scale_factor,
            world_mouse = pixel_mouse * PIXEL_TO_WORLD;

        const alg::vec2 towards_poly = poly.towards_closest_edge_from(world_mouse);
        const float max_dist = 5.f;
        const bool valid_to_add = is_hovered && towards_poly.sq_norm() < max_dist;

        std::size_t to_edit = vertices.size() - 1;
        const float thres_distance = 2.f;
        float min_distance = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const float dist = vertices[i].sq_dist(world_mouse);
            if (dist < min_distance)
            {
                min_distance = dist;
                to_edit = i;
            }
        }

        const bool create_vertex = min_distance >= thres_distance;
        if (create_vertex)
        {
            if (valid_to_add && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                vertices.emplace_back(world_mouse);
            to_edit = vertices.size() - 1;
        }
        if (valid_to_add && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            vertices[to_edit] = world_mouse;

        draw_list->PushClipRect(canvas_p0, canvas_p1, true);
        const float grid_step = 64.f;
        for (float x = fmodf(scrolling.x, grid_step); x < canvas_sz.x; x += grid_step)
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        for (float y = fmodf(scrolling.y, grid_step); y < canvas_sz.y; y += grid_step)
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));

        const sf::Color &entity_col = papp.entity_color();
        const auto col = is_convex ? IM_COL32(entity_col.r, entity_col.g, entity_col.b, entity_col.a)
                                   : IM_COL32(255, 0, 0, 255);

        ImVec2 points[poly.size()];
        for (std::size_t i = 0; i < poly.size(); i++)
        {
            const alg::vec2 p1 = origin + poly[i] * scale_factor * WORLD_TO_PIXEL + canvas_hdim,
                            p2 = origin + poly[i + 1] * scale_factor * WORLD_TO_PIXEL + canvas_hdim;
            draw_list->AddLine(p1, p2, col, 4.f);
            points[i] = p1;
        }

        if (is_convex)
            draw_list->AddConvexPolyFilled(points, poly.size(), IM_COL32(entity_col.r, entity_col.g, entity_col.b, 120));
        if (valid_to_add)
        {
            const alg::vec2 center = create_vertex ? origin + (pixel_mouse + towards_poly * WORLD_TO_PIXEL) * scale_factor + canvas_hdim
                                                   : origin + vertices[to_edit] * scale_factor * WORLD_TO_PIXEL + canvas_hdim;
            draw_list->AddCircleFilled(center, 12.f, IM_COL32(207, 185, 151, 180));
        }
        draw_list->PopClipRect();
    }
}