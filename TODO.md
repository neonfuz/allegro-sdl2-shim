# Allegro Shim TODO List

An incrementally implementable todo list for building the Allegro 5 to SDL2 shim layer.

> **Spec Files**: See [spec/README.md](spec/README.md) for an index of specification documents.
> 
> **Implementation Specs**:
> - [spec/core_impl.md](spec/core_impl.md) - Display, Events, Initialization
> - [spec/graphics_impl.md](spec/graphics_impl.md) - Bitmap, Drawing, Color, Fonts, Image
> - [spec/audio_impl.md](spec/audio_impl.md) - Audio System
> - [spec/input_impl.md](spec/input_impl.md) - Keyboard, Mouse, Joystick
> - [spec/file_impl.md](spec/file_impl.md) - Configuration, File I/O
> - [spec/thread_impl.md](spec/thread_impl.md) - Threading
> - [spec/addons_impl.md](spec/addons_impl.md) - TTF, Image, Primitives Addons

---

## Phase 1: Project Setup

- [x] Create `allegro_shim/` directory structure
- [x] Create `allegro_shim/include/allegro5/` directory for headers
- [x] Create `allegro_shim/src/` directory for implementations
- [x] Set up `CMakeLists.txt` with SDL2 dependencies
- [x] Add SDL2::SDL2, SDL2::SDL2_image, SDL2::SDL2_mixer, SDL2::SDL2_ttf targets
- [x] Create basic build test to verify CMake setup works

---

## Phase 2: Core Types and Constants
> See: [spec/core_impl.md](spec/core_impl.md) for type mappings

- [x] Create `allegro5/allegro_base.h` with base types
- [x] Define `ALLEGRO_VERSION`, `ALLEGRO_VERSION_INT`
- [x] Define `ALLEGRO_DEBUG`, `ALLEGRO_RELEASE` macros
- [x] Define `ALLEGRO_` prefixed basic types (ALLEGRO_INT, ALLEGRO_FLOAT, etc.)
- [x] Define `ALLEGRO_PI` and math constants
- [x] Create `allegro5/internal/allegro.h` includes wrapper
- [x] Create type aliases: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, etc.
- [x] Define `NULL`, `true`, `false` compatibility if needed

---

## Phase 3: Display Management
> See: [spec/core_impl.md](spec/core_impl.md) - Display Functions

- [x] Create `allegro5/allegro_display.h` header
- [x] Create `allegro5/allegro_color.h` header
- [x] Define `ALLEGRO_DISPLAY` opaque type
- [x] Implement `al_create_display(int w, int h)` - creates SDL_Window + SDL_Renderer
- [x] Implement `al_destroy_display(ALLEGRO_DISPLAY*)` - cleanup
- [x] Implement `al_get_current_display()` - get current display
- [x] Implement `al_set_current_display(ALLEGRO_DISPLAY*)` - set current display
- [x] Implement `al_set_display_flag(ALLEGRO_DISPLAY*, int flag, bool onoff)` - set display flags
- [x] Implement `al_get_display_width(ALLEGRO_DISPLAY*)`
- [x] Implement `al_get_display_height(ALLEGRO_DISPLAY*)`
- [x] Implement `al_flip_display()` - SDL_RenderPresent
- [x] Implement `al_clear_to_color(ALLEGRO_COLOR color)`
- [x] Implement `al_get_window_position(ALLEGRO_DISPLAY*, int* x, int* y)`
- [x] Implement `al_set_window_position(ALLEGRO_DISPLAY*, int x, int y)`
- [x] Implement `al_set_window_title(ALLEGRO_DISPLAY*, const char*)`
- [x] Implement `al_resize_display(ALLEGRO_DISPLAY*, int, int)`
- [x] Implement `al_acknowledge_resize(ALLEGRO_DISPLAY*)`
- [x] Implement display settings functions (new_display_flags, refresh_rate, etc.)
- [x] Implement `al_hold_bitmap_drawing` / `al_is_bitmap_drawing_held`
- [x] Implement color mapping functions

---

## Phase 4: Color System
> See: [spec/graphics_impl.md](spec/graphics_impl.md) - Color Functions

- [x] Create `allegro5/allegro_color.h` header
- [x] Define `ALLEGRO_COLOR` struct with r, g, b, a fields
- [x] Implement `al_map_rgb(uint8_t r, uint8_t g, uint8_t b)` - returns ALLEGRO_COLOR
- [x] Implement `al_map_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)`
- [x] Implement `al_map_rgb_f(float r, float g, float b)` - normalized
- [x] Implement `al_map_rgba_f(float r, float g, float b, float a)` - normalized
- [x] Implement `al_premul_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)` - premultiplied
- [x] Implement `al_premul_rgba_f(float r, float g, float b, float a)`
- [x] Implement `al_unmap_rgb(ALLEGRO_COLOR, uint8_t* r, uint8_t* g, uint8_t* b)`
- [x] Implement `al_unmap_rgba(ALLEGRO_COLOR, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a)`
- [x] Implement `al_unmap_rgb_f(ALLEGRO_COLOR, float* r, float* g, float* b)`
- [x] Implement `al_unmap_rgba_f(ALLEGRO_COLOR, float* r, float* g, float* b, float* a)`
- [x] Implement `al_color_name_to_rgb(const char* name, ALLEGRO_COLOR* color)`
- [x] Implement `al_get_pixel(ALLEGRO_BITMAP*, float x, float y, ALLEGRO_COLOR* color)`

---

## Phase 5: Bitmap Management
> See: [spec/graphics_impl.md](spec/graphics_impl.md) - Bitmap Management

- [x] Create `allegro5/allegro_bitmap.h` header
- [x] Define `ALLEGRO_BITMAP` opaque type (wraps SDL_Texture)
- [x] Implement `al_create_bitmap(int w, int h)` - creates SDL_Texture
- [x] Implement `al_destroy_bitmap(ALLEGRO_BITMAP*)` - destroys texture
- [x] Implement `al_get_bitmap_width(ALLEGRO_BITMAP*)`
- [x] Implement `al_get_bitmap_height(ALLEGRO_BITMAP*)`
- [x] Implement `al_get_bitmap_format(ALLEGRO_BITMAP*)`
- [x] Implement `al_get_bitmap_flags(ALLEGRO_BITMAP*)`
- [x] Implement `al_set_target_bitmap(ALLEGRO_BITMAP*)` - sets render target
- [x] Implement `al_get_target_bitmap()` - gets current render target
- [x] Implement `al_set_new_bitmap_flags(int flags)` - set default flags
- [x] Implement `al_get_new_bitmap_flags()` - get default flags
- [x] Implement `al_set_new_bitmap_format(int format)` - set default format
- [x] Implement `al_is_compatible_bitmap(ALLEGRO_BITMAP*)` - check compatibility
- [x] Implement `al_clone_bitmap(ALLEGRO_BITMAP*)` - clone bitmap
- [x] Implement `al_convert_bitmap(ALLEGRO_BITMAP*)` - convert format
- [x] Implement `al_get_backbuffer(ALLEGRO_DISPLAY*)`
- [x] Implement `al_set_target_backbuffer(ALLEGRO_DISPLAY*)`
- [x] Implement `al_draw_bitmap()` - draw bitmap to screen
- [x] Implement `al_draw_bitmap_region()` - draw portion of bitmap
- [x] Implement `al_draw_scaled_bitmap()` - draw scaled bitmap
- [x] Implement `al_draw_tinted_bitmap()` - draw with color tint
- [x] Implement `al_draw_tinted_scaled_bitmap()` - draw scaled and tinted
- [x] Implement `al_put_pixel()` - draw single pixel
- [x] Implement `al_put_blended_pixel()` - draw blended pixel
- [x] Implement `al_get_pixel()` - get pixel color
- [x] Implement `al_set_clipping_rectangle()` / `al_get_clipping_rectangle()` / `al_reset_clipping_rectangle()`

---

## Phase 6: Drawing Primitives
> See: [spec/graphics_impl.md](spec/graphics_impl.md) - Drawing Primitives

- [x] Create `allegro5/allegro_draw.h` header
- [x] Implement `al_draw_filled_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color)`
- [x] Implement `al_draw_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness)`
- [x] Implement `al_draw_line(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness)`
- [x] Implement `al_draw_circle(float cx, float cy, float r, ALLEGRO_COLOR color, float thickness)`
- [x] Implement `al_draw_filled_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color)`
- [x] Implement `al_draw_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color, float thickness)`
- [x] Implement `al_draw_arc(float cx, float cy, float r, float start_angle, float delta_angle, ALLEGRO_COLOR color, float thickness)`
- [x] Implement `al_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR color, float thickness)`
- [x] Implement `al_draw_filled_triangle(float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR color)`
- [x] Implement `al_draw_polygon(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color, float thickness)`
- [x] Implement `al_draw_filled_polygon(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color)`
- [x] Implement `al_draw_polyline(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color, float thickness, bool closed)`

---

## Phase 7: State Management

- [x] Create `allegro5/allegro_state.h` header
- [x] Define `ALLEGRO_STATE` type for saving/restoring state
- [x] Implement `al_store_state(ALLEGRO_STATE* state)` - save current render state
- [x] Implement `al_restore_state(ALLEGRO_STATE* state)` - restore render state
- [x] Implement `al_init_state(ALLEGRO_STATE* state)` - initialize state struct
- [x] Implement state persistence for target bitmap
- [x] Implement state persistence for blending mode
- [x] Implement state persistence for transformation matrix

---

## Phase 8: Transformations

- [x] Create `allegro5/allegro_transform.h` header
- [x] Define `ALLEGRO_TRANSFORM` type (4x4 matrix)
- [x] Implement `al_identity_transform(ALLEGRO_TRANSFORM*)`
- [x] Implement `al_copy_transform(ALLEGRO_TRANSFORM*, const ALLEGRO_TRANSFORM*)`
- [x] Implement `al_use_transform(ALLEGRO_TRANSFORM*)`
- [x] Implement `al_get_current_transform()` - get active transform
- [x] Implement `al_invert_transform(ALLEGRO_TRANSFORM*)`
- [x] Implement `al_check_inverse(ALLEGRO_TRANSFORM*)` - check if invertible
- [x] Implement `al_transform_coordinates(const ALLEGRO_TRANSFORM*, float* x, float* y)`
- [x] Implement `al_compose_transform(ALLEGRO_TRANSFORM*, const ALLEGRO_TRANSFORM*)`
- [x] Implement `al_translate_transform(float x, float y, float z)`
- [x] Implement `al_rotate_transform(float angle)`
- [x] Implement `al_scale_transform(float sx, float sy, float sz)`
- [x] Implement `al_translate_transform_f(float x, float y, float z)`
- [x] Implement `al_rotate_transform_f(float angle, float x, float y, float z)`
- [x] Implement `al_scale_transform_f(float sx, float sy, float sz)`

---

## Phase 9: Blending

- [x] Create `allegro5/allegro_blender.h` header
- [x] Define blend modes: `ALLEGRO_ADD`, `ALLEGRO_SRC_MINUS_DEST`, etc.
- [x] Define blend operations: `ALLEGRO_ZERO`, `ALLEGRO_ONE`, etc.
- [x] Implement `al_set_blender(int op, int src, int dst)`
- [x] Implement `al_get_blender(int* op, int* src, int* dst)`
- [x] Implement `al_set_separate_blender(int op, int src, int dst, int alpha_op, int src_alpha, int dst_alpha)`
- [x] Implement `al_get_separate_blender(int* op, int* src, int* dst, int* alpha_op, int* src_alpha, int* dst_alpha)`

---

## Phase 10: Events
> See: [spec/core_impl.md](spec/core_impl.md) - Event Queue Functions

- [x] Create `allegro5/allegro_events.h` header
- [x] Define `ALLEGRO_EVENT_TYPE` enumeration
- [x] Define `ALLEGRO_EVENT` union (all event types)
- [x] Define `ALLEGRO_KEY_*` constants (KEY_UP, KEY_DOWN, etc.)
- [x] Define `ALLEGRO_MOUSE_*` constants
- [x] Define `ALLEGRO_JOYSTICK_*` constants
- [x] Define `ALLEGRO_EVENT_QUEUE` type
- [x] Implement `al_create_event_queue()` - creates event queue
- [x] Implement `al_destroy_event_queue(ALLEGRO_EVENT_QUEUE*)`
- [x] Implement `al_register_event_source(ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT_SOURCE*)`
- [x] Implement `al_unregister_event_source(ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT_SOURCE*)`
- [x] Implement `al_is_event_queue_empty(ALLEGRO_EVENT_QUEUE*)`
- [x] Implement `al_get_next_event(ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT*)`
- [x] Implement `al_peek_next_event(ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT*)`
- [x] Implement `al_drop_next_event(ALLEGRO_EVENT_QUEUE*)`
- [x] Implement `al_flush_event_queue(ALLEGRO_EVENT_QUEUE*)`
- [x] Implement `al_wait_for_event(ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT*)`
- [x] Implement `al_wait_for_event_timed(ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT*, float seconds)`
- [x] Implement `al_wait_for_event_until(ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT*, ALLEGRO_TIMEOUT*)`

---

## Phase 11: Keyboard Input
> See: [spec/input_impl.md](spec/input_impl.md) - Keyboard Functions

- [x] Create `allegro5/allegro_keyboard.h` header
- [x] Define `ALLEGRO_KEYBOARD` type
- [x] Define `ALLEGRO_KEYBOARD_STATE` type
- [x] Define `ALLEGRO_KEY_*` key code constants (128+ keys)
- [x] Define `ALLEGRO_KEYMOD_*` modifier constants
- [x] Implement `install_keyboard()` - initialize keyboard system
- [x] Implement `remove_keyboard()` - shutdown keyboard
- [x] Implement `al_install_keyboard()` - A5 keyboard init
- [x] Implement `al_uninstall_keyboard()` - A5 keyboard shutdown
- [x] Implement `al_get_keyboard_event_source()` - get event source
- [x] Implement `al_get_keyboard_state(ALLEGRO_KEYBOARD_STATE*)` - get current state
- [x] Implement `al_key_down(ALLEGRO_KEYBOARD_STATE*, int keycode)` - check key
- [x] Implement `al_get_key_name(int keycode)` - get key name string
- [x] Implement `al_set_keyboard_leds(int leds)` - set LED state
- [x] Implement legacy `key[]` array getter/setter functions
- [x] Map SDL2 `SDL_GetKeyboardState()` to `key[]` array

---

## Phase 12: Mouse Input
> See: [spec/input_impl.md](spec/input_impl.md) - Mouse Functions

- [x] Create `allegro5/allegro_mouse.h` header
- [x] Define `ALLEGRO_MOUSE` type
- [x] Define `ALLEGRO_MOUSE_STATE` type
- [x] Define `ALLEGRO_MOUSE_CURSOR` type
- [x] Define `ALLEGRO_MOUSE_*` button constants
- [x] Implement `install_mouse()` - initialize mouse (legacy)
- [x] Implement `remove_mouse()` - shutdown mouse (legacy)
- [x] Implement `al_get_mouse_event_source()` - get event source
- [x] Implement `al_get_mouse_state(ALLEGRO_MOUSE_STATE*)` - get state
- [x] Implement `al_mouse_button_down(ALLEGRO_MOUSE_STATE*, int button)` - check button
- [x] Implement `al_get_mouse_state_axis(ALLEGRO_MOUSE_STATE*, int axis)` - get axis
- [x] Implement `al_get_mouse_num_axes()` - get axis count
- [x] Implement `al_get_mouse_num_buttons()` - get button count
- [x] Implement `al_set_mouse_xy(ALLEGRO_DISPLAY*, float x, float y)` - warp cursor
- [x] Implement `al_set_mouse_z(float z)` - set wheel
- [x] Implement `al_set_mouse_w(float w)` - set wheel
- [x] Implement `al_get_mouse_cursor_position(int* x, int* y)` - get cursor pos

---

## Phase 13: Joystick Input
> See: [spec/input_impl.md](spec/input_impl.md) - Joystick Functions

- [x] Create `allegro5/allegro_joystick.h` header
- [x] Define `ALLEGRO_JOYSTICK` type
- [x] Define `ALLEGRO_JOYSTICK_STATE` type
- [x] Define `ALLEGRO_JOYFLAGS_*` constants
- [x] Implement `install_joystick()` - init joystick
- [x] Implement `remove_joystick()` - shutdown joystick
- [x] Implement `al_install_joystick()` - A5 joystick init
- [x] Implement `al_uninstall_joystick()` - A5 joystick shutdown
- [x] Implement `al_get_joystick_event_source()` - get event source
- [x] Implement `al_reconfigure_joysticks()` - re-detect joysticks
- [x] Implement `al_get_num_joysticks()` - count joysticks
- [x] Implement `al_get_joystick(int number)` - get joystick
- [x] Implement `al_get_joystick_active(ALLEGRO_JOYSTICK*)` - check active
- [x] Implement `al_get_joystick_name(ALLEGRO_JOYSTICK*)` - get name
- [x] Implement `al_get_joystick_num_axes(ALLEGRO_JOYSTICK*)` - axis count
- [x] Implement `al_get_joystick_num_buttons(ALLEGRO_JOYSTICK*)` - button count
- [x] Implement `al_get_joystick_state(ALLEGRO_JOYSTICK*, ALLEGRO_JOYSTICK_STATE*)` - get state

---

## Phase 14: Audio System
> See: [spec/audio_impl.md](spec/audio_impl.md) - Audio Implementation

- [ ] Create `allegro5/allegro_audio.h` header
- [ ] Define `ALLEGRO_AUDIO_DEVICE` type
- [ ] Define `ALLEGRO_SAMPLE` type (wraps Mix_Chunk)
- [ ] Define `ALLEGRO_SAMPLE_INSTANCE` type
- [ ] Define `ALLEGRO_AUDIO_STREAM` type
- [ ] Define `ALLEGRO_MIXER` type
- [ ] Define `ALLEGRO_VOICE` type
- [ ] Define audio depth: `ALLEGRO_AUDIO_DEPTH_*`
- [ ] Define channel config: `ALLEGRO_CHANNEL_*`
- [ ] Define playmode: `ALLEGRO_PLAYMODE_*`
- [ ] Implement `al_install_audio()` - init audio
- [ ] Implement `al_uninstall_audio()` - shutdown audio
- [ ] Implement `al_init_acodec_addon()` - init audio codecs
- [ ] Implement `al_is_audio_installed()` - check if audio installed
- [ ] Implement `al_get_allegro_audio_version()` - get version
- [ ] Implement `al_create_sample(unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chans, bool free_buffer)`
- [ ] Implement `al_destroy_sample(ALLEGRO_SAMPLE*)`
- [ ] Implement `al_play_sample(ALLEGRO_SAMPLE*, float volume, float pan, float speed, int loop, ALLEGRO_SAMPLE_ID*)`
- [ ] Implement `al_stop_sample(ALLEGRO_SAMPLE_ID*)`
- [ ] Implement `al_stop_samples()`
- [ ] Implement `al_get_sample_frequency(ALLEGRO_SAMPLE*)`
- [ ] Implement `al_get_sample_length(ALLEGRO_SAMPLE*)`
- [ ] Implement `al_get_sample_data(ALLEGRO_SAMPLE*)`
- [ ] Implement `al_load_sample(const char* filename)` - load from file
- [ ] Implement `al_load_sample_f(ALLEGRO_FILE* fp, const char* ident)` - load from file handle
- [ ] Implement `al_save_sample(const char* filename, ALLEGRO_SAMPLE*)` - save to file

---

## Phase 15: Audio Sample Instances

- [ ] Implement `al_create_sample_instance(ALLEGRO_SAMPLE*)`
- [ ] Implement `al_destroy_sample_instance(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_play_sample_instance(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_stop_sample_instance(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_get_sample_instance_playing(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_set_sample_instance_playing(ALLEGRO_SAMPLE_INSTANCE*, bool play)`
- [ ] Implement `al_get_sample_instance_position(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_set_sample_instance_position(ALLEGRO_SAMPLE_INSTANCE*, unsigned int pos)`
- [ ] Implement `al_get_sample_instance_length(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_set_sample_instance_length(ALLEGRO_SAMPLE_INSTANCE*, unsigned int len)`
- [ ] Implement `al_get_sample_instance_speed(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_set_sample_instance_speed(ALLEGRO_SAMPLE_INSTANCE*, float speed)`
- [ ] Implement `al_get_sample_instance_gain(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_set_sample_instance_gain(ALLEGRO_SAMPLE_INSTANCE*, float gain)`
- [ ] Implement `al_get_sample_instance_pan(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_set_sample_instance_pan(ALLEGRO_SAMPLE_INSTANCE*, float pan)`
- [ ] Implement `al_get_sample_instance_playmode(ALLEGRO_SAMPLE_INSTANCE*)`
- [ ] Implement `al_set_sample_instance_playmode(ALLEGRO_SAMPLE_INSTANCE*, int mode)`
- [ ]attach_sample_instance_to Implement `al__mixer(ALLEGRO_SAMPLE_INSTANCE*, ALLEGRO_MIXER*)`
- [ ] Implement `al_attach_sample_instance_to_voice(ALLEGRO_SAMPLE_INSTANCE*, ALLEGRO_VOICE*)`

---

## Phase 16: Audio Streams

- [ ] Implement `al_create_audio_stream(size_t fragment_count, unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chans)`
- [ ] Implement `al_destroy_audio_stream(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_get_audio_stream_frequency(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_get_audio_stream_length(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_get_audio_stream_position(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_set_audio_stream_position(ALLEGRO_AUDIO_STREAM*, unsigned int pos)`
- [ ] Implement `al_get_audio_stream_speed(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_set_audio_stream_speed(ALLEGRO_AUDIO_STREAM*, float speed)`
- [ ] Implement `al_get_audio_stream_gain(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_set_audio_stream_gain(ALLEGRO_AUDIO_STREAM*, float gain)`
- [ ] Implement `al_get_audio_stream_pan(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_set_audio_stream_pan(ALLEGRO_AUDIO_STREAM*, float pan)`
- [ ] Implement `al_get_audio_stream_playing(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_set_audio_stream_playing(ALLEGRO_AUDIO_STREAM*, bool play)`
- [ ] Implement `al_get_audio_stream_playmode(ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_set_audio_stream_playmode(ALLEGRO_AUDIO_STREAM*, int mode)`
- [ ] Implement `al_attach_audio_stream_to_mixer(ALLEGRO_AUDIO_STREAM*, ALLEGRO_MIXER*)`
- [ ] Implement `al_attach_audio_stream_to_voice(ALLEGRO_AUDIO_STREAM*, ALLEGRO_VOICE*)`
- [ ] Implement `al_load_audio_stream(const char* filename, size_t buffer_count, unsigned int samples)`
- [ ] Implement `al_load_audio_stream_f(ALLEGRO_FILE* fp, const char* ident, size_t buffer_count, unsigned int samples)`

---

## Phase 17: Audio Mixer and Voice

- [ ] Implement `al_create_mixer(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chans)`
- [ ] Implement `al_destroy_mixer(ALLEGRO_MIXER*)`
- [ ] Implement `al_mixer_attach_sample(ALLEGRO_MIXER*, ALLEGRO_SAMPLE*)`
- [ ] Implement `al_mixer_attach_audio_stream(ALLEGRO_MIXER*, ALLEGRO_AUDIO_STREAM*)`
- [ ] Implement `al_mixer_detach_sample(ALLEGRO_MIXER*)`
- [ ] Implement `al_mixer_detach_audio_stream(ALLEGRO_MIXER*)`
- [ ] Implement `al_mixer_get_frequency(ALLEGRO_MIXER*)`
- [ ] Implement `al_mixer_get_channels(ALLEGRO_MIXER*)`
- [ ] Implement `al_mixer_get_depth(ALLEGRO_MIXER*)`
- [ ] Implement `al_mixer_get_gain(ALLEGRO_MIXER*)`
- [ ] Implement `al_mixer_set_gain(ALLEGRO_MIXER*, float gain)`
- [ ] Implement `al_create_voice(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chans)`
- [ ] Implement `al_destroy_voice(ALLEGRO_VOICE*)`
- [ ] Implement `al_attach_sample_to_voice(ALLEGRO_SAMPLE*, ALLEGRO_VOICE*)`
- [ ] Implement `al_attach_audio_stream_to_voice(ALLEGRO_AUDIO_STREAM*, ALLEGRO_VOICE*)`
- [ ] Implement `al_detach_voice(ALLEGRO_VOICE*)`
- [ ] Implement `al_voice_get_frequency(ALLEGRO_VOICE*)`
- [ ] Implement `al_voice_get_playing(ALLEGRO_VOICE*)`
- [ ] Implement `al_voice_stop(ALLEGRO_VOICE*)`
- [ ] Implement `al_voice_is_playing(ALLEGRO_VOICE*)`
- [ ] Implement `al_get_default_mixer()` - get system mixer
- [ ] Implement `al_set_default_mixer(ALLEGRO_MIXER*)` - set system mixer

---

## Phase 18: Timers

- [ ] Create `allegro5/allegro_timer.h` header
- [ ] Define `ALLEGRO_TIMER` type
- [ ] Implement `al_install_timer()` - init timer system
- [ ] Implement `al_uninstall_timer()` - shutdown timer
- [ ] Implement `al_create_timer(double speed_secs)` - create timer
- [ ] Implement `al_destroy_timer(ALLEGRO_TIMER*)` - destroy timer
- [ ] Implement `al_start_timer(ALLEGRO_TIMER*)` - start timer
- [ ] Implement `al_stop_timer(ALLEGRO_TIMER*)` - stop timer
- [ ] Implement `al_get_timer_started(ALLEGRO_TIMER*)` - check if started
- [ ] Implement `al_get_timer_speed(ALLEGRO_TIMER*)` - get speed
- [ ] Implement `al_set_timer_speed(ALLEGRO_TIMER*, double speed_secs)` - set speed
- [ ] Implement `al_get_timer_count(ALLEGRO_TIMER*)` - get count
- [ ] Implement `al_set_timer_count(ALLEGRO_TIMER*, long long count)` - set count
- [ ] Implement `al_get_timer_event_source(ALLEGRO_TIMER*)` - get event source

---

## Phase 19: Configuration Files
> See: [spec/file_impl.md](spec/file_impl.md) - Configuration Functions

- [ ] Create `allegro5/allegro_config.h` header
- [ ] Define `ALLEGRO_CONFIG` type
- [ ] Define `ALLEGRO_CONFIG_SECTION` iterator type
- [ ] Define `ALLEGRO_CONFIG_ENTRY` iterator type
- [ ] Implement `al_create_config()` - create empty config
- [ ] Implement `al_destroy_config(ALLEGRO_CONFIG*)` - destroy config
- [ ] Implement `al_load_config_file(const char* filename)` - load from file
- [ ] Implement `al_load_config_f(ALLEGRO_FILE* fp, const char* origin)` - load from file handle
- [ ] Implement `al_save_config_file(const char* filename, ALLEGRO_CONFIG*)` - save to file
- [ ] Implement `al_save_config_f(ALLEGRO_FILE* fp, ALLEGRO_CONFIG*)` - save to file handle
- [ ] Implement `al_add_config_section(ALLEGRO_CONFIG*, const char* section)` - add section
- [ ] Implement `al_add_config_key(ALLEGRO_CONFIG*, const char* section, const char* key, const char* value)` - add key
- [ ] Implement `al_get_config_value(ALLEGRO_CONFIG*, const char* section, const char* key, const char* default_value)` - get value
- [ ] Implement `al_set_config_value(ALLEGRO_CONFIG*, const char* section, const char* key, const char* value)` - set value
- [ ] Implement `al_merge_config(ALLEGRO_CONFIG*, const ALLEGRO_CONFIG*)` - merge configs
- [ ] Implement `al_merge_config_into(ALLEGRO_CONFIG*, const ALLEGRO_CONFIG*)` - merge into
- [ ] Implement `al_get_first_config_section(ALLEGRO_CONFIG*, ALLEGRO_CONFIG_SECTION** iterator)` - iterate sections
- [ ] Implement `al_get_next_config_section(ALLEGRO_CONFIG_SECTION** iterator)` - next section
- [ ] Implement `al_get_first_config_entry(ALLEGRO_CONFIG*, const char* section, ALLEGRO_CONFIG_ENTRY** iterator)` - iterate entries
- [ ] Implement `al_get_next_config_entry(ALLEGRO_CONFIG_ENTRY** iterator)` - next entry

---

## Phase 20: File System
> See: [spec/file_impl.md](spec/file_impl.md) - File I/O Functions

- [ ] Create `allegro5/allegro_file.h` header
- [ ] Define `ALLEGRO_FS_ENTRY` type
- [ ] Define `ALLEGRO_FS_MODE` enum
- [ ] Define `ALLEGRO_FILE` type (wraps FILE*)
- [ ] Define `ALLEGRO_FILE_MODE` enum
- [ ] Implement `al_create_fs_entry(const char* path)` - create file entry
- [ ] Implement `al_destroy_fs_entry(ALLEGRO_FS_ENTRY*)` - destroy entry
- [ ] Implement `al_get_fs_entry_name(ALLEGRO_FS_ENTRY*)` - get filename
- [ ] Implement `al_get_fs_entry_mode(ALLEGRO_FS_ENTRY*)` - get mode
- [ ] Implement `al_fs_entry_exists(ALLEGRO_FS_ENTRY*)` - check exists
- [ ] Implement `al_get_fs_entry_atime(ALLEGRO_FS_ENTRY*)` - access time
- [ ] Implement `al_get_fs_entry_mtime(ALLEGRO_FS_ENTRY*)` - modify time
- [ ] Implement `al_get_fs_entry_size(ALLEGRO_FS_ENTRY*)` - file size
- [ ] Implement `al_update_fs_entry_mode(ALLEGRO_FS_ENTRY*)` - refresh mode
- [ ] Implement `al_update_fs_entry_size(ALLEGRO_FS_ENTRY*)` - refresh size
- [ ] Implement `al_open_directory(ALLEGRO_FS_ENTRY*)` - open directory
- [ ] Implement `al_close_directory(ALLEGRO_FS_ENTRY*)` - close directory
- [ ] Implement `al_read_directory(ALLEGRO_FS_ENTRY*)` - read next entry
- [ ] Implement `al_remove_fs_entry(ALLEGRO_FS_ENTRY*)` - delete file
- [ ] Implement `al_rename_fs_entry(ALLEGRO_FS_ENTRY*, const char* newpath)` - rename
- [ ] Implement `al_create_directory(const char* path)` - create dir
- [ ] Implement `al_fopen(const char* path, const char* mode)` - open file (wraps fopen)
- [ ] Implement `al_fclose(ALLEGRO_FILE*)` - close file
- [ ] Implement `al_fread(ALLEGRO_FILE*, void* ptr, size_t size)` - read
- [ ] Implement `al_fwrite(ALLEGRO_FILE*, const void* ptr, size_t size)` - write
- [ ] Implement `al_fflush(ALLEGRO_FILE*)` - flush
- [ ] Implement `al_ftell(ALLEGRO_FILE*)` - tell position
- [ ] Implement `al_fseek(ALLEGRO_FILE*, int64_t offset, int whence)` - seek
- [ ] Implement `al_feof(ALLEGRO_FILE*)` - check EOF
- [ ] Implement `al_ferror(ALLEGRO_FILE*)` - check error
- [ ] Implement `al_fclearerr(ALLEGRO_FILE*)` - clear errors

---

## Phase 21: UTF-8 Strings

- [ ] Create `allegro5/allegro_utf8.h` header
- [ ] Implement `al_utfsize(unsigned int c)` - get char byte size
- [ ] Implement `al_utf8_encode(char* s, unsigned int c)` - encode UTF-8
- [ ] Implement `al_utf16_to_utf8(const uint16_t* s, int* exit_code)` - convert
- [ ] Implement `al_utf8_to_utf16(const char* s, int* exit_code)` - convert

---

## Phase 22: Memory Files

- [ ] Implement `al_open_memfile(const char* mem, int64_t size, const char* mode)` - open memory file

---

## Phase 23: Fonts
> See: [spec/graphics_impl.md](spec/graphics_impl.md) - Font Rendering, [spec/addons_impl.md](spec/addons_impl.md) - Font Addon

- [ ] Create `allegro5/allegro_font.h` header
- [ ] Define `ALLEGRO_FONT` type (wraps TTF_Font)
- [ ] Define `ALLEGRO_FONT_FLAGS_*` constants
- [ ] Implement `al_init_font_addon()` - init font system
- [ ] Implement `al_shutdown_font_addon()` - shutdown font system
- [ ] Implement `al_load_font(const char* filename, int size, int flags)` - load font
- [ ] Implement `al_destroy_font(ALLEGRO_FONT*)` - destroy font
- [ ] Implement `al_draw_text(ALLEGRO_FONT*, ALLEGRO_COLOR color, float x, float y, int flags, const char* text)` - draw text
- [ ] Implement `al_draw_justified_text(ALLEGRO_FONT*, ALLEGRO_COLOR color, float x1, float x2, float y, float diff, int flags, const char* text)` - justified text
- [ ] Implement `al_draw_textf(ALLEGRO_FONT*, ALLEGRO_COLOR color, float x, float y, int flags, const char* format, ...)` - printf-style text
- [ ] Implement `al_get_text_width(ALLEGRO_FONT*, const char* text)` - text width
- [ ] Implement `al_get_font_line_height(ALLEGRO_FONT*)` - line height
- [ ] Implement `al_get_font_ascent(ALLEGRO_FONT*)` - ascent
- [ ] Implement `al_get_font_descent(ALLEGRO_FONT*)` - descent
- [ ] Implement `al_get_text_dimensions(ALLEGRO_FONT*, const char* text, int* bbx, int* bby, int* bbw, int* bbh)` - text bounding box

---

## Phase 24: TrueType Fonts (TTF)
> See: [spec/addons_impl.md](spec/addons_impl.md) - TTF Addon

- [ ] Create `allegro5/allegro_ttf.h` header
- [ ] Implement `al_init_ttf_addon()` - init TTF addon
- [ ] Implement `al_shutdown_ttf_addon()` - shutdown TTF
- [ ] Implement `al_load_ttf_font(const char* filename, int size, int flags)` - load TTF
- [ ] Implement `al_load_ttf_font_stretch(const char* filename, int width, int height, int flags)` - load with stretch

---

## Phase 25: Image Loading
> See: [spec/graphics_impl.md](spec/graphics_impl.md) - Image Loading/Saving, [spec/addons_impl.md](spec/addons_impl.md) - Image Addon

- [ ] Create `allegro5/allegro_image.h` header
- [ ] Define `ALLEGRO_IMAGE_INTERFACE` type
- [ ] Implement `al_init_image_addon()` - init image loading
- [ ] Implement `al_shutdown_image_addon()` - shutdown
- [ ] Implement `al_load_bitmap(const char* filename)` - load image
- [ ] Implement `al_load_bitmap_flags(const char* filename, int flags)` - load with flags
- [ ] Implement `al_load_bitmap_f(ALLEGRO_FILE* fp, const char* ident)` - load from file handle
- [ ] Implement `al_save_bitmap(const char* filename, ALLEGRO_BITMAP*)` - save image
- [ ] Implement `al_save_bitmap_f(ALLEGRO_FILE* fp, const char* ident, ALLEGRO_BITMAP*)` - save to file

---

## Phase 26: Primitives Addon
> See: [spec/addons_impl.md](spec/addons_impl.md) - Primitives Addon

- [ ] Create `allegro5/allegro_primitives.h` header
- [ ] Implement `al_init_primitives_addon()` - init primitives
- [ ] Implement `al_shutdown_primitives_addon()` - shutdown
- [ ] Implement `al_draw_prim(const void* vertices, const ALLEGRO_VERTEX_ELEMENT* decl, int buffer, int start, int end, int type)` - draw primitives

---

## Phase 27: Native Dialogs

- [ ] Create `allegro5/allegro_native_dialog.h` header
- [ ] Define `ALLEGRO_TEXTLOG` type
- [ ] Define `ALLEGRO_NATIVE_DIALOG` type
- [ ] Define `ALLEGRO_MESSAGEBOX_*` flags
- [ ] Implement `al_init_native_dialog_addon()` - init dialogs
- [ ] Implement `al_shutdown_native_dialog_addon()` - shutdown
- [ ] Implement `al_show_native_message_box(ALLEGRO_DISPLAY*, const char* title, const char* heading, const char* text, const char* buttons, int flags)` - show message box
- [ ] Implement `al_open_native_text_log(const char* title, int flags)` - open text log
- [ ] Implement `al_close_native_text_log(ALLEGRO_TEXTLOG*)` - close log
- [ ] Implement `al_append_native_text_log(ALLEGRO_TEXTLOG*, const char* format, ...)` - append to log

---

## Phase 28: Threading
> See: [spec/thread_impl.md](spec/thread_impl.md) - Threading Implementation

- [ ] Create `allegro5/allegro_thread.h` header
- [ ] Define `ALLEGRO_THREAD` type
- [ ] Define `ALLEGRO_MUTEX` type
- [ ] Define `ALLEGRO_COND` type
- [ ] Define `ALLEGRO_MUTEX_TYPE` enum
- [ ] Implement `al_create_thread(ALLEGRO_THREAD* t, void* (*proc)(ALLEGRO_THREAD*, void*), void* arg)` - create thread
- [ ] Implement `al_start_thread(ALLEGRO_THREAD*)` - start thread
- [ ] Implement `al_join_thread(ALLEGRO_THREAD*, void** retval)` - join thread
- [ ] Implement `al_set_thread_should_stop(ALLEGRO_THREAD*)` - signal stop
- [ ] Implement `al_get_thread_should_stop(ALLEGRO_THREAD*)` - check stop flag
- [ ] Implement `al_create_mutex()` - create mutex
- [ ] Implement `al_create_mutex_recursive()` - create recursive mutex
- [ ] Implement `al_destroy_mutex(ALLEGRO_MUTEX*)` - destroy mutex
- [ ] Implement `al_lock_mutex(ALLEGRO_MUTEX*)` - lock mutex
- [ ] Implement `al_unlock_mutex(ALLEGRO_MUTEX*)` - unlock mutex
- [ ] Implement `al_create_cond()` - create condition
- [ ] Implement `al_destroy_cond(ALLEGRO_COND*)` - destroy condition
- [ ] Implement `al_wait_cond(ALLEGRO_COND*, ALLEGRO_MUTEX*, double timeout)` - wait on condition
- [ ] Implement `al_signal_cond(ALLEGRO_COND*)` - signal one
- [ ] Implement `al_broadcast_cond(ALLEGRO_COND*)` - signal all

---

## Phase 29: Time/Date

- [ ] Create `allegro5/allegro_time.h` header
- [ ] Implement `al_get_time()` - get current time in seconds
- [ ] Implement `al_rest(double seconds)` - sleep/rest

---

## Phase 30: Initialization
> See: [spec/core_impl.md](spec/core_impl.md) - Initialization

- [ ] Create `allegro5/allegro.h` main header
- [ ] Define `ALLEGRO_VERSION` macro
- [ ] Define `ALLEGRO_SUB_VERSION` macro
- [ ] Define `ALLEGRO_VERSION_STR` macro
- [ ] Define `ALLEGRO_VERSION_INT` macro
- [ ] Implement `al_init()` - initialize all subsystems
- [ ] Implement `al_is_system_installed()` - check if initialized
- [ ] Implement `al_get_allegro_version()` - get version
- [ ] Implement `al_install_system(int version, int (*atexit_ptr)(int (*)(void)))` - install system

---

## Phase 31: Main Header Organization

- [ ] Create `allegro5/allegro.h` that includes all sub-headers
- [ ] Add `#ifdef ALLEGRO_INCLUDE_HEADERS` guards
- [ ] Add version compatibility checks
- [ ] Add platform-specific includes

---

## Phase 32: Testing

- [ ] Create `test/test_display.cpp` - test display creation
- [ ] Create `test/test_graphics.cpp` - test drawing primitives
- [ ] Create `test/test_audio.cpp` - test audio playback
- [ ] Create `test/test_input.cpp` - test keyboard/mouse input
- [ ] Create `test/test_font.cpp` - test font rendering
- [ ] Create `test/test_image.cpp` - test image loading
- [ ] Run all tests and fix failures

---

## Phase 33: Integration Testing

- [ ] Integrate shim with ZQuest build system
- [ ] Attempt to compile ZQuest with shim
- [ ] Fix compilation errors
- [ ] Run ZQuest with shim and verify functionality
- [ ] Document any missing features or bugs

---

## Phase 34: Performance Optimization (Optional)

- [ ] Profile rendering performance
- [ ] Add texture caching for frequently drawn bitmaps
- [ ] Optimize drawing primitives with batching
- [ ] Add hardware acceleration where possible

---

## Phase 35: Documentation

- [ ] Write `README.md` for allegro_shim
- [ ] Document known limitations
- [ ] Document platform-specific quirks
- [ ] Document how to use with ZQuest

---

## Phase 36: Legacy Allegro 4 Compatibility (Future)

- [ ] Create shim for legacy `<allegro.h>` functions
- [ ] Implement `key[]` array using SDL2 keyboard state
- [ ] Implement `BITMAP` type using SDL surfaces
- [ ] Implement legacy drawing functions
- [ ] Bridge to A5 shim layer
