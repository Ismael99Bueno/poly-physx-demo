#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define DBG_LOG(...)                                            \
    printf("-DBG- LOG in %s at line %d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__);

#define DBG_LOG_IF(cond,                                                                     \
                   ...)                                                                      \
    if (cond)                                                                                \
    {                                                                                        \
        printf("-DBG- LOG in %s at line %d with condition %s: ", __FILE__, __LINE__, #cond); \
        printf(__VA_ARGS__);                                                                 \
    }

#define DBG_EXIT_IF(cond,                                                                              \
                    ...)                                                                               \
    if (cond)                                                                                          \
    {                                                                                                  \
        fprintf(stderr, "-DBG- EXIT in %s at line %d with condition %s: ", __FILE__, __LINE__, #cond); \
        fprintf(stderr, __VA_ARGS__);                                                                  \
        exit(EXIT_FAILURE);                                                                            \
    }
#else
#define DBG_LOG(...)
#define DBG_LOG_IF(cond, \
                   ...)
#define DBG_EXIT_IF(cond, \
                    ...)
#endif

#define NDBG_LOG(...)                                            \
    printf("-NDBG- LOG in %s at line %d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__);

#define NDBG_LOG_IF(cond,                                                                     \
                    ...)                                                                      \
    if (cond)                                                                                 \
    {                                                                                         \
        printf("-NDBG- LOG in %s at line %d with condition %s: ", __FILE__, __LINE__, #cond); \
        printf(__VA_ARGS__);                                                                  \
    }

#define NDBG_EXIT_IF(cond,                                                                              \
                     ...)                                                                               \
    if (cond)                                                                                           \
    {                                                                                                   \
        fprintf(stderr, "-NDBG- EXIT in %s at line %d with condition %s: ", __FILE__, __LINE__, #cond); \
        fprintf(stderr, __VA_ARGS__);                                                                   \
        exit(EXIT_FAILURE);                                                                             \
    }

#endif