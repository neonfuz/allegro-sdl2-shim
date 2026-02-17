#ifndef ALLEGRO_MOUSE_H
#define ALLEGRO_MOUSE_H

#include "allegro_base.h"
#include "allegro_display.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_MOUSE_MAX_EXTRA_AXES 10

#define ALLEGRO_MOUSE_BUTTON_1 1
#define ALLEGRO_MOUSE_BUTTON_2 2
#define ALLEGRO_MOUSE_BUTTON_3 3
#define ALLEGRO_MOUSE_BUTTON_4 4
#define ALLEGRO_MOUSE_BUTTON_5 5
#define ALLEGRO_MOUSE_BUTTON_6 6
#define ALLEGRO_MOUSE_BUTTON_7 7
#define ALLEGRO_MOUSE_BUTTON_8 8

typedef struct ALLEGRO_MOUSE ALLEGRO_MOUSE;
typedef struct ALLEGRO_MOUSE_CURSOR ALLEGRO_MOUSE_CURSOR;

typedef struct ALLEGRO_MOUSE_STATE {
    int x;
    int y;
    int z;
    int w;
    int pressure;
    int button;
    int buttons;
    ALLEGRO_DISPLAY* display;
} ALLEGRO_MOUSE_STATE;

bool al_install_mouse(void);
void al_uninstall_mouse(void);
bool al_is_mouse_installed(void);
void* al_get_mouse_event_source(void);
void al_get_mouse_state(ALLEGRO_MOUSE_STATE* ret_state);
bool al_mouse_button_down(const ALLEGRO_MOUSE_STATE* state, int button);
int al_get_mouse_state_axis(const ALLEGRO_MOUSE_STATE* state, int axis);
int al_get_mouse_num_axes(void);
int al_get_mouse_num_buttons(void);
bool al_set_mouse_xy(ALLEGRO_DISPLAY* display, float x, float y);
bool al_set_mouse_z(float z);
bool al_set_mouse_w(float w);
bool al_get_mouse_cursor_position(int* x, int* y);

#ifdef __cplusplus
}
#endif

#endif
