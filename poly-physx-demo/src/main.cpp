#include "demo_app.hpp"
#include "globals.hpp"

// DBG_LOG_ALLOCATOR()

int main()
{
    // NOTES:
    // Passing by ref alg::vec2 and entity pointers bc it seems tha (at least on my computer) the performance is better
    PERF_SET_PATH("profile-results/")
    PERF_BEGIN_SESSION(PERF_SESSION_NAME, perf::profiler::HIERARCHY)
    ppx_demo::demo_app::get().run();
    PERF_END_SESSION()
}
