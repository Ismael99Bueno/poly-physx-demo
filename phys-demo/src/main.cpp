#include "demo_app.hpp"

// DBG_LOG_ALLOCATOR()

int main()
{
    PERF_BEGIN_SESSION("runtime", perf::profiler::HIERARCHY)
    phys_demo::demo_app::get().run();
    PERF_END_SESSION()
}
