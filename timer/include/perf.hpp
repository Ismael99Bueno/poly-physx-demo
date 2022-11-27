#ifndef PERF_HPP
#define PERF_HPP

#include "timer.hpp"
#include "profiling.hpp"

#ifdef PERF
#define PERF_BEGIN_SESSION(filename) perf::profiler::get().begin_session(filename);
#define PERF_END_SESSION() perf::profiler::get().end_session();
#define PERF_SCOPE(name) perf::timer tm##__LINE__(name);
#define PERF_FUNCION() perf::timer tm##__LINE__(__FUNCSIG__);
#else
#define PERF_BEGIN_SESSION(filename)
#define PERF_END_SESSION()
#define PERF_SCOPE(name)
#define PERF_FUNCION()
#endif

#endif