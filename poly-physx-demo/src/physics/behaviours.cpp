#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/physics/behaviours.hpp"

namespace ppx::demo
{
glm::vec3 gravity::force(const body2D &body) const
{
    return {0.f, body.props().nondynamic.mass * magnitude, 0.f};
}
float gravity::potential_energy(const body2D &body) const
{
    return -body.props().nondynamic.mass * magnitude * body.centroid().y;
}

glm::vec3 drag::force(const body2D &body) const
{
    const glm::vec2 force = -magnitude * body.velocity;
    const float torque = -angular_magnitude * body.angular_velocity;
    return glm::vec3(force, torque);
}

glm::vec3 gravitational::force_pair(const body2D &body1, const body2D &body2) const
{
    const float cte = magnitude * body1.props().nondynamic.mass * body2.props().nondynamic.mass;
    const float dist = glm::distance(body1.centroid(), body2.centroid());

    const glm::vec2 force = cte * (body2.centroid() - body1.centroid()) / (dist * dist * dist);
    return glm::vec3(force, 0.f);
}
float gravitational::potential_energy_pair(const body2D &body1, const body2D &body2) const
{
    const float cte = magnitude * body1.props().nondynamic.mass * body2.props().nondynamic.mass;
    const float dist = glm::distance(body1.centroid(), body2.centroid());
    return -cte / dist;
}

glm::vec3 electrical::force_pair(const body2D &body1, const body2D &body2) const
{
    const float cte = magnitude * body1.charge * body2.charge;
    const float dist = glm::distance(body1.charge_centroid(), body2.charge_centroid());
    float denominator = dist;
    for (std::size_t i = 1; i < exponent; i++)
        denominator *= dist;

    const glm::vec2 force = cte * (body1.charge_centroid() - body2.charge_centroid()) / denominator;
    const glm::vec2 offset = body1.charge_centroid() - body1.centroid();
    const float torque = kit::cross2D(offset, force);
    return glm::vec3(force, torque);
}
float electrical::potential_energy_pair(const body2D &body1, const body2D &body2) const
{
    const float cte = magnitude * body1.charge * body2.charge;
    const float dist = glm::distance(body1.charge_centroid(), body2.charge_centroid());

    if (exponent == 1)
        return -cte * logf(dist);

    float denominator = dist;
    for (std::uint32_t i = 2; i < exponent; i++)
        denominator *= dist;
    return cte / denominator;
}

glm::vec3 exponential::force_pair(const body2D &body1, const body2D &body2) const
{
    const float cte = magnitude * body1.charge * body2.charge;
    const float dist = glm::distance(body1.charge_centroid(), body2.charge_centroid());

    const glm::vec2 force =
        cte * glm::normalize(body1.charge_centroid() - body2.charge_centroid()) * expf(exponent_magnitude * dist);
    const glm::vec2 offset = body1.charge_centroid() - body1.centroid();
    const float torque = kit::cross2D(offset, force);
    return glm::vec3(force, torque);
}
float exponential::potential_energy_pair(const body2D &body1, const body2D &body2) const
{
    const float cte = magnitude * body1.charge * body2.charge;
    const float dist = glm::distance(body1.charge_centroid(), body2.charge_centroid());

    return -cte * expf(exponent_magnitude * dist) / exponent_magnitude;
}

YAML::Node gravity::encode() const
{
    YAML::Node node = force2D::encode();
    node["Magnitude"] = magnitude;
    return node;
}
bool gravity::decode(const YAML::Node &node)
{
    if (!force2D::decode(node))
        return false;
    magnitude = node["Magnitude"].as<float>();
    return true;
}

YAML::Node drag::encode() const
{
    YAML::Node node = force2D::encode();
    node["Magnitude"] = magnitude;
    node["Angular magnitude"] = angular_magnitude;
    return node;
}
bool drag::decode(const YAML::Node &node)
{
    if (!force2D::decode(node))
        return false;
    magnitude = node["Magnitude"].as<float>();
    angular_magnitude = node["Angular magnitude"].as<float>();
    return true;
}

YAML::Node gravitational::encode() const
{
    YAML::Node node = interaction2D::encode();
    node["Magnitude"] = magnitude;
    return node;
}
bool gravitational::decode(const YAML::Node &node)
{
    if (!interaction2D::decode(node))
        return false;
    magnitude = node["Magnitude"].as<float>();
    return true;
}

YAML::Node electrical::encode() const
{
    YAML::Node node = interaction2D::encode();
    node["Magnitude"] = magnitude;
    node["Exponent"] = exponent;
    return node;
}
bool electrical::decode(const YAML::Node &node)
{
    if (!interaction2D::decode(node))
        return false;
    magnitude = node["Magnitude"].as<float>();
    exponent = node["Exponent"].as<std::uint32_t>();
    return true;
}

YAML::Node exponential::encode() const
{
    YAML::Node node = interaction2D::encode();
    node["Magnitude"] = magnitude;
    node["Exponent magnitude"] = exponent_magnitude;
    return node;
}
bool exponential::decode(const YAML::Node &node)
{
    if (!interaction2D::decode(node))
        return false;
    magnitude = node["Magnitude"].as<float>();
    exponent_magnitude = node["Exponent magnitude"].as<float>();
    return true;
}
} // namespace ppx::demo