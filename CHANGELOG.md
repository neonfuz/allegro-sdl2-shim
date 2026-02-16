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
