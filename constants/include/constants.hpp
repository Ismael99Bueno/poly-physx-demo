#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define WIDTH 1920.f
#define HEIGHT 1280.f
#define WORLD_TO_PIXEL 10.f
#define PIXEL_TO_WORLD 0.1f
// #define WINDOW_FONT_SCALE 1.f // REMOVE THIS Y VE A FILE MODIFICADAS UNA A UNA Y ESTANDARIZA LO DE LOS FONTS
#define FONT_SIZE_PIXELS 40.f
#define NO_FPS_LIMIT 0
#define DEFAULT_FPS NO_FPS_LIMIT
#define MIN_FPS 30
#define MAX_FPS 240
#define PERF_SESSION_NAME "runtime"
#define SAVES_DIR "saves/"
#define DEFAULT_SAVE "default.ini"
#define LAST_SAVE "last.ini"

#define SUBSTITUTE(buff, from, to)        \
    for (char *c = buff; *c != '\0'; c++) \
        if (*c == from)                   \
            *c = to;

#endif