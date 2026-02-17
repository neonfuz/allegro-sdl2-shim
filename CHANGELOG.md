#Changelog

## 2026-02-16
- Create allegro_shim directory structure (allegro_shim/include/allegro5/ and allegro_shim/src/)
- Create basic build test to verify CMake setup works - Fixed CMakeLists.txt to use pkg-config instead of missing CMake targets, successfully built allegro_shim static library
- Create allegro5/allegro_base.h with base types, version macros, and math constants
- Create allegro5/internal/allegro.h includes wrapper
- Implement display management functions (create/destroy display, flip/clear, window controls)
- Implement color mapping functions (al_map_rgb, al_map_rgba, etc.)
- Implement bitmap management system (create/destroy/target bitmaps, drawing functions, clipping)
- Implement drawing primitives (rectangles, lines, circles, ellipses, triangles, polygons)
- Implement keyboard subsystem (al_install_keyboard, al_get_keyboard_state, al_key_down, al_keycode_to_name, al_set_keyboard_leds)
- Implement al_color_name_to_rgb with 140+ X11 color names
- Implement State Management (allegro_state.h, al_store_state, al_restore_state, al_init_state)
- Implement Transformations (allegro_transform.h, matrix operations, translate/rotate/scale)
- Implement Blending (allegro_blender.h, al_set_blender, al_set_separate_blender)
- Implement Events (allegro_events.h, event queue, event waiting functions)
- Implement mouse input system (allegro_mouse.h, al_install_mouse, al_get_mouse_state, etc.)
- Implement legacy mouse functions (install_mouse, remove_mouse, al_get_mouse_event_source)
- Create allegro_joystick.h header with type definitions, joystick state, and function declarations
- Implement joystick input system (allegro_joystick.h, al_install_joystick, al_get_joystick, al_get_joystick_state, etc.)
- Create allegro_audio.h header for audio system (types, enums, function declarations)
- Implement al_load_sample_f for loading samples from ALLEGRO_FILE handles
- Implement al_attach_sample_instance_to_voice and voice subsystem
- Create allegro_timer.h header with ALLEGRO_TIMER type and function declarations
- Fix pre-existing audio return type cast issues in allegro_shim.cpp
- Implement al_load_audio_stream_f for loading audio streams from ALLEGRO_FILE handles
- Implement al_mixer_attach_sample convenience function


