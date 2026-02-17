#ifndef ALLEGRO_TRANSFORM_H
#define ALLEGRO_TRANSFORM_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALLEGRO_TRANSFORM {
    float m[16];
} ALLEGRO_TRANSFORM;

void al_identity_transform(ALLEGRO_TRANSFORM* trans);
void al_copy_transform(ALLEGRO_TRANSFORM* dest, const ALLEGRO_TRANSFORM* src);
void al_use_transform(ALLEGRO_TRANSFORM* trans);
ALLEGRO_TRANSFORM* al_get_current_transform(void);
void al_invert_transform(ALLEGRO_TRANSFORM* trans);
int al_check_inverse(const ALLEGRO_TRANSFORM* trans);
void al_transform_coordinates(const ALLEGRO_TRANSFORM* trans, float* x, float* y);
void al_compose_transform(ALLEGRO_TRANSFORM* dest, const ALLEGRO_TRANSFORM* src);
void al_translate_transform(ALLEGRO_TRANSFORM* trans, float x, float y, float z);
void al_rotate_transform(ALLEGRO_TRANSFORM* trans, float angle);
void al_scale_transform(ALLEGRO_TRANSFORM* trans, float sx, float sy, float sz);
void al_translate_transform_f(ALLEGRO_TRANSFORM* trans, float x, float y, float z);
void al_rotate_transform_f(ALLEGRO_TRANSFORM* trans, float angle, float x, float y, float z);
void al_scale_transform_f(ALLEGRO_TRANSFORM* trans, float sx, float sy, float sz);

#ifdef __cplusplus
}
#endif

#endif
