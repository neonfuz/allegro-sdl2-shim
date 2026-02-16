#ifndef ALLEGRO_DISPLAY_H
#define ALLEGRO_DISPLAY_H

#include "allegro_base.h"
#include "allegro_color.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_DISPLAY_FLAGS_MAX 0

#define ALLEGRO_WINDOWED         0
#define ALLEGRO_FULLSCREEN       1
#define ALLEGRO_FULLSCREEN_WINDOW 2
#define ALLEGRO_RESIZABLE        4
#define ALLEGRO_OPENGL           8
#define ALLEGRO_FRAMEBUFFER      16
#define ALLEGRO_FRAMELESS        32
#define ALLEGRO_MINIMIZED        64
#define ALLEGRO_MAXIMIZED        128

typedef struct ALLEGRO_DISPLAY ALLEGRO_DISPLAY;

ALLEGRO_DISPLAY* al_create_display(int w, int h);
void al_destroy_display(ALLEGRO_DISPLAY* display);
ALLEGRO_DISPLAY* al_get_current_display(void);
void al_set_current_display(ALLEGRO_DISPLAY* display);
int al_get_display_width(ALLEGRO_DISPLAY* display);
int al_get_display_height(ALLEGRO_DISPLAY* display);
int al_get_display_flags(ALLEGRO_DISPLAY* display);
bool al_set_display_flag(ALLEGRO_DISPLAY* display, int flag, bool onoff);
void al_set_window_title(ALLEGRO_DISPLAY* display, const char* title);
bool al_resize_display(ALLEGRO_DISPLAY* display, int width, int height);
bool al_acknowledge_resize(ALLEGRO_DISPLAY* display);
void al_set_window_position(ALLEGRO_DISPLAY* display, int x, int y);
void al_get_window_position(ALLEGRO_DISPLAY* display, int* x, int* y);

void al_flip_display(void);
void al_clear_to_color(ALLEGRO_COLOR color);

void al_set_new_display_flags(int flags);
int al_get_new_display_flags(void);
void al_set_new_display_refresh_rate(int refresh_rate);
int al_get_new_display_refresh_rate(void);
void al_set_new_window_title(const char* title);
const char* al_get_new_window_title(void);
void al_set_new_window_position(int x, int y);
void al_get_new_window_position(int* x, int* y);
void al_set_new_display_adapter(int adapter);
int al_get_new_display_adapter(void);
int al_get_display_adapter(ALLEGRO_DISPLAY* display);

void al_hold_bitmap_drawing(bool hold);
bool al_is_bitmap_drawing_held(void);

#ifdef __cplusplus
}
#endif

#endif
