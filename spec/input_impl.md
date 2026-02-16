# Allegro 5 Input Module Implementation Plan (SDL2)

## Overview

This document details the implementation plan for the Allegro 5 Input Module, which handles keyboard, mouse, and joystick input. The implementation maps Allegro 5 API calls to SDL2 equivalents.

---

## Type Mappings

### Input Types

| Allegro Type | SDL2 Equivalent | Notes |
|--------------|-----------------|-------|
| `ALLEGRO_KEYBOARD` | `struct AllegroKeyboard*` | Contains keyboard state and event source |
| `ALLEGRO_KEYBOARD_STATE` | `Uint8 key[512]` + bitfield array | Maintains key[] array for legacy ZQuest |
| `ALLEGRO_MOUSE` | `struct AllegroMouse*` | Contains mouse state and event source |
| `ALLEGRO_MOUSE_STATE` | `struct {int x, y, z, w; int buttons;}` | Position and button state |
| `ALLEGRO_JOYSTICK` | `SDL_GameController*` or `SDL_Joystick*` | Game controller/joystick handle |
| `ALLEGRO_JOYSTICK_STATE` | `struct AllegroJoystickState` | Stick positions and buttons |

### Mouse Button Mapping

| Allegro Button | SDL2 Button | Notes |
|----------------|-------------|-------|
| `ALLEGRO_MOUSE_BUTTON_LEFT` | `SDL_BUTTON_LEFT` | 1 |
| `ALLEGRO_MOUSE_BUTTON_RIGHT` | `SDL_BUTTON_RIGHT` | 2 |
| `ALLEGRO_MOUSE_BUTTON_MIDDLE` | `SDL_BUTTON_MIDDLE` | 3 |

---

## Global State Management

### Input State Structure

```cpp
struct AllegroInputState {
    // Keyboard state
    bool keyboard_installed = false;
    Uint8 key[512] = {0};                    // Legacy ZQuest key[] array
    unsigned int key_down_bits[(ALLEGRO_KEY_MAX + 31) / 32] = {0};  // For ALLEGRO_KEYBOARD_STATE
    
    // Mouse state
    bool mouse_installed = false;
    int mouse_x = 0;
    int mouse_y = 0;
    int mouse_z = 0;
    int mouse_w = 0;
    int mouse_buttons = 0;
    unsigned int mouse_num_buttons = 3;
    unsigned int mouse_num_axes = 2;
    
    // Joystick state
    bool joystick_installed = false;
    std::vector<SDL_GameController*> controllers;
    std::vector<SDL_Joystick*> joysticks;
    
    // Event sources
    AllegroEventSource keyboard_event_source;
    AllegroEventSource mouse_event_source;
    AllegroEventSource joystick_event_source;
};
```

### Keyboard Structure

```cpp
struct AllegroKeyboard {
    AllegroEventSource event_source;
    ALLEGRO_DISPLAY* display;
};
```

### Mouse Structure

```cpp
struct AllegroMouse {
    AllegroEventSource event_source;
    ALLEGRO_DISPLAY* display;
};
```

### Joystick Structure

```cpp
struct AllegroJoystick {
    SDL_GameController* controller;
    SDL_Joystick* joystick;
    char name[256];
    AllegroJoystickState last_state;
    int index;
};
```

---

## Keyboard Functions

### al_install_keyboard()

**Function Signature:**
```c
bool al_install_keyboard(void);
```

**SDL2 Equivalent:** Initialize SDL2 keyboard subsystem via event handling

**Implementation:**
1. Check if already installed, return true if so
2. Set `state.keyboard_installed = true`
3. Ensure SDL_INIT_GAMECONTROLLER is NOT set (we want raw keyboard)
4. Initialize keyboard event source
5. Clear key[] array to zeros
6. Return true

**Notes:** ZQuest Classic uses global `key[]` array for legacy keyboard handling - must maintain this array alongside SDL keyboard state.

---

### al_uninstall_keyboard()

**Function Signature:**
```c
void al_uninstall_keyboard(void);
```

**Implementation:**
1. Set `state.keyboard_installed = false`
2. Clear key[] array
3. Clear key_down_bits array

---

### al_is_keyboard_installed()

**Function Signature:**
```c
bool al_is_keyboard_installed(void);
```

**Implementation:** Return `state.keyboard_installed`

---

### al_get_keyboard_state()

**Function Signature:**
```c
void al_get_keyboard_state(ALLEGRO_KEYBOARD_STATE *ret_state);
```

**SDL2 Equivalent:** Poll SDL keyboard state via `SDL_GetKeyboardState()`

**Implementation:**
1. Get SDL keyboard state via `SDL_GetKeyboardState(NULL)`
2. Map SDL scancodes to Allegro keycodes
3. Update key[] array from SDL state
4. Populate ret_state->__key_down__internal__ bitfield
5. Set ret_state->display to current display

**Key Mapping (SDL to Allegro):**
```cpp
int convert_sdl_scancode_to_allegro(SDL_Scancode sdl_scancode) {
    // SDL scancodes map reasonably well to Allegro keycodes
    // Most letters and numbers match directly (SDL_SCANCODE_A = 0, Allegro KEY_A = 1)
    // Need mapping for special keys
    switch (sdl_scancode) {
        case SDL_SCANCODE_ESCAPE: return ALLEGRO_KEY_ESCAPE;
        case SDL_SCANCODE_BACKSPACE: return ALLEGRO_KEY_BACKSPACE;
        case SDL_SCANCODE_TAB: return ALLEGRO_KEY_TAB;
        case SDL_SCANCODE_RETURN: return ALLEGRO_KEY_ENTER;
        case SDL_SCANCODE_SPACE: return ALLEGRO_KEY_SPACE;
        case SDL_SCANCODE_EXCLAIM: return ALLEGRO_KEY_1;
        case SDL_SCANCODE_QUOTEDBL: return ALLEGRO_KEY_QUOTE;
        case SDL_SCANCODE_HASH: return ALLEGRO_KEY_3;
        case SDL_SCANCODE_DOLLAR: return ALLEGRO_KEY_4;
        case SDL_SCANCODE_AMPERSAND: return ALLEGRO_KEY_7;
        case SDL_SCANCODE_LEFTPAREN: return ALLEGRO_KEY_9;
        case SDL_SCANCODE_RIGHTPAREN: return ALLEGRO_KEY_0;
        case SDL_SCANCODE_ASTERISK: return ALLEGRO_KEY_8;
        case SDL_SCANCODE_PLUS: return ALLEGRO_KEY_EQUALS;
        case SDL_SCANCODE_MINUS: return ALLEGRO_KEY_MINUS;
        case SDL_SCANCODE_PERIOD: return ALLEGRO_KEY_FULLSTOP;
        case SDL_SCANCODE_SLASH: return ALLEGRO_KEY_SLASH;
        case SDL_SCANCODE_COLON: return ALLEGRO_KEY_SEMICOLON;
        case SDL_SCANCODE_SEMICOLON: return ALLEGRO_KEY_SEMICOLON;
        case SDL_SCANCODE_LESS: return ALLEGRO_KEY_COMMA;
        case SDL_SCANCODE_GREATER: return ALLEGRO_KEY_FULLSTOP;
        case SDL_SCANCODE_QUESTION: return ALLEGRO_KEY_SLASH;
        case SDL_SCANCODE_AT: return ALLEGRO_KEY_2;
        case SDL_SCANCODE_LEFTBRACKET: return ALLEGRO_KEY_OPENBRACE;
        case SDL_SCANCODE_BACKSLASH: return ALLEGRO_KEY_BACKSLASH;
        case SDL_SCANCODE_RIGHTBRACKET: return ALLEGRO_KEY_CLOSEBRACE;
        case SDL_SCANCODE_CARET: return ALLEGRO_KEY_6;
        case SDL_SCANCODE_UNDERSCORE: return ALLEGRO_KEY_MINUS;
        case SDL_SCANCODE_BACKQUOTE: return ALLEGRO_KEY_TILDE;
        // Function keys
        case SDL_SCANCODE_F1: return ALLEGRO_KEY_F1;
        case SDL_SCANCODE_F2: return ALLEGRO_KEY_F2;
        case SDL_SCANCODE_F3: return ALLEGRO_KEY_F3;
        case SDL_SCANCODE_F4: return ALLEGRO_KEY_F4;
        case SDL_SCANCODE_F5: return ALLEGRO_KEY_F5;
        case SDL_SCANCODE_F6: return ALLEGRO_KEY_F6;
        case SDL_SCANCODE_F7: return ALLEGRO_KEY_F7;
        case SDL_SCANCODE_F8: return ALLEGRO_KEY_F8;
        case SDL_SCANCODE_F9: return ALLEGRO_KEY_F9;
        case SDL_SCANCODE_F10: return ALLEGRO_KEY_F10;
        case SDL_SCANCODE_F11: return ALLEGRO_KEY_F11;
        case SDL_SCANCODE_F12: return ALLEGRO_KEY_F12;
        // Arrow keys
        case SDL_SCANCODE_UP: return ALLEGRO_KEY_UP;
        case SDL_SCANCODE_DOWN: return ALLEGRO_KEY_DOWN;
        case SDL_SCANCODE_LEFT: return ALLEGRO_KEY_LEFT;
        case SDL_SCANCODE_RIGHT: return ALLEGRO_KEY_RIGHT;
        // Modifier keys
        case SDL_SCANCODE_LSHIFT: return ALLEGRO_KEY_LSHIFT;
        case SDL_SCANCODE_RSHIFT: return ALLEGRO_KEY_RSHIFT;
        case SDL_SCANCODE_LCTRL: return ALLEGRO_KEY_LCTRL;
        case SDL_SCANCODE_RCTRL: return ALLEGRO_KEY_RCTRL;
        case SDL_SCANCODE_LALT: return ALLEGRO_KEY_ALT;
        case SDL_SCANCODE_RALT: return ALLEGRO_KEY_ALTGR;
        case SDL_SCANCODE_LGUI: return ALLEGRO_KEY_LWIN;
        case SDL_SCANCODE_RGUI: return ALLEGRO_KEY_RWIN;
        case SDL_SCANCODE_MENU: return ALLEGRO_KEY_MENU;
        // Numpad
        case SDL_SCANCODE_KP_0: return ALLEGRO_KEY_PAD_0;
        case SDL_SCANCODE_KP_1: return ALLEGRO_KEY_PAD_1;
        case SDL_SCANCODE_KP_2: return ALLEGRO_KEY_PAD_2;
        case SDL_SCANCODE_KP_3: return ALLEGRO_KEY_PAD_3;
        case SDL_SCANCODE_KP_4: return ALLEGRO_KEY_PAD_4;
        case SDL_SCANCODE_KP_5: return ALLEGRO_KEY_PAD_5;
        case SDL_SCANCODE_KP_6: return ALLEGRO_KEY_PAD_6;
        case SDL_SCANCODE_KP_7: return ALLEGRO_KEY_PAD_7;
        case SDL_SCANCODE_KP_8: return ALLEGRO_KEY_PAD_8;
        case SDL_SCANCODE_KP_9: return ALLEGRO_KEY_PAD_9;
        case SDL_SCANCODE_KP_DIVIDE: return ALLEGRO_KEY_PAD_SLASH;
        case SDL_SCANCODE_KP_MULTIPLY: return ALLEGRO_KEY_PAD_ASTERISK;
        case SDL_SCANCODE_KP_MINUS: return ALLEGRO_KEY_PAD_MINUS;
        case SDL_SCANCODE_KP_PLUS: return ALLEGRO_KEY_PAD_PLUS;
        case SDL_SCANCODE_KP_ENTER: return ALLEGRO_KEY_PAD_ENTER;
        case SDL_SCANCODE_KP_PERIOD: return ALLEGRO_KEY_PAD_DELETE;
        case SDL_SCANCODE_KP_EQUALS: return ALLEGRO_KEY_PAD_EQUALS;
        // Special keys
        case SDL_SCANCODE_INSERT: return ALLEGRO_KEY_INSERT;
        case SDL_SCANCODE_DELETE: return ALLEGRO_KEY_DELETE;
        case SDL_SCANCODE_HOME: return ALLEGRO_KEY_HOME;
        case SDL_SCANCODE_END: return ALLEGRO_KEY_END;
        case SDL_SCANCODE_PAGEUP: return ALLEGRO_KEY_PGUP;
        case SDL_SCANCODE_PAGEDOWN: return ALLEGRO_KEY_PGDN;
        case SDL_SCANCODE_PRINTSCREEN: return ALLEGRO_KEY_PRINTSCREEN;
        case SDL_SCANCODE_PAUSE: return ALLEGRO_KEY_PAUSE;
        case SDL_SCANCODE_SCROLLLOCK: return ALLEGRO_KEY_SCROLLLOCK;
        case SDL_SCANCODE_NUMLOCKCLEAR: return ALLEGRO_KEY_NUMLOCK;
        case SDL_SCANCODE_CAPSLOCK: return ALLEGRO_KEY_CAPSLOCK;
        default: return ALLEGRO_KEY_UNKNOWN;
    }
}
```

---

### al_key_down()

**Function Signature:**
```c
bool al_key_down(const ALLEGRO_KEYBOARD_STATE *state, int keycode);
```

**Implementation:**
1. Check if keycode is within valid range (0 to ALLEGRO_KEY_MAX)
2. Use bitfield to check if key is down: `state->__key_down__internal__[keycode / 32] & (1 << (keycode % 32))`
3. Return true if key is down, false otherwise

---

### al_keycode_to_name()

**Function Signature:**
```c
const char* al_keycode_to_name(int keycode);
```

**Implementation:** Return static string array for key names (e.g., "A", "F1", "RETURN")

---

### al_can_set_keyboard_leds()

**Function Signature:**
```c
bool al_can_set_keyboard_leds(void);
```

**Implementation:** Return false (SDL2 does not support setting keyboard LEDs directly)

---

### al_set_keyboard_leds()

**Function Signature:**
```c
bool al_set_keyboard_leds(int leds);
```

**Implementation:** Return false (not supported in SDL2)

---

### al_get_keyboard_event_source()

**Function Signature:**
```c
ALLEGRO_EVENT_SOURCE* al_get_keyboard_event_source(void);
```

**Implementation:** Return `&state.keyboard_event_source`

---

## Mouse Functions

### al_install_mouse()

**Function Signature:**
```c
bool al_install_mouse(void);
```

**SDL2 Equivalent:** Initialize SDL2 mouse subsystem

**Implementation:**
1. Check if already installed, return true if so
2. Set `state.mouse_installed = true`
3. Get mouse capabilities via SDL
4. Query number of buttons and axes
5. Initialize mouse event source
6. Return true

---

### al_uninstall_mouse()

**Function Signature:**
```c
void al_uninstall_mouse(void);
```

**Implementation:**
1. Set `state.mouse_installed = false`
2. Reset mouse position and buttons

---

### al_is_mouse_installed()

**Function Signature:**
```c
bool al_is_mouse_installed(void);
```

**Implementation:** Return `state.mouse_installed`

---

### al_get_mouse_num_buttons()

**Function Signature:**
```c
unsigned int al_get_mouse_num_buttons(void);
```

**Implementation:** Return `state.mouse_num_buttons`

---

### al_get_mouse_num_axes()

**Function Signature:**
```c
unsigned int al_get_mouse_num_axes(void);
```

**Implementation:** Return `state.mouse_num_axes` (at least 2 for X/Y)

---

### al_get_mouse_state()

**Function Signature:**
```c
void al_get_mouse_state(ALLEGRO_MOUSE_STATE *ret_state);
```

**SDL2 Equivalent:** Get mouse state via `SDL_GetMouseState()`

**Implementation:**
1. Call `SDL_GetMouseState(&x, &y)` to get position
2. Get mouse button state via `SDL_GetGlobalMouseState()` or `SDL_GetMouseState()`
3. Set ret_state->x, ret_state->y from SDL coordinates
4. Set ret_state->z = mouse_wheel (tracked separately)
5. Set ret_state->w = 0 (no secondary wheel in SDL2)
6. Convert SDL button states to Allegro format:
   - SDL_BUTTON_LEFT → bit 0
   - SDL_BUTTON_RIGHT → bit 1
   - SDL_BUTTON_MIDDLE → bit 2
7. Set ret_state->display to current display

**Type Mappings:**
- `ret_state->x` → mouse X position
- `ret_state->y` → mouse Y position
- `ret_state->z` → mouse wheel position (tracked from SDL_MOUSEWHEEL events)
- `ret_state->w` → secondary wheel (always 0 in SDL2)
- `ret_state->buttons` → bitfield of pressed buttons

---

### al_mouse_button_down()

**Function Signature:**
```c
bool al_mouse_button_down(const ALLEGRO_MOUSE_STATE *state, int button);
```

**Implementation:**
1. Check if button is valid (1-31)
2. Return `(state->buttons & (1 << (button - 1))) != 0`

---

### al_get_mouse_state_axis()

**Function Signature:**
```c
int al_get_mouse_state_axis(const ALLEGRO_MOUSE_STATE *state, int axis);
```

**Implementation:**
1. Switch on axis:
   - case 0: return state->x
   - case 1: return state->y
   - case 2: return state->z
   - case 3: return state->w
   - default: if axis < ALLEGRO_MOUSE_MAX_EXTRA_AXES, return state->more_axes[axis]

---

### al_set_mouse_xy()

**Function Signature:**
```c
bool al_set_mouse_xy(ALLEGRO_DISPLAY *display, int x, int y);
```

**SDL2 Equivalent:** `SDL_WarpMouseInWindow()`

**Implementation:**
1. Call `SDL_WarpMouseInWindow(state.window, x, y)`
2. Update state.mouse_x, state.mouse_y

---

### al_set_mouse_z()

**Function Signature:**
```c
bool al_set_mouse_z(int z);
```

**Implementation:** Set `state.mouse_z = z`

---

### al_set_mouse_w()

**Function Signature:**
```c
bool al_set_mouse_w(int w);
```

**Implementation:** Set `state.mouse_w = w`

---

### al_set_mouse_axis()

**Function Signature:**
```c
bool al_set_mouse_axis(int axis, int value);
```

**Implementation:**
1. Switch on axis and set appropriate state variable
2. Return true

---

### al_get_mouse_cursor_position()

**Function Signature:**
```c
bool al_get_mouse_cursor_position(int *ret_x, int *ret_y);
```

**SDL2 Equivalent:** `SDL_GetMouseState()`

**Implementation:** Call SDL_GetMouseState and populate ret_x, ret_y

---

### al_can_get_mouse_cursor_position()

**Function Signature:**
```c
bool al_can_get_mouse_cursor_position(void);
```

**Implementation:** Return true (SDL2 supports this)

---

### al_grab_mouse()

**Function Signature:**
```c
bool al_grab_mouse(ALLEGRO_DISPLAY *display);
```

**Implementation:** SDL2 doesn't have direct grab equivalent; capture mouse via SDL_SetRelativeMouseMode

---

### al_ungrab_mouse()

**Function Signature:**
```c
bool al_ungrab_mouse(void);
```

**Implementation:** Call `SDL_SetRelativeMouseMode(SDL_FALSE)`

---

### al_set_mouse_wheel_precision()

**Function Signature:**
```c
void al_set_mouse_wheel_precision(int precision);
```

**Implementation:** Store precision value (not directly used in SDL2)

---

### al_get_mouse_wheel_precision()

**Function Signature:**
```c
int al_get_mouse_wheel_precision(void);
```

**Implementation:** Return stored precision value

---

### al_get_mouse_event_source()

**Function Signature:**
```c
ALLEGRO_EVENT_SOURCE* al_get_mouse_event_source(void);
```

**Implementation:** Return `&state.mouse_event_source`

---

## Joystick Functions

### al_install_joystick()

**Function Signature:**
```c
bool al_install_joystick(void);
```

**SDL2 Equivalent:** Initialize SDL2 joystick subsystem via `SDL_Init(SDL_INIT_JOYSTICK)` or `SDL_Init(SDL_INIT_GAMECONTROLLER)`

**Implementation:**
1. Check if already installed, return true if so
2. Call `SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)` if not already initialized
3. Set `state.joystick_installed = true`
4. Initialize joystick event source
5. Open all connected game controllers via `SDL_GameControllerOpen()`
6. Open legacy joysticks via `SDL_JoystickOpen()` for non-controller devices
7. Return true

**Notes:** SDL2 prefers GameController API for modern gamepads. We maintain both for compatibility.

---

### al_uninstall_joystick()

**Function Signature:**
```c
void al_uninstall_joystick(void);
```

**Implementation:**
1. Close all game controllers via `SDL_GameControllerClose()`
2. Close all joysticks via `SDL_JoystickClose()`
3. Clear controller and joystick vectors
4. Set `state.joystick_installed = false`

---

### al_is_joystick_installed()

**Function Signature:**
```c
bool al_is_joystick_installed(void);
```

**Implementation:** Return `state.joystick_installed`

---

### al_reconfigure_joysticks()

**Function Signature:**
```c
bool al_reconfigure_joysticks(void);
```

**Implementation:**
1. Close all existing controllers/joysticks
2. Re-scan for connected devices
3. Re-open all devices
4. Return true

---

### al_get_num_joysticks()

**Function Signature:**
```c
int al_get_num_joysticks(void);
```

**Implementation:** Return `SDL_NumJoysticks()` (includes both controllers and joysticks)

---

### al_get_joystick()

**Function Signature:**
```c
ALLEGRO_JOYSTICK* al_get_joystick(int joyn);
```

**Implementation:**
1. Check if joyn is valid (within controller/joystick vector bounds)
2. Return pointer to AllegroJoystick at index

---

### al_release_joystick()

**Function Signature:**
```c
void al_release_joystick(ALLEGRO_JOYSTICK *joystick);
```

**Implementation:**
1. Close the SDL_GameController or SDL_Joystick
2. Remove from vector or mark as closed

---

### al_get_joystick_active()

**Function Signature:**
```c
bool al_get_joystick_active(ALLEGRO_JOYSTICK *joystick);
```

**Implementation:** Return true if joystick is connected and active

---

### al_get_joystick_name()

**Function Signature:**
```c
const char* al_get_joystick_name(ALLEGRO_JOYSTICK *joystick);
```

**Implementation:** Return joystick->name

---

### al_get_joystick_num_sticks()

**Function Signature:**
```c
int al_get_joystick_num_sticks(ALLEGRO_JOYSTICK *joystick);
```

**Implementation:**
- For game controllers: return 2 (left stick + right stick) + 1 D-pad = 3
- For legacy joysticks: return number of axes / 2 (pairs)

---

### al_get_joystick_stick_flags()

**Function Signature:**
```c
int al_get_joystick_stick_flags(ALLEGRO_JOYSTICK *joystick, int stick);
```

**Implementation:** Return `ALLEGRO_JOYFLAG_ANALOGUE` for analog sticks

---

### al_get_joystick_stick_name()

**Function Signature:**
```c
const char* al_get_joystick_stick_name(ALLEGRO_JOYSTICK *joystick, int stick);
```

**Implementation:** Return static string for stick names ("Left Stick", "Right Stick", "D-Pad")

---

### al_get_joystick_num_axes()

**Function Signature:**
```c
int al_get_joystick_num_axes(ALLEGRO_JOYSTICK *joystick, int stick);
```

**Implementation:** Return 2 for each stick (X and Y axes)

---

### al_get_joystick_axis_name()

**Function Signature:**
```c
const char* al_get_joystick_axis_name(ALLEGRO_JOYSTICK *joystick, int stick, int axis);
```

**Implementation:** Return static string for axis names ("X", "Y", "Z")

---

### al_get_joystick_num_buttons()

**Function Signature:**
```c
int al_get_joystick_num_buttons(ALLEGRO_JOYSTICK *joystick);
```

**Implementation:**
- For game controllers: return `SDL_CONTROLLER_BUTTON_MAX` (15-20 buttons)
- For legacy joysticks: return number of buttons

---

### al_get_joystick_button_name()

**Function Signature:**
```c
const char* al_get_joystick_button_name(ALLEGRO_JOYSTICK *joystick, int button);
```

**Implementation:** Return static string for button names ("A", "B", "X", "Y", etc.)

---

### al_get_joystick_state()

**Function Signature:**
```c
void al_get_joystick_state(ALLEGRO_JOYSTICK *joystick, ALLEGRO_JOYSTICK_STATE *ret_state);
```

**SDL2 Equivalent:** `SDL_GameControllerGetAxis()` for axes, `SDL_GameControllerGetButton()` for buttons

**Implementation:**
1. For game controllers:
   - Get left stick: axis 0 (X), axis 1 (Y) → normalize to -1.0 to 1.0
   - Get right stick: axis 2 (X), axis 3 (Y) → normalize to -1.0 to 1.0
   - Get triggers: axis 4 (L2), axis 5 (R2) → normalize to 0.0 to 1.0
   - Map buttons: convert SDL_GameControllerButton to button states
2. For legacy joysticks:
   - Get axes via `SDL_JoystickGetAxis()`
   - Get buttons via `SDL_JoystickGetButton()`
3. Populate ret_state->stick[].axis[] with normalized float values (-1.0 to 1.0)
4. Populate ret_state->button[] with integer values (0 or non-zero)

**Type Mappings:**
- `ret_state->stick[0].axis[0]` → left stick X (-1.0 to 1.0)
- `ret_state->stick[0].axis[1]` → left stick Y (-1.0 to 1.0)
- `ret_state->stick[1].axis[0]` → right stick X (-1.0 to 1.0)
- `ret_state->stick[1].axis[1]` → right stick Y (-1.0 to 1.0)
- `ret_state->stick[2]` → D-pad (digital, so -1/0/1 values)
- `ret_state->button[n]` → button states (0 or 1, but Allegro uses 0-32767)

---

### al_get_joystick_event_source()

**Function Signature:**
```c
ALLEGRO_EVENT_SOURCE* al_get_joystick_event_source(void);
```

**Implementation:** Return `&state.joystick_event_source`

---

## Event Handling

### SDL to Allegro Input Event Conversion

```cpp
void process_sdl_keyboard_event(SDL_Event& sdl_event) {
    int allegro_keycode = convert_sdl_keycode(sdl_event.key.keysym.scancode);
    
    // Update global key[] array (legacy ZQuest)
    if (sdl_event.type == SDL_KEYDOWN) {
        state.key[allegro_keycode] = 1;
    } else {
        state.key[allegro_keycode] = 0;
    }
    
    // Update bitfield for ALLEGRO_KEYBOARD_STATE
    if (sdl_event.type == SDL_KEYDOWN) {
        state.key_down_bits[allegro_keycode / 32] |= (1 << (allegro_keycode % 32));
    } else {
        state.key_down_bits[allegro_keycode / 32] &= ~(1 << (allegro_keycode % 32));
    }
}

void process_sdl_mouse_event(SDL_Event& sdl_event) {
    switch (sdl_event.type) {
        case SDL_MOUSEMOTION:
            state.mouse_x = sdl_event.motion.x;
            state.mouse_y = sdl_event.motion.y;
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            state.mouse_buttons |= (1 << (sdl_event.button.button - 1));
            break;
            
        case SDL_MOUSEBUTTONUP:
            state.mouse_buttons &= ~(1 << (sdl_event.button.button - 1));
            break;
            
        case SDL_MOUSEWHEEL:
            state.mouse_z += sdl_event.wheel.y;  // Accumulate wheel movement
            break;
    }
}

void process_sdl_joystick_event(SDL_Event& sdl_event) {
    // Update joystick state based on SDL events
    // For game controllers, use SDL_GameControllerEventState()
}
```

---

## Legacy ZQuest Compatibility

### Global key[] Array

ZQuest Classic uses a global `key[]` array (512 elements) for legacy keyboard handling. This array must be maintained for backward compatibility:

```cpp
// In global state
Uint8 key[512] = {0};

// Updated via al_get_keyboard_state() or event processing
void update_key_array() {
    const Uint8* sdl_keyboard = SDL_GetKeyboardState(NULL);
    // Map SDL scancodes to Allegro keycodes and update key[]
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        int allegro_key = convert_sdl_scancode_to_allegro((SDL_Scancode)i);
        if (allegro_key < 512) {
            key[allegro_key] = sdl_keyboard[i] ? 1 : 0;
        }
    }
}
```

---

## File Organization

```
allegro_shim/
├── include/
│   └── allegro5/
│       ├── allegro.h
│       ├── keyboard.h
│       ├── mouse.h
│       ├── joystick.h
│       └── keycodes.h
├── src/
│   ├── input.cpp              # Input module implementation
│   └── shim_state.cpp         # Global state
└── spec/
    └── input_impl.md          # This document
```

---

## Implementation Order

### Phase 1: Keyboard (Priority: High)
1. `al_install_keyboard()` / `al_uninstall_keyboard()`
2. `al_is_keyboard_installed()`
3. `al_get_keyboard_state()` - maintains key[] array
4. `al_key_down()`

### Phase 2: Mouse (Priority: High)
1. `al_install_mouse()` / `al_uninstall_mouse()`
2. `al_is_mouse_installed()`
3. `al_get_mouse_state()`
4. `al_mouse_button_down()`

### Phase 3: Joystick (Priority: Medium)
1. `al_install_joystick()` / `al_uninstall_joystick()`
2. `al_is_joystick_installed()`
3. `al_get_num_joysticks()`
4. `al_get_joystick()`
5. `al_get_joystick_state()`

### Phase 4: Advanced Features
1. Event sources
2. Mouse positioning functions
3. Joystick configuration
4. LED control (keyboard)

---

## Notes

- SDL2 keyboard state is obtained via `SDL_GetKeyboardState()` which provides current key states
- SDL2 uses scancodes (physical key position) vs keycodes (character mapping) - we map scancodes to Allegro keycodes
- Mouse wheel events in SDL2 come as SDL_MOUSEWHEEL, need to track accumulated z value
- GameController API is preferred in SDL2 for modern controllers; legacy joystick API for older devices
- The global key[] array must be updated every frame or on keyboard events for ZQuest compatibility
- SDL2 mouse coordinates are relative to window; may need conversion for fullscreen

---

## New Details from SDL Source Analysis

### SDL2 Input APIs

The repository contains **SDL2** with input capabilities:

1. **SDL2 Event System**: Located in `src/events/`
   - `SDL_events.c` - Main event handling
   - `SDL_keyboard.c` - Keyboard handling
   - `SDL_mouse.c` - Mouse handling
   - `SDL_windowevents.c` - Window events

2. **Gamepad/Joystick**: Located in `src/joystick/`
   - `SDL_gamepad.c` - Game controller API (SDL 2.0.4+)
   - `SDL_joystick.c` - Legacy joystick support
   - Controller database for automatic mapping
   - HIDAPI backends

3. **Sensor Support**: Located in `src/sensor/` (SDL 2.0.5+)
   - `SDL_sensor.c` - Sensor API
   - Platform implementations: CoreMotion (iOS), Android, Windows
   - Accelerometer, gyroscope support

4. **Haptic Feedback**: Located in `src/haptic/`
   - `SDL_haptic.c` - Force feedback
   - Windows, Linux, Darwin, Android implementations

### SDL2 Event Types

SDL2 event types:
- `SDL_KEYDOWN` / `SDL_KEYUP` - Keyboard
- `SDL_TEXTINPUT` - Text input
- `SDL_MOUSEMOTION` / `SDL_MOUSEBUTTON_*` / `SDL_MOUSEWHEEL` - Mouse
- `SDL_JOYAXISMOTION` / `SDL_JOYBUTTON_*` - Joystick
- `SDL_CONTROLLER_*` - Game controller (SDL 2.0.4+)

### SDL2 Input Headers

| Header | Purpose |
|--------|---------|
| SDL2/SDL_keyboard.h | Keyboard state |
| SDL2/SDL_keycode.h | Key codes |
| SDL2/SDL_scancode.h | Scan codes |
| SDL2/SDL_mouse.h | Mouse state |
| SDL2/SDL_gamepad.h | Game controller (2.0.4+) |
| SDL2/SDL_joystick.h | Joystick |
| SDL2/SDL_sensor.h | Sensors (2.0.5+) |
| SDL2/SDL_haptic.h | Haptic feedback |

### SDL2 Implementation Notes

1. **Keyboard**: Use `SDL_GetKeyboardState()` for current key states
2. **Mouse**: Use `SDL_GetMouseState()` for position/buttons
3. **Gamepad**: Use `SDL_GameControllerOpen()`, `SDL_GameControllerGetAxis()`, `SDL_GameControllerGetButton()` (SDL 2.0.4+)
4. **Joystick**: Use `SDL_JoystickOpen()` for legacy joysticks

### Game Controller Database

SDL2 includes a controller database:
- Located in `src/joystick/controller_db.h`
- Automatic mapping for many controllers
- Can be updated via `SDL_GameControllerAddMapping()`

### Dependencies Update

| Library | Notes |
|---------|-------|
| SDL2 | Native input handling |
| SDL_hidapi | For custom HID devices |
