#include "ppxdpch.hpp"
#include "demo_app.hpp"
#include "globals.hpp"
#include "prm/flat_line_strip.hpp"
#include "prm/flat_line.hpp"

namespace ppx_demo
{
    demo_app::demo_app() : app()
    {
#ifdef ROOT_PATH
        ImGui::GetIO().IniFilename = ROOT_PATH "imgui.ini";
#endif
    }

    void demo_app::on_start()
    {
        p_phys_panel = push_layer<phys_panel>();
        p_perf_panel = push_layer<perf_panel>();
        p_engine_panel = push_layer<engine_panel>();
        p_actions_panel = push_layer<actions_panel>();
        p_menu_bar = push_layer<menu_bar>();

        p_selector.start();
        p_outline_manager.start();
        p_predictor.start();
        p_trails.start();
        p_follower.start();
        write_save(DEFAULT_SAVE);
        if (!read_save(LAST_SAVE))
            add_borders();
    }

    void demo_app::on_end() { write_save(LAST_SAVE); }

    bool demo_app::validate_session()
    {
        const bool exists = std::filesystem::exists(SAVES_DIR + m_session);
        if (!exists)
            m_session.clear();
        return exists;
    }

    void demo_app::serialize(const std::string &filepath) const
    {
        YAML::Emitter out;
        out.SetFloatPrecision(std::numeric_limits<float>::max_digits10);
        out << YAML::BeginMap;
        out << YAML::Key << "PPX Demo" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "Session" << YAML::Value << m_session;
        out << YAML::Key << "PPX Base app" << YAML::Value << *this;
        out << YAML::Key << "Selector" << YAML::Value << p_selector;
        out << YAML::Key << "Copy paste" << YAML::Value << p_copy_paste;
        out << YAML::Key << "Predictor" << YAML::Value << p_predictor;
        out << YAML::Key << "Trails" << YAML::Value << p_trails;
        out << YAML::Key << "Follower" << YAML::Value << p_follower << YAML::EndMap;
        out << YAML::EndMap;

        std::ofstream file(filepath);
        file << out.c_str();
    }
    bool demo_app::deserialize(const std::string &filepath)
    {
        if (!std::filesystem::exists(filepath))
            return false;

        YAML::Node node = YAML::LoadFile(filepath);
        if (!node["PPX Demo"])
            return false;

        const YAML::Node &demo = node["PPX Demo"];
        demo["PPX Base app"].as<ppx::app>(*((ppx::app *)this));

        m_session = demo["Session"].as<std::string>();
        p_selector = demo["Selector"].as<selector>();
        p_copy_paste = demo["Copy paste"].as<copy_paste>();
        p_predictor = demo["Predictor"].as<predictor>();
        p_trails = demo["Trails"].as<trail_manager>();
        p_follower = demo["Follower"].as<follower>();
        validate_session();
        return true;
    }

    void demo_app::write_save(const std::string &filename) const
    {
        if (!std::filesystem::exists(SAVES_DIR))
            std::filesystem::create_directory(SAVES_DIR);
        serialize(SAVES_DIR + filename);
    }
    bool demo_app::read_save(const std::string &filename) { return deserialize(SAVES_DIR + filename); }

    void demo_app::write_save() const
    {
        DBG_ASSERT(has_session(), "No current session active. Must specify a specific session name to save.\n")
        write_save(m_session);
    }
    bool demo_app::read_save()
    {
        DBG_ASSERT(has_session(), "No current session active. Must specify a specific session name to load.\n")
        return read_save(m_session);
    }

    bool demo_app::read_example(const std::string &filepath) { return deserialize(EXAMPLES_DIR + filepath); }

    void demo_app::on_update()
    {
        PERF_FUNCTION()
        p_predictor.update();
        p_trails.update();
        p_follower.update();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::N))
            {
                read_save(DEFAULT_SAVE);
                add_borders();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && has_session())
                write_save();
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::L) && has_session())
                read_save();
        }
    }

    void demo_app::on_render()
    {
        PERF_FUNCTION()
        // draw_interaction_lines();
        p_selector.render();
        p_copy_paste.render();
        p_predictor.render();
        p_trails.render();
#ifdef DEBUG
        ImGui::ShowDemoWindow();
        ImPlot::ShowDemoWindow();
#endif
    }

    void demo_app::on_late_update()
    {
        PERF_FUNCTION()
        p_outline_manager.update();
    }

    void demo_app::on_entity_draw(const ppx::entity2D_ptr &e, sf::Shape &shape)
    {
        if (p_selector.is_selecting(e))
            p_outline_manager.load_outline(e.index(), sf::Color(150, 90, 70), 3);
    }

    void demo_app::on_event(sf::Event &event)
    {
        switch (event.type)
        {
        case sf::Event::KeyPressed:
            if (ImGui::GetIO().WantCaptureKeyboard)
                break;
            switch (event.key.code)
            {
            case sf::Keyboard::Backspace:
                p_copy_paste.delete_copy();
                break;
            case sf::Keyboard::C:
                p_copy_paste.copy();
                break;
            case sf::Keyboard::V: // TODO: Que para añadir sea clic izq
                p_copy_paste.paste();
                break;
            case sf::Keyboard::F10:
                recreate_window(style() == sf::Style::Fullscreen ? sf::Style::Default : sf::Style::Fullscreen);
                break;
            case sf::Keyboard::R:
                remove_selected();
                break;
            default:
                break;
            }
            break;
        case sf::Event::MouseButtonPressed:
            p_copy_paste.delete_copy();
            break;
        default:
            break;
        }
    }

    demo_app &demo_app::get()
    {
        static demo_app app;
        return app;
    }

    void demo_app::remove_selected() // TODO: Que remove selected se llame solo con backspace una vez se cancele tb con clic der
    {
        const auto selected = p_selector.entities();
        for (ppx::const_entity2D_ptr e : selected)
            if (e.validate())
                engine().remove_entity(*e);
        p_actions_panel->cancel_add_attach();
    }

    void demo_app::add_borders()
    {
        const sf::Color prev_color = entity_color();
        entity_color(sf::Color(86, 113, 137));

        const float w = 0.5f * WIDTH * PIXEL_TO_WORLD, h = 0.5f * HEIGHT * PIXEL_TO_WORLD;
        const float thck = 20.f;

        ppx::engine2D &eng = engine();
        const ppx::entity2D_ptr &e1 = eng.add_entity(glm::vec2(-w - 0.5f * thck, 0.f)),
                                e2 = eng.add_entity(glm::vec2(w + 0.5f * thck, 0.f)),
                                e3 = eng.add_entity(glm::vec2(0.f, -h - 0.5f * thck)),
                                e4 = eng.add_entity(glm::vec2(0.f, h + 0.5f * thck));

        e1->shape(geo::polygon::rect(thck, 2.f * (h + thck)));
        e2->shape(geo::polygon::rect(thck, 2.f * (h + thck)));
        e3->shape(geo::polygon::rect(2.f * w, thck));
        e4->shape(geo::polygon::rect(2.f * w, thck));

        e1->kinematic(false);
        e2->kinematic(false);
        e3->kinematic(false);
        e4->kinematic(false);

        entity_color(prev_color);
        update_shapes();
    }

    const std::string &demo_app::session() const { return m_session; }
    void demo_app::session(const std::string &session) { m_session = session; }
    bool demo_app::has_session() const { return !m_session.empty(); }
}