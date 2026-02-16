#ifndef ALLEGRO_BASE_H
#define ALLEGRO_BASE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ALLEGRO_VERSION        5
#define ALLEGRO_SUB_VERSION    2
#define ALLEGRO_VERSION_STR    "5.2.8"
#define ALLEGRO_VERSION_INT    5280

#ifdef DEBUG
    #define ALLEGRO_DEBUG
#else
    #define ALLEGRO_RELEASE
#endif

#define ALLEGRO_PI        3.14159265358979323846
#define ALLEGRO_PI_2      1.57079632679489661923
#define ALLEGRO_PI_3      1.04719755119659774615
#define ALLEGRO_2PI       6.28318530717958647692
#define ALLEGRO_E         2.71828182845904523536

typedef int8_t      ALLEGRO_INT8;
typedef uint8_t     ALLEGRO_UINT8;
typedef int16_t     ALLEGRO_INT16;
typedef uint16_t    ALLEGRO_UINT16;
typedef int32_t     ALLEGRO_INT32;
typedef uint32_t    ALLEGRO_UINT32;
typedef int64_t     ALLEGRO_INT64;
typedef uint64_t    ALLEGRO_UINT64;
typedef float       ALLEGRO_FLOAT;
typedef double      ALLEGRO_DOUBLE;
typedef size_t      ALLEGRO_SIZE;
typedef ptrdiff_t   ALLEGRO_PTRDIFF;
typedef intptr_t    ALLEGRO_INTPTR;
typedef uintptr_t   ALLEGRO_UINTPTR;

typedef ALLEGRO_INT8    ALLEGRO_S8;
typedef ALLEGRO_UINT8   ALLEGRO_U8;
typedef ALLEGRO_INT16  ALLEGRO_S16;
typedef ALLEGRO_UINT16 ALLEGRO_U16;
typedef ALLEGRO_INT32  ALLEGRO_S32;
typedef ALLEGRO_UINT32 ALLEGRO_U32;
typedef ALLEGRO_INT64  ALLEGRO_S64;
typedef ALLEGRO_UINT64 ALLEGRO_U64;

typedef bool ALLEGRO_BOOL;

#ifndef NULL
    #ifdef __cplusplus
        #define NULL nullptr
    #else
        #define NULL ((void*)0)
    #endif
#endif

#endif
