#include "templates.hpp"
#include "constants.hpp"

namespace phys_demo
{
    void entity_template::write(ini::output &out) const
    {
        out.begin_section("pos");
        pos.write(out);
        out.end_section();
        out.begin_section("vel");
        vel.write(out);
        out.end_section();
        out.write("index", index);
        out.write("id", id);
        out.write("angpos", angpos);
        out.write("angvel", angvel);
        out.write("mass", mass);
        out.write("charge", charge);
        out.write("kynematic", kynematic);

        const std::string section = "vertex";
        std::size_t index = 0;
        for (const alg::vec2 &v : vertices)
        {
            out.begin_section(section + std::to_string(index++));
            v.write(out);
            out.end_section();
        }
    }
    void entity_template::read(ini::input &in)
    {
        in.begin_section("pos");
        pos.read(in);
        in.end_section();
        in.begin_section("vel");
        vel.read(in);
        in.end_section();
        index = in.readi("index");
        id = in.readi("id");
        angpos = in.readf("angpos");
        angvel = in.readf("angvel");
        mass = in.readf("mass");
        charge = in.readf("charge");
        kynematic = (bool)in.readi("kynematic");

        vertices.clear();
        std::size_t index = 0;
        const std::string section = "vertex";
        while (true)
        {
            in.begin_section(section + std::to_string(index++));
            if (!in.contains_section())
            {
                in.end_section();
                break;
            }
            vertices.emplace_back().read(in);
            in.end_section();
        }
    }

    entity_template entity_template::from_entity(const phys::entity2D &e)
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
        tmpl.vertices = e.shape().vertices();
        tmpl.kynematic = e.kynematic();
        return tmpl;
    }

    void spring_template::write(ini::output &out) const
    {
        out.write("stiffness", stiffness);
        out.write("dampening", dampening);
        out.write("length", length);
        out.write("id1", id1);
        out.write("id2", id2);
        out.begin_section("joint1");
        joint1.write(out);
        out.end_section();
        out.begin_section("joint2");
        joint2.write(out);
        out.end_section();
        out.write("has_joints", has_joints);
    }
    void spring_template::read(ini::input &in)
    {
        stiffness = in.readf("stiffness");
        dampening = in.readf("dampening");
        length = in.readf("length");
        id1 = in.readi("id1");
        id2 = in.readi("id2");
        in.begin_section("joint1");
        joint1.read(in);
        in.end_section();
        in.begin_section("joint2");
        joint2.read(in);
        in.end_section();
        has_joints = (bool)in.readi("has_joints");
    }

    spring_template spring_template::from_spring(const phys::spring2D &sp)
    {
        spring_template tmpl;
        tmpl.stiffness = sp.stiffness();
        tmpl.dampening = sp.dampening();
        tmpl.length = sp.length();
        tmpl.id1 = sp.e1().id();
        tmpl.id2 = sp.e2().id();
        tmpl.joint1 = sp.joint1();
        tmpl.joint2 = sp.joint2();
        tmpl.has_joints = sp.has_joints();
        return tmpl;
    }

    void rigid_bar_template::write(ini::output &out) const
    {
        out.write("stiffness", stiffness);
        out.write("dampening", dampening);
        out.write("length", length);
        out.write("id1", id1);
        out.write("id2", id2);
        out.begin_section("joint1");
        joint1.write(out);
        out.end_section();
        out.begin_section("joint2");
        joint2.write(out);
        out.end_section();
        out.write("has_joints", has_joints);
    }
    void rigid_bar_template::read(ini::input &in)
    {
        stiffness = in.readf("stiffness");
        dampening = in.readf("dampening");
        length = in.readf("length");
        id1 = in.readi("id1");
        id2 = in.readi("id2");
        in.begin_section("joint1");
        joint1.read(in);
        in.end_section();
        in.begin_section("joint2");
        joint2.read(in);
        in.end_section();
        has_joints = (bool)in.readi("has_joints");
    }

    rigid_bar_template rigid_bar_template::from_bar(const phys::rigid_bar2D &rb)
    {
        rigid_bar_template tmpl;
        tmpl.stiffness = rb.stiffness();
        tmpl.dampening = rb.dampening();
        tmpl.length = rb.length();
        tmpl.id1 = rb.e1().id();
        tmpl.id2 = rb.e2().id();
        tmpl.joint1 = rb.joint1();
        tmpl.joint2 = rb.joint2();
        tmpl.has_joints = rb.has_joints();
        return tmpl;
    }
}