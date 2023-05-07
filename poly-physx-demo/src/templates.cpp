#include "ppxdpch.hpp"
#include "templates.hpp"

namespace ppx_demo
{

    entity_template entity_template::from_entity(const ppx::entity2D &e)
    {
        entity_template tmpl;
        tmpl.pos = e.pos();
        tmpl.vel = e.vel();
        tmpl.index = e.index();
        tmpl.id = e.id();
        tmpl.angpos = e.angpos();
        tmpl.angvel = e.angvel();
        tmpl.mass = e.mass();
        tmpl.charge = e.charge();
        tmpl.kinematic = e.kinematic();

        if (const geo::polygon *poly = e.shape_if<geo::polygon>())
        {
            tmpl.type = ppx::entity2D::POLYGON;
            tmpl.shape = poly->vertices();
        }
        else
        {
            tmpl.type = ppx::entity2D::CIRCLE;
            tmpl.shape = e.shape<geo::circle>().radius();
        }
        return tmpl;
    }

    spring_template spring_template::from_spring(const ppx::spring2D &sp)
    {
        spring_template tmpl;
        tmpl.stiffness = sp.stiffness();
        tmpl.dampening = sp.dampening();
        tmpl.length = sp.length();
        tmpl.id1 = sp.e1().id();
        tmpl.id2 = sp.e2().id();
        tmpl.anchor1 = sp.anchor1();
        tmpl.anchor2 = sp.anchor2();
        tmpl.has_anchors = sp.has_anchors();
        return tmpl;
    }

    rigid_bar_template rigid_bar_template::from_bar(const ppx::rigid_bar2D &rb)
    {
        rigid_bar_template tmpl;
        tmpl.stiffness = rb.stiffness();
        tmpl.dampening = rb.dampening();
        tmpl.length = rb.length();
        tmpl.id1 = rb.e1().id();
        tmpl.id2 = rb.e2().id();
        tmpl.anchor1 = rb.anchor1();
        tmpl.anchor2 = rb.anchor2();
        tmpl.has_anchors = rb.has_anchors();
        return tmpl;
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const entity_template &tmpl)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "UUID" << YAML::Value << (std::uint64_t)tmpl.id;
        out << YAML::Key << "Index" << YAML::Value << tmpl.index;
        out << YAML::Key << "Position" << YAML::Value << tmpl.pos;
        out << YAML::Key << "Angular position" << YAML::Value << tmpl.angpos;
        out << YAML::Key << "Angular velocity" << YAML::Value << tmpl.angvel;
        out << YAML::Key << "Mass" << YAML::Value << tmpl.mass;
        out << YAML::Key << "Charge" << YAML::Value << tmpl.charge;
        out << YAML::Key << "Shape" << YAML::Value;
        if (const auto &poly = std::get_if<geo::polygon>(&tmpl.shape))
            out << *poly;
        else
            out << std::get<geo::circle>(tmpl.shape);
        out << YAML::Key << "Kinematic" << YAML::Value << tmpl.kinematic;
        out << YAML::EndMap;
        return out;
    }
    YAML::Emitter &operator<<(YAML::Emitter &out, const spring_template &tmpl)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "UUID1" << YAML::Value << (std::uint64_t)tmpl.id1;
        out << YAML::Key << "UUID2" << YAML::Value << (std::uint64_t)tmpl.id2;
        out << YAML::Key << "Stiffness" << YAML::Value << tmpl.stiffness;
        out << YAML::Key << "Dampening" << YAML::Value << tmpl.dampening;
        out << YAML::Key << "Length" << YAML::Value << tmpl.length;
        if (tmpl.has_anchors)
        {
            out << YAML::Key << "Joint1" << YAML::Value << tmpl.anchor1;
            out << YAML::Key << "Joint2" << YAML::Value << tmpl.anchor2;
        }
        out << YAML::EndMap;
        return out;
    }
    YAML::Emitter &operator<<(YAML::Emitter &out, const rigid_bar_template &tmpl)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "UUID1" << YAML::Value << (std::uint64_t)tmpl.id1;
        out << YAML::Key << "UUID2" << YAML::Value << (std::uint64_t)tmpl.id2;
        out << YAML::Key << "Stiffness" << YAML::Value << tmpl.stiffness;
        out << YAML::Key << "Dampening" << YAML::Value << tmpl.dampening;
        out << YAML::Key << "Length" << YAML::Value << tmpl.length;
        if (tmpl.has_anchors)
        {
            out << YAML::Key << "Joint1" << YAML::Value << tmpl.anchor1;
            out << YAML::Key << "Joint2" << YAML::Value << tmpl.anchor2;
        }
        out << YAML::EndMap;
        return out;
    }

}

namespace YAML
{
    Node convert<ppx_demo::entity_template>::encode(const ppx_demo::entity_template &tmpl)
    {
        Node node;
        node["UUID"] = (std::uint64_t)tmpl.id;
        node["Index"] = tmpl.index;
        node["Position"] = tmpl.pos;
        node["Angular position"] = tmpl.angpos;
        node["Angular velocity"] = tmpl.angvel;
        node["Mass"] = tmpl.mass;
        node["Charge"] = tmpl.charge;
        if (const auto &poly = std::get_if<geo::polygon>(&tmpl.shape))
            node["Shape"] = *poly;
        else
            node["Shape"] = std::get<geo::circle>(tmpl.shape);
        node["Kinematic"] = tmpl.kinematic;
        return node;
    }
    bool convert<ppx_demo::entity_template>::decode(const Node &node, ppx_demo::entity_template &tmpl)
    {
        if (!node.IsMap() || node.size() != 9)
            return false;

        tmpl.id = node["UUID"].as<std::uint64_t>();
        tmpl.index = node["Index"].as<std::size_t>();
        tmpl.pos = node["Position"].as<glm::vec2>();
        tmpl.angpos = node["Angular position"].as<float>();
        tmpl.angvel = node["Angular velocity"].as<float>();
        tmpl.mass = node["Mass"].as<float>();
        tmpl.charge = node["Charge"].as<float>();
        if (node["Shape"]["Vertices"])
            tmpl.shape = node["Shape"].as<geo::polygon>();
        else
            tmpl.shape = node["Shape"].as<geo::circle>();
        tmpl.kinematic = node["Kinematic"].as<bool>();
        return true;
    }

    Node convert<ppx_demo::spring_template>::encode(const ppx_demo::spring_template &tmpl)
    {
        Node node;
        node["UUID1"] = (std::uint64_t)tmpl.id1;
        node["UUID2"] = (std::uint64_t)tmpl.id2;
        node["Stiffness"] = tmpl.stiffness;
        node["Dampening"] = tmpl.dampening;
        node["Length"] = tmpl.length;
        if (tmpl.has_anchors)
        {
            node["Joint1"] = tmpl.anchor1;
            node["Joint2"] = tmpl.anchor2;
        }
        return node;
    }
    bool convert<ppx_demo::spring_template>::decode(const Node &node, ppx_demo::spring_template &tmpl)
    {
        if (!node.IsMap() || (node.size() != 3 && node.size() != 5))
            return false;
        tmpl.id1 = node["UUID1"].as<std::uint64_t>();
        tmpl.id2 = node["UUID2"].as<std::uint64_t>();
        tmpl.stiffness = node["Stiffness"].as<float>();
        tmpl.dampening = node["Dampening"].as<float>();
        tmpl.length = node["Length"].as<float>();
        if (node["anchor1"])
        {
            tmpl.anchor1 = node["anchor1"].as<glm::vec2>();
            tmpl.anchor2 = node["anchor2"].as<glm::vec2>();
            tmpl.has_anchors = true;
        }
        else
            tmpl.has_anchors = false;
        return true;
    }

    Node convert<ppx_demo::rigid_bar_template>::encode(const ppx_demo::rigid_bar_template &tmpl)
    {
        Node node;
        node["UUID1"] = (std::uint64_t)tmpl.id1;
        node["UUID2"] = (std::uint64_t)tmpl.id2;
        node["Stiffness"] = tmpl.stiffness;
        node["Dampening"] = tmpl.dampening;
        node["Length"] = tmpl.length;
        if (tmpl.has_anchors)
        {
            node["Joint1"] = tmpl.anchor1;
            node["Joint2"] = tmpl.anchor2;
        }
        return node;
    }
    bool convert<ppx_demo::rigid_bar_template>::decode(const Node &node, ppx_demo::rigid_bar_template &tmpl)
    {
        if (!node.IsMap() || (node.size() != 3 && node.size() != 5))
            return false;
        tmpl.id1 = node["UUID1"].as<std::uint64_t>();
        tmpl.id2 = node["UUID2"].as<std::uint64_t>();
        tmpl.stiffness = node["Stiffness"].as<float>();
        tmpl.dampening = node["Dampening"].as<float>();
        tmpl.length = node["Length"].as<float>();
        if (node["anchor1"])
        {
            tmpl.anchor1 = node["anchor1"].as<glm::vec2>();
            tmpl.anchor2 = node["anchor2"].as<glm::vec2>();
            tmpl.has_anchors = true;
        }
        else
            tmpl.has_anchors = false;
        return true;
    }
}