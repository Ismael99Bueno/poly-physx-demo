#pragma once

#include "ppx-demo/app/demo_layer.hpp"

#ifndef PPX_DEMO_ROOT_PATH
#define PPX_DEMO_ROOT_PATH "./"
#endif

#define PPX_DEMO_LAST_SAVE_FILENAME "last.yaml"
#define PPX_DEMO_DEFAULT_SAVE_FILENAME "default.yaml"

#define PPX_DEMO_CONFIG_FILENAME "config.yaml"
#define PPX_DEMO_DEFAULT_CONFIG_FILENAME "default-config.yaml"

namespace ppx::demo
{
class menu_bar : public demo_layer
{
  public:
    menu_bar();

    inline static constexpr const char *SAVES_DIRECTORY = PPX_DEMO_ROOT_PATH "saves/";
    inline static constexpr const char *EXAMPLES_DIRECTORY = PPX_DEMO_ROOT_PATH "examples/";

    inline static constexpr const char *LAST_SAVE_FILEPATH = PPX_DEMO_ROOT_PATH "saves/" PPX_DEMO_LAST_SAVE_FILENAME;
    inline static constexpr const char *DEFAULT_SAVE_FILEPATH =
        PPX_DEMO_ROOT_PATH "saves/" PPX_DEMO_DEFAULT_SAVE_FILENAME;

  private:
    std::string m_session;
    std::string m_serialize_path;
    std::string m_deserialize_path;

    void on_update(float ts) override;
    void on_render(float ts) override;
    bool on_event(const lynx::event2D &event) override;

    void load_new_session();
    void save();
    void load();

    void save_as();
    void load_as(const char *path);

    void render_load_prompts(const char *path);

    bool has_active_session() const;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo
