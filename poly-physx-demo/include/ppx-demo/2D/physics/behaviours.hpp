#pragma once

#include "ppx/behaviours/force.hpp"
#include "ppx/behaviours/interaction.hpp"

namespace ppx::demo
{
class gravity final : public force2D
{
  public:
    using force2D::force2D;
    float magnitude = -35.f;

    glm::vec3 force(const state2D &state) const override;
    float potential_energy(const state2D &state) const override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};

class drag final : public force2D
{
  public:
    using force2D::force2D;
    float magnitude = 5.f;
    float angular_magnitude = 1.f;

    glm::vec3 force(const state2D &state) const override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};

class gravitational final : public interaction2D
{
  public:
    using interaction2D::interaction2D;

    float magnitude = 150.f;

    glm::vec3 force_pair(const state2D &state1, const state2D &state2) const override;
    float potential_energy_pair(const state2D &state1, const state2D &state2) const override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};

class electrical final : public interaction2D
{
  public:
    using interaction2D::interaction2D;

    float magnitude = 200.f;
    std::uint32_t exponent = 2;

    glm::vec3 force_pair(const state2D &state1, const state2D &state2) const override;
    float potential_energy_pair(const state2D &state1, const state2D &state2) const override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};

class exponential final : public interaction2D
{
  public:
    using interaction2D::interaction2D;

    float magnitude = 1.f;
    float exponent_magnitude = 0.01f;

    glm::vec3 force_pair(const state2D &state1, const state2D &state2) const override;
    float potential_energy_pair(const state2D &state1, const state2D &state2) const override;

    YAML::Node encode() const override;
    bool decode(const YAML::Node &node) override;
};

} // namespace ppx::demo
