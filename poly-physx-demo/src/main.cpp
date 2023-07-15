#include "ppxdpch.hpp"
#include "demo_app.hpp"
#include "globals.hpp"

int main()
{
    KIT_SET_LEVEL(info)
    KIT_PERF_SET_PATH("profile-results/")
    KIT_PERF_BEGIN_SESSION(PERF_SESSION_NAME, perf::profiler::HIERARCHY)
    ppx_demo::demo_app::get().run();
    KIT_PERF_END_SESSION()
}
