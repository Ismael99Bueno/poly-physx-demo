#include "ppxdpch.hpp"
#include "joints_tab.hpp"
#include "demo_app.hpp"
#include "globals.hpp"

namespace ppx_demo
{
joints_tab::joints_tab(attacher &attch) : m_attacher(attch)
{
}

template <typename... Args> static void dfloat_with_tooltip(const char *tooltip, Args &&...args)
{
    ImGui::DragFloat(std::forward<Args>(args)...);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("%s", tooltip);
}

void joints_tab::render() const
{
    static const char *attach_types[2] = {"Spring", "Rigid bar"};

    demo_app &papp = demo_app::get();

    ImGui::PushItemWidth(150);
    ImGui::ListBox("Attach type", (int *)&m_attacher.p_attach, attach_types, IM_ARRAYSIZE(attach_types));

    const char *ttip = "How stiff the spring will be.";
    switch (m_attacher.p_attach)
    {
    case attacher::SPRING: {
        dfloat_with_tooltip(ttip, "Stiffness", &m_attacher.p_sp_stiffness, 0.3f, 0.f, FLT_MAX);

        ttip = "How much the spring will resist to movement.";
        dfloat_with_tooltip(ttip, "Dampening", &m_attacher.p_sp_dampening, 0.3f, 0.f, FLT_MAX);

        if (!m_attacher.p_auto_length)
            ImGui::DragFloat("Length", &m_attacher.p_sp_length, 0.3f, 0.f, FLT_MAX);
        else
            ImGui::Text("Length: %f", m_attacher.p_sp_length);
        ImGui::Checkbox("Auto adjust length", &m_attacher.p_auto_length);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The length at which the spring will neither pull nor push.");

        render_springs_list();
        if (!papp.p_selector.spring_pairs().empty())
            render_selected_springs();
        break;
    }
    case attacher::RIGID_BAR: {
        ttip = "How stiff the recovery spring of the bar will be.";
        dfloat_with_tooltip(ttip, "Stiffness", &m_attacher.p_rb_stiffness, 0.3f, 0.f, FLT_MAX, "%.1f");

        ttip = "How much the recovery spring of the bar will resist to movement.";
        dfloat_with_tooltip(ttip, "Dampening", &m_attacher.p_rb_dampening, 0.3f, 0.f, FLT_MAX, "%.2f");
        render_rigid_bars_list();
        if (!papp.p_selector.rbar_pairs().empty())
            render_selected_rbars();
        break;
    }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    switch (m_attacher.p_attach)
    {
    case attacher::SPRING:
        render_spring_color_pickers();
        break;
    case attacher::RIGID_BAR:
        render_rb_color_pickers();
        break;
    }
    ImGui::PopItemWidth();
}

template <typename T> static bool render_list(const char *name, const float stress, const sf::Color &c, T &elm)
{
    const bool expanded = ImGui::TreeNode(&elm, "%s '%s' - '%s'", name, glob::generate_name(elm.e1().id()),
                                          glob::generate_name(elm.e2().id()));
    bool to_remove = false;

    if (expanded || ImGui::IsItemHovered())
    {
        outline_manager &outlmng = demo_app::get().p_outline_manager;
        outlmng.load_outline(elm.e1().index(), c, 4);
        outlmng.load_outline(elm.e2().index(), c, 4);
    }

    if (expanded)
    {
        float stf = elm.stiffness(), dmp = elm.dampening(), len = elm.length();
        ImGui::Text("Stress - %f", stress);
        if (ImGui::DragFloat("Stiffness", &stf, 0.3f, 0.f, FLT_MAX))
            elm.stiffness(stf);
        if (ImGui::DragFloat("Dampening", &dmp, 0.3f, 0.f, FLT_MAX))
            elm.dampening(dmp);
        if (ImGui::DragFloat("Length", &len, 0.3f, 0.f, FLT_MAX))
            elm.length(len);
        ImGui::TreePop();
    }
    else
        ImGui::SameLine();

    ImGui::PushID(&elm);
    to_remove = ImGui::Button("Remove");
    ImGui::PopID();

    return to_remove;
}

void joints_tab::render_springs_list() const
{
    demo_app &papp = demo_app::get();

    auto springs = papp.engine().springs();
    const std::size_t spring_count = springs.unwrap().size();
    std::size_t to_remove = spring_count;

    if (ImGui::CollapsingHeader("Springs"))
        for (std::size_t i = 0; i < spring_count; i++)
        {
            ppx::spring2D &sp = springs[i];
            if (render_list("Spring", glm::length(std::get<glm::vec2>(sp.force())), papp.springs_color(), sp))
                to_remove = i;
        }
    papp.engine().remove_spring(to_remove);
}

void joints_tab::render_rigid_bars_list() const
{
    demo_app &papp = demo_app::get();

    auto &ctrs = papp.engine().compeller().constraints();
    ppx::ref<ppx::constraint_interface2D> to_remove = nullptr;

    if (ImGui::CollapsingHeader("Rigid bars"))
        for (std::size_t i = 0; i < ctrs.size(); i++)
        {
            const auto rb = std::dynamic_pointer_cast<ppx::rigid_bar2D>(ctrs[i]);
            if (render_list("Rigid bar", rb->value(), papp.rigid_bars_color(), *rb))
                to_remove = rb;
        }
    if (to_remove)
        papp.engine().compeller().remove_constraint(to_remove);
}

static std::vector<ppx::spring2D *> from_ids(const ppx::uuid id1, const ppx::uuid id2, cvw::vector<ppx::spring2D> vec)
{
    std::vector<ppx::spring2D *> res;
    res.reserve(10);
    for (ppx::spring2D &sp : vec)
        if (sp.e1().id() == id1 && sp.e2().id() == id2)
            res.push_back(&sp);
    return res;
}

static std::vector<ppx::ref<ppx::rigid_bar2D>> from_ids(const ppx::uuid id1, const ppx::uuid id2,
                                                        const std::vector<ppx::ref<ppx::rigid_bar2D>> &vec)
{
    std::vector<ppx::ref<ppx::rigid_bar2D>> res;
    res.reserve(10);
    for (const auto &rb : vec)
        if (rb->e1().id() == id1 && rb->e2().id() == id2)
            res.push_back(rb);

    return res;
}

template <typename C, typename S>
std::tuple<float, float, float, std::size_t> compute_averages(const std::vector<selector::id_pair> &pairs,
                                                              std::vector<S> &selected, const C &container)
{
    float avg_stiffness = 0.f, avg_dampening = 0.f, avg_length = 0.f;
    std::size_t amount = 0;
    for (const auto &[id1, id2] : pairs)
    {
        const std::vector<S> matches = from_ids(id1, id2, container);
        for (const S &elm : matches)
        {
            avg_stiffness += elm->stiffness();
            avg_dampening += elm->dampening();
            avg_length += elm->length();
            amount++;
        }
        selected.insert(selected.end(), matches.begin(), matches.end());
    }
    return {avg_stiffness, avg_dampening, avg_length, amount};
}

template <typename C> static void dragfloat_stf_dmp_len(const C &container, float *stf, float *dmp, float *len)
{
    if (ImGui::DragFloat("Stiffness##Selected", stf, 0.3f, 0.f, FLT_MAX))
        for (const auto &elm : container)
            elm->stiffness(*stf);
    if (ImGui::DragFloat("Dampening##Selected", dmp, 0.3f, 0.f, FLT_MAX))
        for (const auto &elm : container)
            elm->dampening(*dmp);
    if (ImGui::DragFloat("Length##Selected", len, 0.3f, 0.f, FLT_MAX))
        for (const auto &elm : container)
            elm->length(*len);
}

void joints_tab::render_selected_springs() const
{
    demo_app &papp = demo_app::get();
    selector &slct = papp.p_selector;

    std::vector<ppx::spring2D *> selected_sps;
    selected_sps.reserve(slct.spring_pairs().size());

    auto [avg_stiffness, avg_dampening, avg_length, amount] =
        compute_averages(slct.spring_pairs(), selected_sps, papp.engine().springs());
    ImGui::Text("Selected springs: %zu", amount);

    dragfloat_stf_dmp_len(selected_sps, &avg_stiffness, &avg_dampening, &avg_length);
    if (ImGui::Button("Auto adjust length##Selected"))
        for (ppx::spring2D *sp : selected_sps)
            sp->length(glm::distance(sp->e1()->pos(), sp->e2()->pos()));

    const auto remove_springs = [&slct, &papp]() {
        const auto selected_springs = slct.spring_pairs(); // To not modify container mid iteration
        for (const auto &[id1, id2] : selected_springs)
        {
            std::vector<ppx::spring2D> sps;
            for (ppx::spring2D *sp : from_ids(id1, id2, papp.engine().springs()))
                sps.push_back(*sp);
            for (const ppx::spring2D &sp : sps)
                papp.engine().remove_spring(sp);
        }
    };

    if (ImGui::Button("Transform to rigid bars"))
    {
        for (ppx::spring2D *sp : selected_sps)
        {
            if (sp->has_anchors())
                papp.engine().compeller().add_constraint<ppx::rigid_bar2D>(sp->e1(), sp->e2(), sp->anchor1(),
                                                                           sp->anchor2(), m_attacher.p_rb_stiffness,
                                                                           m_attacher.p_rb_dampening);
            else
                papp.engine().compeller().add_constraint<ppx::rigid_bar2D>(
                    sp->e1(), sp->e2(), m_attacher.p_rb_stiffness, m_attacher.p_rb_dampening);
        }
        remove_springs();
        slct.update_selected_springs_rbars();
    }
    else if (ImGui::Button("Remove##Selected"))
        remove_springs();
}

void joints_tab::render_selected_rbars() const
{
    demo_app &papp = demo_app::get();
    selector &slct = papp.p_selector;

    std::vector<ppx::ref<ppx::rigid_bar2D>> selected_rbars, rbars;
    for (const auto &ctr : papp.engine().compeller().constraints())
        rbars.push_back(std::dynamic_pointer_cast<ppx::rigid_bar2D>(ctr));

    selected_rbars.reserve(slct.rbar_pairs().size());
    auto [avg_stiffness, avg_dampening, avg_length, amount] =
        compute_averages(slct.rbar_pairs(), selected_rbars, rbars);

    ImGui::Text("Selected rigid bars: %zu", amount);

    dragfloat_stf_dmp_len(selected_rbars, &avg_stiffness, &avg_dampening, &avg_length);
    if (ImGui::Button("Transform to springs"))
    {
        for (auto &rb : selected_rbars)
        {
            if (rb->has_anchors())
                papp.engine().add_spring(rb->e1(), rb->e2(), rb->anchor1(), rb->anchor2(), m_attacher.p_sp_stiffness,
                                         m_attacher.p_sp_dampening, m_attacher.p_sp_length);
            else
                papp.engine().add_spring(rb->e1(), rb->e2(), m_attacher.p_sp_stiffness, m_attacher.p_sp_dampening,
                                         m_attacher.p_sp_length);
            papp.engine().compeller().remove_constraint(rb);
        }
        slct.update_selected_springs_rbars();
    }
    if (ImGui::Button("Remove##Selected"))
        for (auto &rb : selected_rbars)
            papp.engine().compeller().remove_constraint(rb);
}

void joints_tab::render_spring_color_pickers() const
{
    demo_app &papp = demo_app::get();

    const sf::Color &color = papp.springs_color();
    float att_color[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
    if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
        papp.springs_color(sf::Color((sf::Uint8)(att_color[0] * 255.f), (sf::Uint8)(att_color[1] * 255.f),
                                     (sf::Uint8)(att_color[2] * 255.f)));
}

void joints_tab::render_rb_color_pickers() const
{
    demo_app &papp = demo_app::get();

    const sf::Color &color = papp.rigid_bars_color();
    float att_color[3] = {(float)color.r / 255.f, (float)color.g / 255.f, (float)color.b / 255.f};
    if (ImGui::ColorPicker3("Attach color", att_color, ImGuiColorEditFlags_NoTooltip))
        papp.rigid_bars_color(sf::Color((sf::Uint8)(att_color[0] * 255.f), (sf::Uint8)(att_color[1] * 255.f),
                                        (sf::Uint8)(att_color[2] * 255.f)));
}

} // namespace ppx_demo