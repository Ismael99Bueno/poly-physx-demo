#ifndef PERF_HPP
#define PERF_HPP

#include "timer.hpp"
#include "profiler.hpp"

#ifdef PERF
#define PERF_BEGIN_SESSION(name, pexport) perf::profiler::get().begin_session(pexport, name);
#define PERF_END_SESSION() perf::profiler::get().end_session();
#define PERF_SCOPE(name) perf::timer tm##__LINE__(name);
#define PERF_FUNCTION() perf::timer tm##__LINE__(__FUNCTION__);
#define PERF_SET_MAX_FILE_MB(size) perf::profiler::get().max_mb(size);
#define PERF_SET_EXTENSION(extension) perf::profiler::get().extension(extension);
#define PERF_SET_PATH(path) perf::profiler::get().path(path);
#else
#define PERF_BEGIN_SESSION(pexport, name)
#define PERF_END_SESSION()
#define PERF_SCOPE(name)
#define PERF_FUNCTION()
#define PERF_SET_MAX_FILE_MB(size)
#define PERF_SET_EXTENSION(extension)
#define PERF_SET_PATH(path)
#endif

#endif