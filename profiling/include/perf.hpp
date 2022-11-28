#ifndef PERF_HPP
#define PERF_HPP

#include "timer.hpp"
#include "profiling.hpp"

#ifdef PERF
#define PERF_BEGIN_SESSION(filename, extension) perf::profiler::get().begin_session(filename, extension);
#define PERF_END_SESSION() perf::profiler::get().end_session();
#define PERF_SCOPE(name) perf::timer tm##__LINE__(name);
#define PERF_FUNCTION() perf::timer tm##__LINE__(__PRETTY_FUNCTION__);
#define PERF_SET_MAX_FILE_MB(size) perf::profiler::get().max_mb(size);
#else
#define PERF_BEGIN_SESSION(filename, extension)
#define PERF_END_SESSION()
#define PERF_SCOPE(name)
#define PERF_FUNCTION()
#define PERF_SET_MAX_FILE_MB(size)
#endif

#endif