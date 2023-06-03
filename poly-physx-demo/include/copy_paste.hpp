#ifndef COPY_PASTE_HPP
#define COPY_PASTE_HPP

#include "selector.hpp"
#include <unordered_map>
#include <SFML/Graphics.hpp>

namespace ppx_demo
{
    class copy_paste
    {
    public:
        copy_paste() = default;

        void render();
        void copy();
        void paste();
        void delete_copy();

        void save_group(const std::string &name);
        void load_group(const std::string &name);
        void erase_group(const std::string &name);

    private:
        struct group
        {
            template <typename T>
            struct idjoint
            {
                idjoint() = default;
                idjoint(const T &j, ppx::uuid i1, ppx::uuid i2) : id1(i1), id2(i2), joint(j) {}
                ppx::uuid id1, id2;
                T joint;
            };
            using idsp = idjoint<ppx::spring2D::specs>;
            using idrb = idjoint<ppx::rigid_bar2D::specs>;
            std::string name;
            glm::vec2 ref_pos{0.f};
            std::unordered_map<ppx::uuid, ppx::entity2D::specs> entities;
            std::vector<idsp> springs;
            std::vector<idrb> rbars;
        };

    public:
        const std::map<std::string, group> &groups() const;
        const group &current_group() const;

    private:
        group m_copy;
        bool m_has_copy = false;
        std::map<std::string, group> m_groups;

        void preview();
        void copy(group &group);

        friend struct YAML::convert<copy_paste>;
        friend YAML::Emitter &operator<<(YAML::Emitter &, const group &);
        friend struct YAML::convert<group>;
    };
    YAML::Emitter &operator<<(YAML::Emitter &out, const copy_paste &cp);
    YAML::Emitter &operator<<(YAML::Emitter &out, const copy_paste::group &g);
}

namespace YAML
{
    template <>
    struct convert<ppx_demo::copy_paste>
    {
        static Node encode(const ppx_demo::copy_paste &cp);
        static bool decode(const Node &node, ppx_demo::copy_paste &cp);
    };

    template <>
    struct convert<ppx_demo::copy_paste::group>
    {
        static Node encode(const ppx_demo::copy_paste::group &g);
        static bool decode(const Node &node, ppx_demo::copy_paste::group &g);
    };
}

#endif