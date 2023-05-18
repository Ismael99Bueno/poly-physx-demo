#include "ppxdpch.hpp"
#include "demo_app.hpp"
#include "globals.hpp"

int main()
{
    DBG_SET_LEVEL(info)
    PERF_SET_PATH("profile-results/")
    PERF_BEGIN_SESSION(PERF_SESSION_NAME, perf::profiler::HIERARCHY)
    ppx_demo::demo_app::get().run();
    PERF_END_SESSION()
}
