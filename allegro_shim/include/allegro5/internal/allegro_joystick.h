#ifndef ALLEGRO_INTERNAL_JOYSTICK_H
#define ALLEGRO_INTERNAL_JOYSTICK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>
#include "allegro5/allegro_joystick.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ALLEGRO_JOYSTICK {
    SDL_GameController* controller;
    SDL_Joystick* joystick;
    char name[256];
    int index;
    bool is_controller;
};

#ifdef __cplusplus
}
#endif

#endif
