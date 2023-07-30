#include "ppx-app/app.hpp"
#include "ppx-demo/app/demo_app.hpp"

int main()
{
    KIT_PERF_BEGIN_SESSION("ppx-demo-app", kit::instrumentor::output_format::HIERARCHY)
    ppx::demo::demo_app app;
    app.run();
    KIT_PERF_END_SESSION()
}