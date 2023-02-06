#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define DBG_LOG(...)                                            \
    printf("-DBG- LOG in %s at line %d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__);

#define DBG_LOG_IF(cond,                                                                                 \
                   ...)                                                                                  \
    if (cond)                                                                                            \
    {                                                                                                    \
        printf("-DBG- CONDITIONAL LOG in %s at line %d with condition %s: ", __FILE__, __LINE__, #cond); \
        printf(__VA_ARGS__);                                                                             \
    }

#define DBG_ASSERT(cond,                                                                                 \
                   ...)                                                                                  \
    if (!(cond))                                                                                         \
    {                                                                                                    \
        fprintf(stderr, "-DBG- ASSERT in %s at line %d with condition %s: ", __FILE__, __LINE__, #cond); \
        fprintf(stderr, __VA_ARGS__);                                                                    \
        exit(EXIT_FAILURE);                                                                              \
    }

#define DBG_LOG_ALLOCATOR()                                 \
    void *operator new(std::size_t size)                    \
    {                                                       \
        void *ptr = std::malloc(size);                      \
        printf("Allocating %zu bytes at %p.\n", size, ptr); \
        return ptr;                                         \
    }                                                       \
                                                            \
    void operator delete(void *ptr) noexcept                \
    {                                                       \
        printf("Deallocating %p.\n", ptr);                  \
        std::free(ptr);                                     \
    }

#else
#define DBG_LOG(...)
#define DBG_LOG_IF(cond, \
                   ...)
#define DBG_ASSERT(cond, \
                   ...)
#define DBG_LOG_ALLOCATOR()
#endif
#endif