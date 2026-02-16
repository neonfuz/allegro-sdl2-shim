#ifndef ALLEGRO_COLOR_H
#define ALLEGRO_COLOR_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALLEGRO_COLOR {
    float r;
    float g;
    float b;
    float a;
} ALLEGRO_COLOR;

ALLEGRO_COLOR al_map_rgb(uint8_t r, uint8_t g, uint8_t b);
ALLEGRO_COLOR al_map_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
ALLEGRO_COLOR al_map_rgb_f(float r, float g, float b);
ALLEGRO_COLOR al_map_rgba_f(float r, float g, float b, float a);
ALLEGRO_COLOR al_premul_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
ALLEGRO_COLOR al_premul_rgba_f(float r, float g, float b, float a);

void al_unmap_rgb(ALLEGRO_COLOR color, uint8_t* r, uint8_t* g, uint8_t* b);
void al_unmap_rgba(ALLEGRO_COLOR color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
void al_unmap_rgb_f(ALLEGRO_COLOR color, float* r, float* g, float* b);
void al_unmap_rgba_f(ALLEGRO_COLOR color, float* r, float* g, float* b, float* a);

bool al_color_name_to_rgb(const char* name, ALLEGRO_COLOR* color);

#ifdef __cplusplus
}
#endif

#endif
