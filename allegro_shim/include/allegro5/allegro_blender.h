#ifndef ALLEGRO_BLENDER_H
#define ALLEGRO_BLENDER_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_ADD                         1
#define ALLEGRO_SUB                         2
#define ALLEGRO_DEST_MINUS_SRC              3
#define ALLEGRO_SRC_MINUS_DEST              4

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

void al_set_blender(int op, int src, int dst);
void al_get_blender(int* op, int* src, int* dst);
void al_set_separate_blender(int op, int src, int dst, int alpha_op, int src_alpha, int dst_alpha);
void al_get_separate_blender(int* op, int* src, int* dst, int* alpha_op, int* src_alpha, int* dst_alpha);

#ifdef __cplusplus
}
#endif

#endif
