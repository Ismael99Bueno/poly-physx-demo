#include "demo_app.hpp"

// DBG_LOG_ALLOCATOR()

int main()
{
    PERF_BEGIN_SESSION("runtime", perf::profiler::profile_export::SAVE_HIERARCHY)
    phys_demo::demo_app app;
    app.run();
    PERF_END_SESSION()
}
