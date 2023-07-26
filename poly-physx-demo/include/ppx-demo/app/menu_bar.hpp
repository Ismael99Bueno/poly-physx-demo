#ifndef PPX_MENU_BAR_HPP
#define PPX_MENU_BAR_HPP

#include "ppx-demo/app/demo_layer.hpp"

#ifndef ROOT_PATH
#define ROOT_PATH "./"
#endif

namespace ppx::demo
{
class menu_bar : public demo_layer
{
  public:
    menu_bar();

    inline static constexpr const char *SAVES_DIRECTORY = ROOT_PATH "saves/";
    inline static constexpr const char *EXAMPLES_DIRECTORY = ROOT_PATH "examples/";

    inline static constexpr const char *LAST_SAVE_FILEPATH = ROOT_PATH "saves/last.yaml";
    inline static constexpr const char *DEFAULT_SAVE_FILEPATH = ROOT_PATH "saves/default.yaml";

  private:
    void on_render(float ts) override;

    void serialize_to_saves(const std::string &filename) const;
    void serialize_to_examples(const std::string &filename) const;

    void deserialize_from_saves(const std::string &filename) const;
    void deserialize_from_examples(const std::string &filename) const;
};
} // namespace ppx::demo

#endif