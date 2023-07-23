#include "ppx-app/app.hpp"
#include "lynx/rendering/buffer.hpp" // This must be put into a lynx.hpp file

int main()
{
    ppx::app app;
    // app.world().add_body();
    app.deserialize("test.yaml");

    app.run();

    // app.serialize("test.yaml");
}