#ifndef ALLEGRO_JOYSTICK_H
#define ALLEGRO_JOYSTICK_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_JOYFLAG_DIGITAL 1
#define ALLEGRO_JOYFLAG_ANALOGUE 2
#define ALLEGRO_JOYFLAG_POLEV 4
#define ALLEGRO_JOYFLAG_XINPUT 8

#define ALLEGRO_JOYSTICK_MAX_STICKS 8
#define ALLEGRO_JOYSTICK_MAX_AXES 8

typedef struct ALLEGRO_JOYSTICK ALLEGRO_JOYSTICK;

typedef struct ALLEGRO_JOYSTICK_STATE {
    float stick[ALLEGRO_JOYSTICK_MAX_STICKS][ALLEGRO_JOYSTICK_MAX_AXES];
    unsigned int button[32];
} ALLEGRO_JOYSTICK_STATE;

bool al_install_joystick(void);
void al_uninstall_joystick(void);
bool al_is_joystick_installed(void);
bool al_reconfigure_joysticks(void);
int al_get_num_joysticks(void);
ALLEGRO_JOYSTICK* al_get_joystick(int joyn);
void al_release_joystick(ALLEGRO_JOYSTICK* joystick);
bool al_get_joystick_active(ALLEGRO_JOYSTICK* joystick);
const char* al_get_joystick_name(ALLEGRO_JOYSTICK* joystick);
int al_get_joystick_num_sticks(ALLEGRO_JOYSTICK* joystick);
int al_get_joystick_stick_flags(ALLEGRO_JOYSTICK* joystick, int stick);
const char* al_get_joystick_stick_name(ALLEGRO_JOYSTICK* joystick, int stick);
int al_get_joystick_num_axes(ALLEGRO_JOYSTICK* joystick, int stick);
const char* al_get_joystick_axis_name(ALLEGRO_JOYSTICK* joystick, int stick, int axis);
int al_get_joystick_num_buttons(ALLEGRO_JOYSTICK* joystick);
const char* al_get_joystick_button_name(ALLEGRO_JOYSTICK* joystick, int button);
void al_get_joystick_state(ALLEGRO_JOYSTICK* joystick, ALLEGRO_JOYSTICK_STATE* ret_state);
void* al_get_joystick_event_source(void);

int install_joystick(void);
int remove_joystick(void);

#ifdef __cplusplus
}
#endif

#endif
