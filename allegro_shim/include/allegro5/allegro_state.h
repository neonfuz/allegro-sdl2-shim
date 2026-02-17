#ifndef ALLEGRO_STATE_H
#define ALLEGRO_STATE_H

#include "allegro_base.h"
#include "allegro_display.h"
#include "allegro_bitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_STATE_NEW_DISPLAY_FLAGS    1
#define ALLEGRO_STATE_NEW_BITMAP_FLAGS     2
#define ALLEGRO_STATE_NEW_BITMAP_FORMAT    4
#define ALLEGRO_STATE_DISPLAY              8
#define ALLEGRO_STATE_TARGET_BITMAP         16
#define ALLEGRO_STATE_TRANSFORM             32
#define ALLEGRO_STATE_PROJECTION            64
#define ALLEGRO_STATE_BLENDER              128
#define ALLEGRO_STATE_ALL                  255

#define ALLEGRO_ADD                         1
#define ALLEGRO_SUB                         2
#define ALLEGRO_DEST_MINUS_SRC              3
#define ALLEGRO_SRC_MINUS_DEST             4

#define ALLEGRO_ZERO                        0
#define ALLEGRO_ONE                         1
#define ALLEGRO_ALPHA                       2
#define ALLEGRO_INVERSE_ALPHA               3
#define ALLEGRO_SRC_COLOR                   4
#define ALLEGRO_DEST_COLOR                  5
#define ALLEGRO_INVERSE_SRC_COLOR          6
#define ALLEGRO_INVERSE_DEST_COLOR         7
#define ALLEGRO_CONST_COLOR                8
#define ALLEGRO_INVERSE_CONST_COLOR        9

typedef struct ALLEGRO_STATE {
    int new_display_flags;
    int new_bitmap_flags;
    int new_bitmap_format;
    ALLEGRO_DISPLAY* current_display;
    ALLEGRO_BITMAP* target_bitmap;
    float transform[16];
    int blender_op;
    int blender_src;
    int blender_dst;
    int blender_alpha_op;
    int blender_alpha_src;
    int blender_alpha_dst;
} ALLEGRO_STATE;

void al_store_state(ALLEGRO_STATE* state, int flags);
void al_restore_state(ALLEGRO_STATE* state);
void al_init_state(ALLEGRO_STATE* state, int flags);

#ifdef __cplusplus
}
#endif

#endif
