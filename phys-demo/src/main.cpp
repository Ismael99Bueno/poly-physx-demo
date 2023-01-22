#include "demo_app.hpp"
#include "constants.hpp"

// DBG_LOG_ALLOCATOR()

int main()
{
    PERF_BEGIN_SESSION(PERF_SESSION_NAME, perf::profiler::HIERARCHY)
    phys_demo::demo_app::get().run();
    PERF_END_SESSION()
}
