#ifndef GLOBALS_HPP
#define GLOBALS_HPP

// #define WINDOW_FONT_SCALE 1.f // REMOVE THIS Y VE A FILE MODIFICADAS UNA A UNA Y ESTANDARIZA LO DE LOS FONTS
#define PLOT_HEIGHT 0
#define DEFAULT_SIZE 5.f
#define MIN_FPS 30
#define MAX_FPS 240
#define PERF_SESSION_NAME "runtime"
#define SAVES_DIR "saves/"
#define EXAMPLES_DIR "examples/"
#define DEFAULT_SAVE "default.ini"
#define LAST_SAVE "last.ini"

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