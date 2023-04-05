#include "templates.hpp"

namespace ppx_demo
{
    void entity_template::write(ini::output &out) const
    {
        out.write("px", pos.x);
        out.write("py", pos.y);
        out.write("vx", vel.x);
        out.write("vy", vel.y);
        out.write("index", index);
        out.write("id", id);
        out.write("angpos", angpos);
        out.write("angvel", angvel);
        out.write("mass", mass);
        out.write("charge", charge);
        out.write("kinematic", kinematic);

        const std::string key = "vertex";
        std::size_t index = 0;
        for (const alg::vec2 &v : vertices)
        {
            out.write(key + std::to_string(index) + "x", v.x);
            out.write(key + std::to_string(index++) + "y", v.y);
        }
    }
    void entity_template::read(ini::input &in)
    {
        pos = {in.readf("px"), in.readf("py")};
        vel = {in.readf("vx"), in.readf("vy")};
        index = in.readi("index");
        id = in.readi("id");
        angpos = in.readf("angpos");
        angvel = in.readf("angvel");
        mass = in.readf("mass");
        charge = in.readf("charge");
        kinematic = (bool)in.readi("kinematic");

        vertices.clear();
        std::size_t index = 0;
        const std::string key = "vertex";
        while (true)
        {
            const std::string kx = key + std::to_string(index) + "x",
                              ky = key + std::to_string(index++) + "y";
            DBG_ASSERT((in.contains_key(kx) && in.contains_key(ky)) ||
                           (!in.contains_key(kx) && !in.contains_key(ky)),
                       "Vector key only contains a component of the vector! Weird.\n")
            if (!in.contains_key(kx) || !in.contains_key(ky)) // Just for ick reasons
                break;
            vertices.emplace_back(in.readf(kx), in.readf(ky));
        }
    }

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
        tmpl.vertices = e.shape().vertices();
        tmpl.kinematic = e.kinematic();
        return tmpl;
    }

    void spring_template::write(ini::output &out) const
    {
        out.write("stiffness", stiffness);
        out.write("dampening", dampening);
        out.write("length", length);
        out.write("id1", id1);
        out.write("id2", id2);
        out.write("joint1x", joint1.x);
        out.write("joint1y", joint1.y);
        out.write("joint2x", joint2.x);
        out.write("joint2y", joint2.y);
        out.write("has_joints", has_joints);
    }
    void spring_template::read(ini::input &in)
    {
        stiffness = in.readf("stiffness");
        dampening = in.readf("dampening");
        length = in.readf("length");
        id1 = in.readi("id1");
        id2 = in.readi("id2");
        joint1 = {in.readf("joint1x"), in.readf("joint1y")};
        joint2 = {in.readf("joint2x"), in.readf("joint2y")};
        has_joints = (bool)in.readi("has_joints");
    }

    spring_template spring_template::from_spring(const ppx::spring2D &sp)
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
        out.write("joint1x", joint1.x);
        out.write("joint1y", joint1.y);
        out.write("joint2x", joint2.x);
        out.write("joint2y", joint2.y);
        out.write("has_joints", has_joints);
    }
    void rigid_bar_template::read(ini::input &in)
    {
        stiffness = in.readf("stiffness");
        dampening = in.readf("dampening");
        length = in.readf("length");
        id1 = in.readi("id1");
        id2 = in.readi("id2");
        joint1 = {in.readf("joint1x"), in.readf("joint1y")};
        joint2 = {in.readf("joint2x"), in.readf("joint2y")};
        has_joints = (bool)in.readi("has_joints");
    }

    rigid_bar_template rigid_bar_template::from_bar(const ppx::rigid_bar2D &rb)
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