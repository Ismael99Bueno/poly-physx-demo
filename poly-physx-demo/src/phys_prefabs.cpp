#include "ppxdpch.hpp"
#include "phys_prefabs.hpp"
#include "demo_app.hpp"

namespace ppx_demo
{
    std::pair<glm::vec2, float> gravity::force(const ppx::entity2D &e) const
    {
        return std::make_pair(glm::vec2(0.f, e.mass() * p_mag), 0.f);
    }

    float gravity::potential_energy(const ppx::entity2D &e) const { return -e.mass() * p_mag * e.pos().y; }

    std::pair<glm::vec2, float> drag::force(const ppx::entity2D &e) const
    {
        return std::make_pair(-p_lin_mag * e.vel(), -p_ang_mag * e.angvel());
    }

    std::pair<glm::vec2, float> gravitational::force(const ppx::entity2D &e1, const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.mass() * e2.mass();
        const glm::vec2 force = cte * glm::normalize(e2.pos() - e1.pos()) / glm::distance2(e1.pos(), e2.pos());
        return std::make_pair(force, 0.f);
    }

    float gravitational::potential_energy_pair(const ppx::entity2D &e1,
                                               const ppx::entity2D &e2) const
    {
        const float cte = -p_mag * e1.mass() * e2.mass();
        return cte / glm::distance(e1.pos(), e2.pos());
    }

    std::pair<glm::vec2, float> electrical::force(const ppx::entity2D &e1, const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        float den = dist;
        for (std::uint32_t i = 0; i < p_exp; i++)
            den *= dist;
        return std::make_pair(cte * (e1.pos() - e2.pos()) / den, 0.f);
    }

    float electrical::potential_energy_pair(const ppx::entity2D &e1,
                                            const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        if (p_exp > 1)
        {
            float den = dist;
            for (std::uint32_t i = 2; i < p_exp; i++)
                den *= dist;
            return cte / den;
        }
        return -cte * logf(dist);
    }

    std::pair<glm::vec2, float> exponential::force(const ppx::entity2D &e1, const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        return std::make_pair(cte * glm::normalize(e1.pos() - e2.pos()) * expf(p_exp * dist), 0.f);
    }

    float exponential::potential_energy_pair(const ppx::entity2D &e1,
                                             const ppx::entity2D &e2) const
    {
        const float cte = p_mag * e1.charge() * e2.charge(),
                    dist = glm::distance(e1.pos(), e2.pos());
        return -cte * expf(p_exp * dist) / p_exp;
    }

    void gravity::write(YAML::Emitter &out) const
    {
        ppx::entity2D_set::write(out);
        out << YAML::Key << "Enabled" << YAML::Value << p_enabled;
        out << YAML::Key << "Magnitude" << YAML::Value << p_mag;
    }
    YAML::Node gravity::encode() const
    {
        YAML::Node node = ppx::entity2D_set::encode();
        node["Enabled"] = p_enabled;
        node["Magnitude"] = p_mag;
        return node;
    }
    bool gravity::decode(const YAML::Node &node)
    {
        if (!ppx::entity2D_set::decode(node))
            return false;

        p_enabled = node["Enabled"].as<bool>();
        p_mag = node["Magnitude"].as<float>();
        return true;
    }
    void drag::write(YAML::Emitter &out) const
    {
        ppx::entity2D_set::write(out);
        out << YAML::Key << "Enabled" << YAML::Value << p_enabled;
        out << YAML::Key << "Linear magnitude" << YAML::Value << p_lin_mag;
        out << YAML::Key << "Angular magnitude" << YAML::Value << p_ang_mag;
    }
    YAML::Node drag::encode() const
    {
        YAML::Node node = ppx::entity2D_set::encode();
        node["Enabled"] = p_enabled;
        node["Linear magnitude"] = p_lin_mag;
        node["Angular magnitude"] = p_ang_mag;
        return node;
    }
    bool drag::decode(const YAML::Node &node)
    {
        if (!ppx::entity2D_set::decode(node))
            return false;

        p_enabled = node["Enabled"].as<bool>();
        p_lin_mag = node["Linear magnitude"].as<float>();
        p_ang_mag = node["Angular magnitude"].as<float>();
        return true;
    }
    void gravitational::write(YAML::Emitter &out) const
    {
        ppx::entity2D_set::write(out);
        out << YAML::Key << "Enabled" << YAML::Value << p_enabled;
        out << YAML::Key << "Magnitude" << YAML::Value << p_mag;
    }
    YAML::Node gravitational::encode() const
    {
        YAML::Node node = ppx::entity2D_set::encode();
        node["Enabled"] = p_enabled;
        node["Magnitude"] = p_mag;
        return node;
    }
    bool gravitational::decode(const YAML::Node &node)
    {
        if (!ppx::entity2D_set::decode(node))
            return false;

        p_enabled = node["Enabled"].as<bool>();
        p_mag = node["Magnitude"].as<float>();
        return true;
    }
    void electrical::write(YAML::Emitter &out) const
    {
        ppx::entity2D_set::write(out);
        out << YAML::Key << "Enabled" << YAML::Value << p_enabled;
        out << YAML::Key << "Magnitude" << YAML::Value << p_mag;
        out << YAML::Key << "Exponent" << YAML::Value << p_exp;
    }
    YAML::Node electrical::encode() const
    {
        YAML::Node node = ppx::entity2D_set::encode();
        node["Enabled"] = p_enabled;
        node["Magnitude"] = p_mag;
        node["Exponent"] = p_exp;
        return node;
    }
    bool electrical::decode(const YAML::Node &node)
    {
        if (!ppx::entity2D_set::decode(node))
            return false;

        p_enabled = node["Enabled"].as<bool>();
        p_mag = node["Magnitude"].as<float>();
        p_exp = node["Exponent"].as<std::uint32_t>();
        return true;
    }
    void exponential::write(YAML::Emitter &out) const
    {
        ppx::entity2D_set::write(out);
        out << YAML::Key << "Enabled" << YAML::Value << p_enabled;
        out << YAML::Key << "Magnitude" << YAML::Value << p_mag;
        out << YAML::Key << "Exponent" << YAML::Value << p_exp;
    }
    YAML::Node exponential::encode() const
    {
        YAML::Node node = ppx::entity2D_set::encode();
        node["Enabled"] = p_enabled;
        node["Magnitude"] = p_mag;
        return node;
    }
    bool exponential::decode(const YAML::Node &node)
    {
        if (!ppx::entity2D_set::decode(node))
            return false;

        p_enabled = node["Enabled"].as<bool>();
        p_mag = node["Magnitude"].as<float>();
        p_exp = node["Exponent"].as<float>();
        return true;
    }
}