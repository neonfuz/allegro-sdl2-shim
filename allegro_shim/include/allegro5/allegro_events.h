#ifndef ALLEGRO_EVENTS_H
#define ALLEGRO_EVENTS_H

#include "allegro_base.h"
#include "allegro_display.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_EVENT_ANY                      0
#define ALLEGRO_EVENT_JOYSTICK                 1
#define ALLEGRO_EVENT_KEYBOARD                 2
#define ALLEGRO_EVENT_MOUSE                    3
#define ALLEGRO_EVENT_TIMER                    4
#define ALLEGRO_EVENT_DISPLAY                  5

#define ALLEGRO_EVENT_JOYSTICK_AXIS            1
#define ALLEGRO_EVENT_JOYSTICK_BUTTON_UP       2
#define ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN     3
#define ALLEGRO_EVENT_JOYSTICK_CONFIGURATION  4

#define ALLEGRO_EVENT_KEY_DOWN                 1
#define ALLEGRO_EVENT_KEY_UP                   2
#define ALLEGRO_EVENT_KEY_CHAR                 3

#define ALLEGRO_EVENT_MOUSE_AXES               1
#define ALLEGRO_EVENT_MOUSE_BUTTON_DOWN        2
#define ALLEGRO_EVENT_MOUSE_BUTTON_UP          3
#define ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY    4
#define ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY     5
#define ALLEGRO_EVENT_MOUSE_WARPED             6

#define ALLEGRO_EVENT_TIMER                    1

#define ALLEGRO_EVENT_DISPLAY_EXPOSE           1
#define ALLEGRO_EVENT_DISPLAY_RESIZE           2
#define ALLEGRO_EVENT_DISPLAY_CLOSE            3
#define ALLEGRO_EVENT_DISPLAY_FOCUS_LOST      4
#define ALLEGRO_EVENT_DISPLAY_FOCUS_GAINED     5
#define ALLEGRO_EVENT_DISPLAY_SWITCH_OUT       6
#define ALLEGRO_EVENT_DISPLAY_SWITCH_IN        7

typedef struct ALLEGRO_EVENT_SOURCE {
    int dummy;
} ALLEGRO_EVENT_SOURCE;

typedef struct ALLEGRO_EVENT_QUEUE ALLEGRO_EVENT_QUEUE;

typedef struct ALLEGRO_EVENT {
    int type;
    ALLEGRO_DISPLAY* display;
    double timestamp;
    union {
        struct {
            int dx;
            int dy;
            int dz;
            int dw;
            int x;
            int y;
            int button;
            float pressure;
        } mouse;
        struct {
            int keycode;
            int unichar;
            int modifiers;
        } keyboard;
        struct {
            int id;
            float x;
            float y;
            float z;
            float rx;
            float ry;
            float rz;
            int button[4];
        } joystick;
        struct {
            long long count;
        } timer;
        struct {
            int x;
            int y;
            int width;
            int height;
        } display_expose;
    };
} ALLEGRO_EVENT;

void al_init_event_queue(ALLEGRO_EVENT_QUEUE* queue);
void al_destroy_event_queue(ALLEGRO_EVENT_QUEUE* queue);
bool al_is_event_queue_empty(ALLEGRO_EVENT_QUEUE* queue);
bool al_get_next_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event);
bool al_peek_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event);
void al_drop_next_event(ALLEGRO_EVENT_QUEUE* queue);
void al_flush_event_queue(ALLEGRO_EVENT_QUEUE* queue);
void al_wait_for_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event);
bool al_wait_for_event_timed(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event, float secs);
bool al_wait_for_event_until(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event, void* timeout);

void al_register_event_source(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT_SOURCE* source);
void al_unregister_event_source(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT_SOURCE* source);

void al_init_event_source(ALLEGRO_EVENT_SOURCE* source);
void al_destroy_event_source(ALLEGRO_EVENT_SOURCE* source);

#ifdef __cplusplus
}
#endif

#endif
