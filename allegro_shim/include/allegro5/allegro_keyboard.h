#ifndef ALLEGRO_KEYBOARD_H
#define ALLEGRO_KEYBOARD_H

#include "allegro_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_KEY_MAX 238

#define ALLEGRO_KEY_ESCAPE          1
#define ALLEGRO_KEY_1               2
#define ALLEGRO_KEY_2               3
#define ALLEGRO_KEY_3               4
#define ALLEGRO_KEY_4               5
#define ALLEGRO_KEY_5               6
#define ALLEGRO_KEY_6               7
#define ALLEGRO_KEY_7               8
#define ALLEGRO_KEY_8               9
#define ALLEGRO_KEY_9               10
#define ALLEGRO_KEY_0               11
#define ALLEGRO_KEY_MINUS           12
#define ALLEGRO_KEY_EQUALS          13
#define ALLEGRO_KEY_BACKSPACE       14
#define ALLEGRO_KEY_TAB             15
#define ALLEGRO_KEY_Q               16
#define ALLEGRO_KEY_W               17
#define ALLEGRO_KEY_E               18
#define ALLEGRO_KEY_R               19
#define ALLEGRO_KEY_T               20
#define ALLEGRO_KEY_Y               21
#define ALLEGRO_KEY_U               22
#define ALLEGRO_KEY_I               23
#define ALLEGRO_KEY_O               24
#define ALLEGRO_KEY_P               25
#define ALLEGRO_KEY_OPENBRACE       26
#define ALLEGRO_KEY_CLOSEBRACE      27
#define ALLEGRO_KEY_ENTER           28
#define ALLEGRO_KEY_LCTRL           29
#define ALLEGRO_KEY_A               30
#define ALLEGRO_KEY_S               31
#define ALLEGRO_KEY_D               32
#define ALLEGRO_KEY_F               33
#define ALLEGRO_KEY_G               34
#define ALLEGRO_KEY_H               35
#define ALLEGRO_KEY_J               36
#define ALLEGRO_KEY_K               37
#define ALLEGRO_KEY_L               38
#define ALLEGRO_KEY_SEMICOLON       39
#define ALLEGRO_KEY_QUOTE           40
#define ALLEGRO_KEY_TILDE           41
#define ALLEGRO_KEY_LSHIFT          42
#define ALLEGRO_KEY_BACKSLASH       43
#define ALLEGRO_KEY_Z               44
#define ALLEGRO_KEY_X               45
#define ALLEGRO_KEY_C               46
#define ALLEGRO_KEY_V               47
#define ALLEGRO_KEY_B               48
#define ALLEGRO_KEY_N               49
#define ALLEGRO_KEY_M               50
#define ALLEGRO_KEY_COMMA           51
#define ALLEGRO_KEY_FULLSTOP        52
#define ALLEGRO_KEY_SLASH           53
#define ALLEGRO_KEY_RSHIFT          54
#define ALLEGRO_KEY_PAD_ASTERISK    55
#define ALLEGRO_KEY_LALT            56
#define ALLEGRO_KEY_SPACE           57
#define ALLEGRO_KEY_CAPSLOCK        58
#define ALLEGRO_KEY_F1              59
#define ALLEGRO_KEY_F2              60
#define ALLEGRO_KEY_F3              61
#define ALLEGRO_KEY_F4              62
#define ALLEGRO_KEY_F5              63
#define ALLEGRO_KEY_F6              64
#define ALLEGRO_KEY_F7              65
#define ALLEGRO_KEY_F8              66
#define ALLEGRO_KEY_F9              67
#define ALLEGRO_KEY_F10             68
#define ALLEGRO_KEY_NUMLOCK         69
#define ALLEGRO_KEY_SCROLLLOCK      70
#define ALLEGRO_KEY_PAD_7           71
#define ALLEGRO_KEY_PAD_8           72
#define ALLEGRO_KEY_PAD_9           73
#define ALLEGRO_KEY_PAD_MINUS       74
#define ALLEGRO_KEY_PAD_4           75
#define ALLEGRO_KEY_PAD_5           76
#define ALLEGRO_KEY_PAD_6           77
#define ALLEGRO_KEY_PAD_PLUS        78
#define ALLEGRO_KEY_PAD_1           79
#define ALLEGRO_KEY_PAD_2           80
#define ALLEGRO_KEY_PAD_3           81
#define ALLEGRO_KEY_PAD_0           82
#define ALLEGRO_KEY_PAD_DELETE      83
#define ALLEGRO_KEY_F11             84
#define ALLEGRO_KEY_F12             85
#define ALLEGRO_KEY_PAD_ENTER       86
#define ALLEGRO_KEY_RCTRL           87
#define ALLEGRO_KEY_PAD_SLASH       88
#define ALLEGRO_KEY_ALTGR           89
#define ALLEGRO_KEY_PAUSE           90
#define ALLEGRO_KEY_HOME            91
#define ALLEGRO_KEY_UP              92
#define ALLEGRO_KEY_PGUP            93
#define ALLEGRO_KEY_LEFT            94
#define ALLEGRO_KEY_RIGHT           95
#define ALLEGRO_KEY_END             96
#define ALLEGRO_KEY_DOWN            97
#define ALLEGRO_KEY_PGDN            98
#define ALLEGRO_KEY_INSERT          99
#define ALLEGRO_KEY_DELETE          100
#define ALLEGRO_KEY_LWIN            101
#define ALLEGRO_KEY_RWIN            102
#define ALLEGRO_KEY_MENU            103

#define ALLEGRO_KEYMOD_SHIFT     1
#define ALLEGRO_KEYMOD_CTRL      2
#define ALLEGRO_KEYMOD_ALT       4
#define ALLEGRO_KEYMOD_LWIN      8
#define ALLEGRO_KEYMOD_RWIN      16
#define ALLEGRO_KEYMOD_MENU      32
#define ALLEGRO_KEYMOD_ALTGR     64
#define ALLEGRO_KEYMOD_COMMAND   128

typedef struct ALLEGRO_KEYBOARD ALLEGRO_KEYBOARD;
typedef struct ALLEGRO_KEYBOARD_STATE ALLEGRO_KEYBOARD_STATE;

struct ALLEGRO_KEYBOARD_STATE {
    unsigned int __key_down__internal__[(ALLEGRO_KEY_MAX + 31) / 32];
};

bool al_install_keyboard(void);
void al_uninstall_keyboard(void);
bool al_is_keyboard_installed(void);
void al_get_keyboard_state(ALLEGRO_KEYBOARD_STATE* ret_state);
bool al_key_down(const ALLEGRO_KEYBOARD_STATE* state, int keycode);
const char* al_keycode_to_name(int keycode);
bool al_can_set_keyboard_leds(void);
bool al_set_keyboard_leds(int leds);
void* al_get_keyboard_event_source(void);

#ifdef __cplusplus
}
#endif

#endif
