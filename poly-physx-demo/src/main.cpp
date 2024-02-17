#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_app.hpp"

int main()
{
    KIT_PERF_SESSION("ppx-demo-app")
    ppx::demo::demo_app app;
    app.run();
}