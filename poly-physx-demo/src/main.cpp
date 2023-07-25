#include "ppx-app/app.hpp"
#include "lynx/rendering/buffer.hpp" // This must be put into a lynx.hpp file

class my_app : public ppx::app
{
    bool on_event(const lynx::event &event) override
    {
        if (ppx::app::on_event(event))
            return true;
        switch (event.type)
        {
        case lynx::event::KEY_PRESSED:
            switch (event.key)
            {
            case lynx::input::key::F:
                if (world().size() > 0)
                    world().remove_body(world().size() - 1);
                break;
            default:
                break;
            }
        default:
            break;
        }
        return false;
    }

    void on_render(const float ts) override
    {
        ppx::app::on_render(ts);
        ImGui::Begin("Info");
        ImGui::Text("FPS: %u", (std::uint32_t)(1.f / frame_time().as<kit::time::seconds, float>()));
        ImGui::End();
    }
};

class gravity : public ppx::force2D
{
  public:
    using ppx::force2D::force2D;

  private:
    glm::vec3 force(const ppx::body2D &body) const override
    {
        return {0.f, -15.f, 0.f};
    }
};

int main()
{
    my_app app;

    ppx::body2D::specs floor;
    floor.position = {0.f, -45.f};
    floor.vertices = {{-150.f, -45.f}, {-150.f, -40.f}, {150.f, -45.f}, {150.f, -40.f}};
    floor.kinematic = false;

    gravity *grav = app.world().add_behaviour<gravity>("Gravity");
    app.world().add_body(floor);
    for (float y = 0.f; y <= 25.f; y += 8.f)
        for (float x = -35.f; x <= 55.f; x += 8.f)
        {
            const ppx::body2D::ptr body = app.world().add_body(glm::vec2(x, y));
            body->rotation(glm::length(body->position()));
            body->dispatch();
            grav->include(body);
        }
    // app.deserialize("test.yaml");

    app.run();

    // app.serialize("test.yaml");
}