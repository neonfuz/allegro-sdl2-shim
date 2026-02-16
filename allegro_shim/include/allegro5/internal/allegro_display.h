#ifndef ALLEGRO_INTERNAL_DISPLAY_H
#define ALLEGRO_INTERNAL_DISPLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include "allegro5/allegro_display.h"
#include "allegro5/allegro_color.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ALLEGRO_DISPLAY {
    SDL_Window* window;
    SDL_Renderer* renderer;
    void* backbuffer;
    int width;
    int height;
    int flags;
    int refresh_rate;
};

extern ALLEGRO_DISPLAY* _al_get_current_display(void);
extern void _al_set_current_display(ALLEGRO_DISPLAY* display);

#ifdef __cplusplus
}
#endif

#endif
