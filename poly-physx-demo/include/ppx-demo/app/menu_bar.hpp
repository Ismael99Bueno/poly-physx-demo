#ifndef PPX_MENU_BAR_HPP
#define PPX_MENU_BAR_HPP

#include "ppx-demo/app/demo_layer.hpp"

#ifndef PPX_DEMO_ROOT_PATH
#define PPX_DEMO_ROOT_PATH "./"
#endif

#define PPX_DEMO_LAST_SAVE_FILENAME "last.yaml"
#define PPX_DEMO_DEFAULT_SAVE_FILENAME "default.yaml"

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

    void on_render(float ts) override;

    void serialize_to_saves(const std::string &filename) const;
    void serialize_to_examples(const std::string &filename) const;

    void deserialize_from_saves(const std::string &filename) const;
    void deserialize_from_examples(const std::string &filename) const;

    void render_save_prompt();
    void render_load_and_removal_prompts();

    bool has_active_session() const;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};
} // namespace ppx::demo

#endif