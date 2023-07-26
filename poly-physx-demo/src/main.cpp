#include "ppx-app/app.hpp"
#include "lynx/rendering/buffer.hpp" // This must be put into a lynx.hpp file
#include "ppx-demo/app/demo_app.hpp"

int main()
{
    KIT_PERF_BEGIN_SESSION("ppx-demo-app",
                           kit::instrumentor::output_format::HIERARCHY | kit::instrumentor::output_format::JSON_TRACE)
    ppx::demo::demo_app app;
    app.run();
    KIT_PERF_END_SESSION()
}