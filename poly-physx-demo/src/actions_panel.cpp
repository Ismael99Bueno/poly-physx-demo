#include "ppxdpch.hpp"
#include "actions_panel.hpp"
#include "globals.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    actions_panel::actions_panel() : ppx::layer("actions_panel") {}

    void actions_panel::on_start() { m_grabber.start(); }
    void actions_panel::on_update()
    {
        update_current_action();
        m_grabber.update();
        m_attacher.update(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
    }

    void actions_panel::on_render()
    {
        m_adder.render();
        m_grabber.render();
        m_attacher.render(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));

        if (!p_visible)
            return;
        const bool enabled = ImGui::Begin("Actions", &p_visible, ImGuiWindowFlags_MenuBar);
        if (enabled)
            ImGui::HelpMarker("The Actions panel allows you to make high level changes to the simulation, such as adding, removing and grabbing entities, modify their properties or attach springs and rigid bars to them.");

        if (enabled)
        {
            render_current_action();
            render_tabs();
        }

        ImGui::End();
    }

    void actions_panel::update_current_action()
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
            m_action = GRAB;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
            m_action = ATTACH;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
            m_action = SELECT;
        else
            m_action = ADD;
    }

    void actions_panel::render_current_action()
    {
        ImGui::Text("Actions (hold keys): LMB | F + LMB | E + LMB | LShift + LMB | LControl + LMB");
        switch (m_action)
        {
        case ADD:
            ImGui::Text("Current action - Add entity");
            break;
        case GRAB:
            ImGui::Text("Current action - Grab");
            break;
        case ATTACH:
            ImGui::Text("Current action - Attach");
            break;
        case SELECT:
            ImGui::Text("Current action - Select");
            break;
        case NONE:
            ImGui::Text("No action");
            break;
        }
    }

    template <typename T>
    static void render_tab(const char *name, const char *tooltip, T &tab)
    {
        const bool expanded = ImGui::BeginTabItem(name);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("%s", tooltip);
        if (expanded)
        {
            tab.render();
            ImGui::EndTabItem();
        }
    }

    void actions_panel::render_tabs() const
    {
        ImGui::BeginTabBar("Actions tab bar");
        render_tab("Add", "Add entities", m_add_tab);
        render_tab("Grab", "Grab entities", m_grab_tab);
        render_tab("Attach", "Attach entities", m_attach_tab);
        render_tab("Entities", "Entities overview", m_entities_tab);
        ImGui::EndTabBar();
    }

    void actions_panel::on_event(sf::Event &event)
    {
        demo_app &papp = demo_app::get();
        switch (event.type)
        {
        case sf::Event::MouseButtonPressed:
            switch (event.mouseButton.button)
            {
            case sf::Mouse::Left:
                switch (action())
                {
                case actions_panel::ADD:
                    m_adder.setup();
                    break;
                case actions_panel::GRAB:
                    m_grabber.try_grab_entity();
                    break;
                case actions_panel::ATTACH:
                    if (m_attacher.has_first())
                        m_attacher.try_attach_second(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                    else
                        m_attacher.try_attach_first(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                    break;
                case actions_panel::SELECT:
                    papp.p_selector.begin_select();
                    break;
                default:
                    break;
                }
                break;
            case sf::Mouse::Right:
                cancel_add_attach();
                break;
            default:
                break;
            }
            break;
        case sf::Event::MouseButtonReleased:
            switch (action())
            {
            case actions_panel::ADD:
                m_adder.add();
                break;
            case actions_panel::GRAB:
                m_grabber.null();
                break;
            case actions_panel::SELECT:
                papp.p_selector.end_select(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                break;
            default:
                break;
            }
            break;
        case sf::Event::KeyPressed:
            if (ImGui::GetIO().WantCaptureKeyboard)
                break;
            switch (event.key.code)
            {
            case sf::Keyboard::BackSpace:
                cancel_add_attach();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    void actions_panel::cancel_add_attach()
    {
        m_attacher.cancel();
        m_adder.cancel();
    }

    actions_panel::actions actions_panel::action() const
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return NONE;
        return m_action;
    }

    void actions_panel::write(YAML::Emitter &out) const
    {
        layer::write(out);
        out << YAML::Key << "Adder" << YAML::Value << m_adder;
        out << YAML::Key << "Grabber" << YAML::Value << m_grabber;
        out << YAML::Key << "Attacher" << YAML::Value << m_attacher;
    }
    YAML::Node actions_panel::encode() const
    {
        YAML::Node node = layer::encode();
        node["Adder"] = m_adder;
        node["Grabber"] = m_grabber;
        node["Attacher"] = m_attacher;
        return node;
    }
    bool actions_panel::decode(const YAML::Node &node)
    {
        if (!layer::decode(node))
            return false;
        node["Adder"].as<adder>(m_adder);
        node["Grabber"].as<grabber>(m_grabber);
        node["Attacher"].as<attacher>(m_attacher);
        return true;
    }
}