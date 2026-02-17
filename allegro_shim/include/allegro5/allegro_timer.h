#ifndef ALLEGRO_TIMER_H
#define ALLEGRO_TIMER_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALLEGRO_TIMER ALLEGRO_TIMER;

bool al_install_timer(void);
void al_uninstall_timer(void);
ALLEGRO_TIMER* al_create_timer(double speed_secs);
void al_destroy_timer(ALLEGRO_TIMER* timer);
void al_start_timer(ALLEGRO_TIMER* timer);
void al_stop_timer(ALLEGRO_TIMER* timer);
bool al_get_timer_started(ALLEGRO_TIMER* timer);
double al_get_timer_speed(ALLEGRO_TIMER* timer);
void al_set_timer_speed(ALLEGRO_TIMER* timer, double speed_secs);
long long al_get_timer_count(ALLEGRO_TIMER* timer);
void al_set_timer_count(ALLEGRO_TIMER* timer, long long count);
void al_add_timer_count(ALLEGRO_TIMER* timer, long long diff);

typedef struct ALLEGRO_EVENT_SOURCE ALLEGRO_EVENT_SOURCE;
ALLEGRO_EVENT_SOURCE* al_get_timer_event_source(ALLEGRO_TIMER* timer);

#ifdef __cplusplus
}
#endif

#endif
