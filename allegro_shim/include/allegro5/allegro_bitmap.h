#ifndef ALLEGRO_BITMAP_H
#define ALLEGRO_BITMAP_H

#include "allegro_base.h"
#include "allegro_color.h"
#include "allegro_display.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_BITMAP_FLAGS_MAX 0

#define ALLEGRO_MEMORY_BITMAP      1
#define ALLEGRO_VIDEO_BITMAP       2
#define ALLEGRO_FORCE_LOCKING       4
#define ALLEGRO_NO_PRESERVE_TEXTURE 8
#define ALLEGRO_ALPHA_TEST         16
#define ALLEGRO_INTERNAL_OPENGL    32
#define ALLEGRO_MIN_LINEAR         64
#define ALLEGRO_MAG_LINEAR         128
#define ALLEGRO_MIPMAP             256
#define ALLEGRO_NO_PRELOADING      512
#define ALLEGRO_HAMT               1024
#define ALLEGRO_FORCE_INTEGER_SCALE 2048

#define ALLEGRO_FLIP_HORIZONTAL    1
#define ALLEGRO_FLIP_VERTICAL      2

#define ALLEGRO_PIXEL_FORMAT_ANY                 0
#define ALLEGRO_PIXEL_FORMAT_ANY_NO_ALPHA        1
#define ALLEGRO_PIXEL_FORMAT_ANY_WITH_ALPHA      2
#define ALLEGRO_PIXEL_FORMAT_ANY_15_NO_ALPHA     3
#define ALLEGRO_PIXEL_FORMAT_ANY_15_WITH_ALPHA   4
#define ALLEGRO_PIXEL_FORMAT_ANY_16_NO_ALPHA     5
#define ALLEGRO_PIXEL_FORMAT_ANY_16_WITH_ALPHA   6
#define ALLEGRO_PIXEL_FORMAT_ANY_24_NO_ALPHA     7
#define ALLEGRO_PIXEL_FORMAT_ANY_24_WITH_ALPHA   8
#define ALLEGRO_PIXEL_FORMAT_ANY_32_NO_ALPHA     9
#define ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA   10
#define ALLEGRO_PIXEL_FORMAT_ARGB_8888           11
#define ALLEGRO_PIXEL_FORMAT_RGBA_8888           12
#define ALLEGRO_PIXEL_FORMAT_ABGR_8888           13
#define ALLEGRO_PIXEL_FORMAT_BGRA_8888           14
#define ALLEGRO_PIXEL_FORMAT_RGB_888              15
#define ALLEGRO_PIXEL_FORMAT_BGR_888              16
#define ALLEGRO_PIXEL_FORMAT_RGB_565              17
#define ALLEGRO_PIXEL_FORMAT_RGBA_5551            18
#define ALLEGRO_PIXEL_FORMAT_ARGB_1555            19
#define ALLEGRO_PIXEL_FORMAT_ABGR_F32             20
#define ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE        21

typedef struct ALLEGRO_BITMAP ALLEGRO_BITMAP;

ALLEGRO_BITMAP* al_create_bitmap(int w, int h);
void al_destroy_bitmap(ALLEGRO_BITMAP* bitmap);
int al_get_bitmap_width(ALLEGRO_BITMAP* bitmap);
int al_get_bitmap_height(ALLEGRO_BITMAP* bitmap);
int al_get_bitmap_format(ALLEGRO_BITMAP* bitmap);
int al_get_bitmap_flags(ALLEGRO_BITMAP* bitmap);
void al_set_target_bitmap(ALLEGRO_BITMAP* bitmap);
ALLEGRO_BITMAP* al_get_target_bitmap(void);
void al_set_new_bitmap_flags(int flags);
int al_get_new_bitmap_flags(void);
void al_set_new_bitmap_format(int format);
int al_get_new_bitmap_format(void);
bool al_is_compatible_bitmap(ALLEGRO_BITMAP* bitmap);
ALLEGRO_BITMAP* al_clone_bitmap(ALLEGRO_BITMAP* bitmap);
void al_convert_bitmap(ALLEGRO_BITMAP* bitmap);
ALLEGRO_BITMAP* al_get_backbuffer(ALLEGRO_DISPLAY* display);
void al_set_target_backbuffer(ALLEGRO_DISPLAY* display);

void al_draw_bitmap(ALLEGRO_BITMAP* bitmap, float dx, float dy, int flags);
void al_draw_bitmap_region(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh, float dx, float dy, int flags);
void al_draw_scaled_bitmap(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, int flags);
void al_draw_tinted_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float dx, float dy, int flags);
void al_draw_tinted_scaled_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, int flags);

void al_put_pixel(float x, float y, ALLEGRO_COLOR color);
void al_put_blended_pixel(float x, float y, ALLEGRO_COLOR color);
void al_get_pixel(ALLEGRO_BITMAP* bitmap, float x, float y, ALLEGRO_COLOR* color);

void al_set_clipping_rectangle(float x, float y, float w, float h);
void al_get_clipping_rectangle(float* x, float* y, float* w, float* h);
void al_reset_clipping_rectangle(void);

#ifdef __cplusplus
}
#endif

#endif
