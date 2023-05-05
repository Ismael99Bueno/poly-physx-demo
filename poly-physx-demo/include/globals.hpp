#ifndef GLOBALS_HPP
#define GLOBALS_HPP

// #define WINDOW_FONT_SCALE 1.f // REMOVE THIS Y VE A FILE MODIFICADAS UNA A UNA Y ESTANDARIZA LO DE LOS FONTS
#define PLOT_HEIGHT 0
#define DEFAULT_SIZE 5.f
#define MIN_FPS 30
#define MAX_FPS 240
#define PERF_SESSION_NAME "runtime"
#ifdef ROOT_PATH
#define SAVES_DIR ROOT_PATH "saves/"
#define EXAMPLES_DIR ROOT_PATH "examples/"
#else
#define SAVES_DIR "saves/"
#define EXAMPLES_DIR "examples/"
#endif
#define DEFAULT_SAVE "default.yaml"
#define LAST_SAVE "last.yaml"

#define SUBSTITUTE(buff, from, to)        \
    for (char *c = buff; *c != '\0'; c++) \
        if (*c == from)                   \
            *c = to;

#include <cstddef>

namespace glob
{
    const char *generate_name(std::size_t id);
}

#endif