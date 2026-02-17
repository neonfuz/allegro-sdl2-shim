# Event Implementation Research

## Event Types
- ALLEGRO_EVENT types are defined as #define constants (not enum for C compatibility)
- Events: JOYSTICK, KEYBOARD, MOUSE, TIMER, DISPLAY
- Event subtypes: KEY_DOWN, KEY_UP, KEY_CHAR, MOUSE_BUTTON_DOWN, etc.

## Event Structure
- ALLEGRO_EVENT is a union containing event-type-specific data
- Contains: type, display, timestamp, and type-specific union

## Event Queue Implementation Notes
- SDL2 uses SDL_PollEvent and SDL_WaitEvent
- Need to map SDL events to Allegro events
- Event queue can use std::vector or circular buffer

## SDL to Allegro Event Mapping
- SDL_KEYDOWN/SDL_KEYUP -> ALLEGRO_EVENT_KEY_DOWN/KEY_UP
- SDL_MOUSEBUTTONDOWN/SDL_MOUSEBUTTONUP -> ALLEGRO_EVENT_MOUSE_BUTTON_DOWN/UP
- SDL_MOUSEWHEEL -> MOUSE_AXES
- SDL_WINDOWEVENT -> ALLEGRO_EVENT_DISPLAY_*
- SDL_JOY* events -> ALLEGRO_EVENT_JOYSTICK_*
