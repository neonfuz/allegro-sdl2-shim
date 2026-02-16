# Allegro Shim Specification Files

This directory contains the specification and implementation planning documents for the Allegro 5 to SDL2 shim layer.

## Index

| File | Description |
|------|-------------|
| [SPEC.md](SPEC.md) | Original design document outlining architecture, module structure, type mappings, and implementation priorities |
| [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) | Detailed implementation guide mapping each Allegro function to its SDL2 equivalent, including type definitions and code structures |
| [core_impl.md](core_impl.md) | Detailed implementation plan for Core Module (display, events, initialization) |
| [graphics_impl.md](graphics_impl.md) | Detailed implementation plan for Graphics Module (bitmaps, drawing, colors, fonts) |
| [audio_impl.md](audio_impl.md) | Detailed implementation plan for Audio Module (samples, streams, mixer) |
| [input_impl.md](input_impl.md) | Detailed implementation plan for Input Module (keyboard, mouse, joystick) |
| [thread_impl.md](thread_impl.md) | Detailed implementation plan for Threading Module (mutex, cond, thread) |
| [file_impl.md](file_impl.md) | Detailed implementation plan for File I/O Module (config, file operations) |
| [addons_impl.md](addons_impl.md) | Detailed implementation plan for Addons Module (image, font, TTF, primitives) |

## Module Implementation Plans

Each module has its own detailed implementation plan created from analyzing the allegro5/ source headers:

### 1. Core Module
**File:** [core_impl.md](core_impl.md)

Functions covered: `al_init()`, `al_create_display()`, `al_destroy_display()`, `al_get_display()`, `al_create_event_queue()`, `al_destroy_event_queue()`, `al_register_event_source()`, `al_flip_display()`, `al_clear_to_color()`, and 20+ additional display/event functions

Type mappings: `ALLEGRO_DISPLAY` → `SDL_Window* + SDL_Renderer*`, `ALLEGRO_EVENT_QUEUE` → custom struct, `ALLEGRO_EVENT` → `SDL_Event`

### 2. Graphics Module
**File:** [graphics_impl.md](graphics_impl.md)

Functions covered: `al_create_bitmap()`, `al_destroy_bitmap()`, `al_set_target_bitmap()`, `al_draw_bitmap()`, `al_draw_scaled_bitmap()`, `al_draw_filled_rectangle()`, `al_draw_line()`, `al_draw_circle()`, `al_map_rgb()`, `al_draw_text()`, `al_load_bitmap()`, and 40+ additional bitmap/drawing/color/font functions

Type mappings: `ALLEGRO_BITMAP` → `SDL_Texture*`, `ALLEGRO_COLOR` → `SDL_Color` (float r,g,b,a), `ALLEGRO_FONT` → `TTF_Font*`

### 3. Audio Module
**File:** [audio_impl.md](audio_impl.md)

Functions covered: `al_install_audio()`, `al_create_sample()`, `al_create_sample_instance()`, `al_attach_sample_instance_to_mixer()`, `al_set_sample_instance_playing()`, `al_load_audio_stream()`, and 60+ additional audio functions

Type mappings: `ALLEGRO_SAMPLE` → `Mix_Chunk*`, `ALLEGRO_SAMPLE_INSTANCE` → custom struct, `ALLEGRO_AUDIO_STREAM` → custom struct

### 4. Input Module
**File:** [input_impl.md](input_impl.md)

Functions covered: `al_install_keyboard()`, `al_install_mouse()`, `al_install_joystick()`, `al_get_mouse_state()`, `al_mouse_button_down()`, and 30+ additional input functions

Type mappings: `ALLEGRO_KEYBOARD_STATE` → key[] array (512 elements for legacy ZQuest), `ALLEGRO_MOUSE_STATE` → struct, `ALLEGRO_JOYSTICK` → `SDL_GameController*`

### 5. Threading Module
**File:** [thread_impl.md](thread_impl.md)

Functions covered: `al_create_mutex()`, `al_lock_mutex()`, `al_unlock_mutex()`, `al_create_cond()`, `al_wait_cond()`, `al_signal_cond()`, and 10+ additional threading functions

Type mappings: `ALLEGRO_THREAD` → `SDL_Thread*`, `ALLEGRO_MUTEX` → `SDL_mutex*`, `ALLEGRO_COND` → `SDL_cond*`

### 6. File I/O Module
**File:** [file_impl.md](file_impl.md)

Functions covered: `al_create_config()`, `al_load_config_file()`, `al_get_config_value()`, `al_set_config_value()`, `al_fopen()`, `al_open_memfile()`, and 40+ additional file/config functions

Type mappings: `ALLEGRO_CONFIG` → custom struct, `ALLEGRO_FILE` → `FILE*`

### 7. Addons Module
**File:** [addons_impl.md](addons_impl.md)

Functions covered: `al_init_image_addon()`, `al_init_font_addon()`, `al_init_primitives_addon()`, `al_init_ttf_addon()`, and addon-specific font/TTF functions

Uses: SDL2_image, SDL2_ttf, SDL2_gfx

## Quick Reference

### Module Coverage

| Module | Implementation Plan File |
|--------|-------------------------|
| Core (display, events, init) | [core_impl.md](core_impl.md) |
| Graphics (bitmaps, drawing, colors) | [graphics_impl.md](graphics_impl.md) |
| Audio (samples, streams, mixer) | [audio_impl.md](audio_impl.md) |
| Input (keyboard, mouse, joystick) | [input_impl.md](input_impl.md) |
| Threading (mutex, cond, thread) | [thread_impl.md](thread_impl.md) |
| File I/O (config, file, fs) | [file_impl.md](file_impl.md) |
| Addons (image, font, TTF, primitives) | [addons_impl.md](addons_impl.md) |

### Type Mapping Summary

| Allegro Type | SDL2 Equivalent |
|--------------|-----------------|
| ALLEGRO_DISPLAY | SDL_Window* + SDL_Renderer* |
| ALLEGRO_BITMAP | SDL_Texture* |
| ALLEGRO_COLOR | SDL_Color (float r,g,b,a) |
| ALLEGRO_EVENT_QUEUE | std::queue<SDL_Event> |
| ALLEGRO_MUTEX | SDL_mutex* |
| ALLEGRO_SAMPLE | Mix_Chunk* |
| ALLEGRO_FONT | TTF_Font* |
