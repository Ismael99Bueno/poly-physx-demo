#ifndef FOLLOWER_HPP
#define FOLLOWER_HPP

#include "ppx/entity2D_ptr.hpp"

namespace ppx_demo
{
    class follower
    {
    public:
        follower() = default;

        void start();
        void update();

        void follow(const ppx::const_entity2D_ptr &e);
        void unfollow(const ppx::entity2D &e);
        bool is_following(const ppx::entity2D &e) const;
        bool empty() const;
        void clear();

    private:
        std::vector<ppx::const_entity2D_ptr> m_entities;
        glm::vec2 m_prev_com{0.f};

        glm::vec2 center_of_mass() const;

        friend YAML::Emitter &operator<<(YAML::Emitter &, const follower &);
        friend struct YAML::convert<follower>;
    };

    YAML::Emitter &operator<<(YAML::Emitter &out, const follower &flw);
}

namespace YAML
{
    template <>
    struct convert<ppx_demo::follower>
    {
        static Node encode(const ppx_demo::follower &flw);
        static bool decode(const Node &node, ppx_demo::follower &flw);
    };
}

#endif