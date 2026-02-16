#ifndef ALLEGRO_DRAW_H
#define ALLEGRO_DRAW_H

#include "allegro_base.h"
#include "allegro_color.h"
#include "allegro_bitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

void al_draw_filled_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color);
void al_draw_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness);
void al_draw_line(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness);
void al_draw_circle(float cx, float cy, float r, ALLEGRO_COLOR color, float thickness);
void al_draw_filled_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color);
void al_draw_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color, float thickness);
void al_draw_arc(float cx, float cy, float r, float start_angle, float delta_angle, ALLEGRO_COLOR color, float thickness);
void al_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR color, float thickness);
void al_draw_filled_triangle(float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR color);
void al_draw_polygon(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color, float thickness);
void al_draw_filled_polygon(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color);
void al_draw_polyline(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color, float thickness, bool closed);

void al_draw_pixel(float x, float y, ALLEGRO_COLOR color);

#ifdef __cplusplus
}
#endif

#endif
