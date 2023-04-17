#ifndef FOLLOWER_HPP
#define FOLLOWER_HPP

#include "entity2D_ptr.hpp"

namespace ppx_demo
{
    class follower : public ini::saveable
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

        void write(ini::output &out) const override;
        void read(ini::input &in) override;

    private:
        std::vector<ppx::const_entity2D_ptr> m_entities;
        glm::vec2 m_prev_com{0.f};

        glm::vec2 center_of_mass() const;
    };
}

#endif