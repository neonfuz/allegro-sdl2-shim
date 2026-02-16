# Allegro 5 Core Module Implementation Plan (SDL2)

## Overview

This document details the implementation plan for the Allegro 5 Core Module, which handles display management, event queues, and global state. The implementation maps Allegro 5 API calls to SDL2 equivalents.

---

## Type Mappings

### Core Types

| Allegro Type | SDL2 Equivalent | Notes |
|--------------|-----------------|-------|
| `ALLEGRO_DISPLAY` | `struct AllegroDisplay*` | Contains `SDL_Window*` + `SDL_Renderer*` |
| `ALLEGRO_EVENT_QUEUE` | `struct AllegroEventQueue*` | Contains `SDL_Event*` circular buffer |
| `ALLEGRO_EVENT` | `union AllegroEvent` | Mapped from `SDL_Event` |
| `ALLEGRO_EVENT_SOURCE` | `struct AllegroEventSource*` | Custom struct with callbacks |
| `ALLEGRO_COLOR` | `SDL_Color` | RGBA color structure |
| `ALLEGRO_BITMAP` | `SDL_Texture*` | Texture for rendering |
| `ALLEGRO_TIMEOUT` | `SDL_TouchID` (for timeout) | Via SDL_WaitEventTimeout |

### Display Flags

| Allegro Flag | SDL2 Equivalent | Notes |
|--------------|-----------------|-------|
| `ALLEGRO_WINDOWED` | `0` (default) | SDL_WINDOW_SHOWN |
| `ALLEGRO_FULLSCREEN` | `SDL_WINDOW_FULLSCREEN` | Exclusive fullscreen |
| `ALLEGRO_FULLSCREEN_WINDOW` | `SDL_WINDOW_FULLSCREEN_DESKTOP` | Borderless fullscreen |
| `ALLEGRO_RESIZABLE` | `SDL_WINDOW_RESIZABLE` | Window can resize |
| `ALLEGRO_FRAMELESS` | `SDL_WINDOW_BORDERLESS` | No window decorations |
| `ALLEGRO_OPENGL` | N/A | Software renderer fallback |
| `ALLEGRO_MINIMIZED` | `SDL_WINDOW_MINIMIZED` | Window minimized |
| `ALLEGRO_MAXIMIZED` | `SDL_WINDOW_MAXIMIZED` | Window maximized |

---

## Global State Management

```cpp
struct AllegroShimState {
    // Display state
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    AllegroDisplay* current_display = nullptr;
    ALLEGRO_BITMAP* backbuffer = nullptr;
    ALLEGRO_BITMAP* target_bitmap = nullptr;
    
    // Display settings
    int new_display_flags = 0;
    int new_display_refresh_rate = 60;
    std::string new_window_title = "Allegro SDL2 Shim";
    int new_window_x = -1;  // -1 = center
    int new_window_y = -1;
    int new_display_adapter = 0;
    
    // Event queue
    AllegroEventQueue* global_queue = nullptr;
    
    // Input state
    Uint8 key[512] = {0};
    Uint32 mouse_buttons = 0;
    int mouse_x = 0, mouse_y = 0;
    int mouse_z = 0, mouse_w = 0;
    
    // Drawing state
    bool bitmap_drawing_held = false;
    
    // Initialization
    bool initialized = false;
    SDL_mutex* state_mutex = nullptr;
};
```

### Display Structure

```cpp
struct AllegroDisplay {
    SDL_Window* window;
    SDL_Renderer* renderer;
    ALLEGRO_BITMAP* backbuffer;
    int width;
    int height;
    int flags;
    int refresh_rate;
    AllegroEventSource event_source;
};
```

### Event Queue Structure

```cpp
struct AllegroEventQueue {
    std::vector<ALLEGRO_EVENT> events;
    size_t read_index;
    size_t write_index;
    std::vector<AllegroEventSource*> registered_sources;
    bool paused;
    SDL_mutex* mutex;
};
```

### Event Source Structure

```cpp
struct AllegroEventSource {
    AllegroDisplay* display;  // NULL for non-display sources
    intptr_t user_data;
    // For custom event sources
    void (*destructor)(ALLEGRO_USER_EVENT*) = nullptr;
};
```

---

## Function Implementations

### 1. Initialization

#### al_init()

**Function Signature:**
```c
bool al_init(void);
```

**SDL2 Equivalent:** Initialize SDL2 video, initialize global state

**Implementation:**
1. Call `SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)` 
2. Initialize global `AllegroShimState` structure
3. Create mutex for thread safety
4. Set `state.initialized = true`
5. Return `true` on success

**Notes:** SDL_Init must be called before any other SDL functions. The timer subsystem is needed for event timing.

---

### 2. Display Functions

#### al_create_display()

**Function Signature:**
```c
ALLEGRO_DISPLAY* al_create_display(int w, int h);
```

**SDL2 Equivalent:** Create SDL_Window + SDL_Renderer

**Implementation:**
1. Read global display settings (flags, position, adapter)
2. Convert Allegro flags to SDL window flags:
   - `ALLEGRO_WINDOWED` → `SDL_WINDOW_SHOWN`
   - `ALLEGRO_FULLSCREEN` → `SDL_WINDOW_FULLSCREEN`
   - `ALLEGRO_FULLSCREEN_WINDOW` → `SDL_WINDOW_FULLSCREEN_DESKTOP`
   - `ALLEGRO_RESIZABLE` → `SDL_WINDOW_RESIZABLE`
   - `ALLEGRO_FRAMELESS` → `SDL_WINDOW_BORDERLESS`
3. Call `SDL_CreateWindow(title, x, y, w, h, flags)`
4. Call `SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)`
5. Create `ALLEGRO_BITMAP` wrapper for backbuffer (stores SDL_Texture*)
6. Allocate and populate `AllegroDisplay` struct
7. Set global `state.current_display` and `state.backbuffer`
8. Return display pointer

**Type Mappings:**
- `w` → `width` parameter
- `h` → `height` parameter
- Return: `AllegroDisplay*`

---

#### al_destroy_display()

**Function Signature:**
```c
void al_destroy_display(ALLEGRO_DISPLAY* display);
```

**SDL2 Equivalent:** Destroy SDL_Renderer + SDL_Window

**Implementation:**
1. Check if display is NULL, return if so
2. Destroy backbuffer bitmap (if exists)
3. Call `SDL_DestroyRenderer(display->renderer)`
4. Call `SDL_DestroyWindow(display->window)`
5. Free `AllegroDisplay` struct
6. If this was the current display, set global state to NULL

**Notes:** Order matters - destroy renderer before window.

---

#### al_get_current_display()

**Function Signature:**
```c
ALLEGRO_DISPLAY* al_get_current_display(void);
```

**SDL2 Equivalent:** Return global display pointer

**Implementation:** Return `state.current_display`

---

#### al_get_display_width()

**Function Signature:**
```c
int al_get_display_width(ALLEGRO_DISPLAY* display);
```

**SDL2 Equivalent:** `SDL_GetWindowWidth()`

**Implementation:**
1. If display is NULL, use current display
2. Return `SDL_GetWindowWidth(display->window)`

---

#### al_get_display_height()

**Function Signature:**
```c
int al_get_display_height(ALLEGRO_DISPLAY* display);
```

**SDL2 Equivalent:** `SDL_GetWindowHeight()`

**Implementation:**
1. If display is NULL, use current display
2. Return `SDL_GetWindowHeight(display->window)`

---

#### al_set_display_flag()

**Function Signature:**
```c
bool al_set_display_flag(ALLEGRO_DISPLAY* display, int flag, bool onoff);
```

**SDL2 Equivalent:** `SDL_SetWindowFullscreen()`, `SDL_SetWindowResizable()`, etc.

**Implementation:**
1. Convert Allegro flag to SDL flag
2. Handle specific flags:
   - `ALLEGRO_FULLSCREEN`: Call `SDL_SetWindowFullscreen(window, onoff ? SDL_WINDOW_FULLSCREEN : 0)`
   - `ALLEGRO_FULLSCREEN_WINDOW`: Call `SDL_SetWindowFullscreen(window, onoff ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)`
   - `ALLEGRO_RESizable`: Call `SDL_SetWindowResizable(window, onoff ? SDL_TRUE : SDL_FALSE)`
   - `ALLEGRO_MINIMIZED`: Call `SDL_MinimizeWindow(window)` / `SDL_RestoreWindow(window)`
   - `ALLEGRO_MAXIMIZED`: Call `SDL_MaximizeWindow(window)` / `SDL_RestoreWindow(window)`
3. Update display->flags
4. Return true

---

#### al_get_display_flags()

**Function Signature:**
```c
int al_get_display_flags(ALLEGRO_DISPLAY* display);
```

**SDL2 Equivalent:** Query SDL window state

**Implementation:**
1. Get current SDL window flags via `SDL_GetWindowFlags(window)`
2. Convert to Allegro flags:
   - `SDL_WINDOW_FULLSCREEN` → `ALLEGRO_FULLSCREEN`
   - `SDL_WINDOW_FULLSCREEN_DESKTOP` → `ALLEGRO_FULLSCREEN_WINDOW`
   - `SDL_WINDOW_RESIZABLE` → `ALLEGRO_RESIZABLE`
   - `SDL_WINDOW_BORDERLESS` → `ALLEGRO_FRAMELESS`
   - `SDL_WINDOW_MINIMIZED` → `ALLEGRO_MINIMIZED`
   - `SDL_WINDOW_MAXIMIZED` → `ALLEGRO_MAXIMIZED`

---

#### al_set_window_title()

**Function Signature:**
```c
void al_set_window_title(ALLEGRO_DISPLAY* display, const char* title);
```

**SDL2 Equivalent:** `SDL_SetWindowTitle()`

**Implementation:** Call `SDL_SetWindowTitle(display->window, title)`

---

#### al_resize_display()

**Function Signature:**
```c
bool al_resize_display(ALLEGRO_DISPLAY* display, int width, int height);
```

**SDL2 Equivalent:** `SDL_SetWindowSize()`

**Implementation:**
1. Call `SDL_SetWindowSize(display->window, width, height)`
2. Update display->width and display->height
3. Recreate backbuffer texture at new size
4. Return true on success

---

#### al_acknowledge_resize()

**Function Signature:**
```c
bool al_acknowledge_resize(ALLEGRO_DISPLAY* display);
```

**SDL2 Equivalent:** Handle SDL_WINDOWEVENT_RESIZED

**Implementation:**
1. Query new window size via `SDL_GetWindowSize(window)`
2. Update display dimensions
3. Recreate backbuffer texture if needed
4. Return true

---

#### al_set_window_position()

**Function Signature:**
```c
void al_set_window_position(ALLEGRO_DISPLAY* display, int x, int y);
```

**SDL2 Equivalent:** `SDL_SetWindowPosition()`

**Implementation:** Call `SDL_SetWindowPosition(display->window, x, y)`

---

#### al_get_window_position()

**Function Signature:**
```c
void al_get_window_position(ALLEGRO_DISPLAY* display, int* x, int* y);
```

**SDL2 Equivalent:** `SDL_GetWindowPosition()`

**Implementation:** Call `SDL_GetWindowPosition(display->window, x, y)`

---

### 3. Display Settings Functions

#### al_set_new_display_flags()

**Function Signature:**
```c
void al_set_new_display_flags(int flags);
```

**Implementation:** Set `state.new_display_flags = flags`

---

#### al_get_new_display_flags()

**Function Signature:**
```c
int al_get_new_display_flags(void);
```

**Implementation:** Return `state.new_display_flags`

---

#### al_set_new_display_refresh_rate()

**Function Signature:**
```c
void al_set_new_display_refresh_rate(int refresh_rate);
```

**Implementation:** Set `state.new_display_refresh_rate = refresh_rate`

---

#### al_get_new_display_refresh_rate()

**Function Signature:**
```c
int al_get_new_display_refresh_rate(void);
```

**Implementation:** Return `state.new_display_refresh_rate`

---

#### al_set_new_window_title()

**Function Signature:**
```c
void al_set_new_window_title(const char* title);
```

**Implementation:** Set `state.new_window_title = title`

---

#### al_get_new_window_title()

**Function Signature:**
```c
const char* al_get_new_window_title(void);
```

**Implementation:** Return `state.new_window_title.c_str()`

---

#### al_set_new_window_position()

**Function Signature:**
```c
void al_set_new_window_position(int x, int y);
```

**Implementation:** Set `state.new_window_x = x`, `state.new_window_y = y`

---

#### al_get_new_window_position()

**Function Signature:**
```c
void al_get_new_window_position(int* x, int* y);
```

**Implementation:** Return `state.new_window_x` and `state.new_window_y`

---

#### al_set_new_display_adapter()

**Function Signature:**
```c
void al_set_new_display_adapter(int adapter);
```

**Implementation:** Set `state.new_display_adapter = adapter` (stored for multi-monitor)

---

#### al_get_new_display_adapter()

**Function Signature:**
```c
int al_get_new_display_adapter(void);
```

**Implementation:** Return `state.new_display_adapter`

---

#### al_get_display_adapter()

**Function Signature:**
```c
int al_get_display_adapter(ALLEGRO_DISPLAY* display);
```

**Implementation:** Return display's adapter index (stored in display struct)

---

### 4. Drawing Functions

#### al_flip_display()

**Function Signature:**
```c
void al_flip_display(void);
```

**SDL2 Equivalent:** `SDL_RenderPresent()`

**Implementation:**
1. Call `SDL_RenderPresent(state.renderer)`
2. If double-buffering is emulated, swap buffers

**Notes:** SDL uses immediate mode rendering - this presents the rendered frame.

---

#### al_clear_to_color()

**Function Signature:**
```c
void al_clear_to_color(ALLEGRO_COLOR color);
```

**SDL2 Equivalent:** `SDL_SetRenderDrawColor()` + `SDL_RenderClear()`

**Implementation:**
1. Extract RGBA from ALLEGRO_COLOR
2. Call `SDL_SetRenderDrawColor(renderer, r, g, b, a)`
3. Call `SDL_RenderClear(renderer)`

**Type Mappings:**
- `ALLEGRO_COLOR.r` → `Uint8` (0-255)
- `ALLEGRO_COLOR.g` → `Uint8` (0-255)
- `ALLEGRO_COLOR.b` → `Uint8` (0-255)
- `ALLEGRO_COLOR.a` → `Uint8` (0-255)

---

#### al_hold_bitmap_drawing()

**Function Signature:**
```c
void al_hold_bitmap_drawing(bool hold);
```

**Implementation:** Set `state.bitmap_drawing_held = hold`

---

#### al_is_bitmap_drawing_held()

**Function Signature:**
```c
bool al_is_bitmap_drawing_held(void);
```

**Implementation:** Return `state.bitmap_drawing_held`

---

#### al_wait_for_vsync()

**Function Signature:**
```c
bool al_wait_for_vsync(void);
```

**Implementation:** Present with vsync (already enabled in renderer creation)

**Notes:** This is handled automatically when renderer is created with `SDL_RENDERER_PRESENTVSYNC`.

---

### 5. Target Bitmap Functions

#### al_set_target_bitmap()

**Function Signature:**
```c
void al_set_target_bitmap(ALLEGRO_BITMAP* bitmap);
```

**Implementation:**
1. Set `state.target_bitmap = bitmap`
2. If bitmap is NULL or is backbuffer, target the SDL renderer
3. Otherwise, target the bitmap's texture (for rendering to texture)

---

#### al_set_target_backbuffer()

**Function Signature:**
```c
void al_set_target_backbuffer(ALLEGRO_DISPLAY* display);
```

**Implementation:**
1. Set `state.target_bitmap = display->backbuffer`
2. Target the main SDL renderer

---

#### al_get_backbuffer()

**Function Signature:**
```c
ALLEGRO_BITMAP* al_get_backbuffer(ALLEGRO_DISPLAY* display);
```

**Implementation:** Return `display->backbuffer`

---

#### al_get_target_bitmap()

**Function Signature:**
```c
ALLEGRO_BITMAP* al_get_target_bitmap(void);
```

**Implementation:** Return `state.target_bitmap`

---

#### al_is_compatible_bitmap()

**Function Signature:**
```c
bool al_is_compatible_bitmap(ALLEGRO_BITMAP* bitmap);
```

**Implementation:** Return true if bitmap was created with current display settings

---

### 6. Display Event Source

#### al_get_display_event_source()

**Function Signature:**
```c
ALLEGRO_EVENT_SOURCE* al_get_display_event_source(ALLEGRO_DISPLAY* display);
```

**Implementation:** Return `&display->event_source`

---

#### al_set_display_icon()

**Function Signature:**
```c
void al_set_display_icon(ALLEGRO_DISPLAY* display, ALLEGRO_BITMAP* icon);
```

**SDL2 Equivalent:** `SDL_SetWindowIcon()`

**Implementation:**
1. Convert ALLEGRO_BITMAP to SDL_Surface
2. Call `SDL_SetWindowIcon(window, surface)`

---

#### al_set_display_icons()

**Function Signature:**
```c
void al_set_display_icons(ALLEGRO_DISPLAY* display, int num_icons, ALLEGRO_BITMAP* icons[]);
```

**Implementation:**
1. Convert each ALLEGRO_BITMAP to SDL_Surface
2. Call `SDL_SetWindowIcon()` with first icon (SDL2 limitation)

---

### 7. Display Options

#### al_set_new_display_option()

**Function Signature:**
```c
void al_set_new_display_option(int option, int value, int importance);
```

**Implementation:** Store in global options map (for later apply on display creation)

---

#### al_get_new_display_option()

**Function Signature:**
```c
int al_get_new_display_option(int option, int* importance);
```

**Implementation:** Return stored option value

---

#### al_reset_new_display_options()

**Function Signature:**
```c
void al_reset_new_display_options(void);
```

**Implementation:** Reset all display options to defaults

---

#### al_set_display_option()

**Function Signature:**
```c
void al_set_display_option(ALLEGRO_DISPLAY* display, int option, int value);
```

**Implementation:** Apply option to existing display (if applicable)

---

#### al_get_display_option()

**Function Signature:**
```c
int al_get_display_option(ALLEGRO_DISPLAY* display, int option);
```

**Implementation:** Return current option value for display

---

### 8. Event Queue Functions

#### al_create_event_queue()

**Function Signature:**
```c
ALLEGRO_EVENT_QUEUE* al_create_event_queue(void);
```

**SDL2 Equivalent:** Create custom event queue structure

**Implementation:**
1. Allocate `AllegroEventQueue` struct
2. Initialize `std::vector<ALLEGRO_EVENT>` for events
3. Set read_index = write_index = 0
4. Initialize registered_sources vector
5. Create SDL mutex for thread safety
6. Return queue pointer

---

#### al_destroy_event_queue()

**Function Signature:**
```c
void al_destroy_event_queue(ALLEGRO_EVENT_QUEUE* queue);
```

**SDL2 Equivalent:** Free event queue structure

**Implementation:**
1. If queue is NULL, return
2. Destroy mutex
3. Clear vectors
4. Free queue struct

---

#### al_register_event_source()

**Function Signature:**
```c
void al_register_event_source(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT_SOURCE* source);
```

**Implementation:**
1. Add source to `queue->registered_sources`
2. Mark source as registered

---

#### al_unregister_event_source()

**Function Signature:**
```c
void al_unregister_event_source(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT_SOURCE* source);
```

**Implementation:** Remove source from `queue->registered_sources`

---

#### al_is_event_source_registered()

**Function Signature:**
```c
bool al_is_event_source_registered(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT_SOURCE* source);
```

**Implementation:** Check if source is in registered_sources vector

---

#### al_pause_event_queue()

**Function Signature:**
```c
void al_pause_event_queue(ALLEGRO_EVENT_QUEUE* queue, bool pause);
```

**Implementation:** Set `queue->paused = pause`

---

#### al_is_event_queue_paused()

**Function Signature:**
```c
bool al_is_event_queue_paused(const ALLEGRO_EVENT_QUEUE* queue);
```

**Implementation:** Return `queue->paused`

---

#### al_is_event_queue_empty()

**Function Signature:**
```c
bool al_is_event_queue_empty(ALLEGRO_EVENT_QUEUE* queue);
```

**Implementation:** Return `(queue->read_index == queue->write_index)`

---

#### al_get_next_event()

**Function Signature:**
```c
bool al_get_next_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ret_event);
```

**SDL2 Equivalent:** Pop event from queue

**Implementation:**
1. If queue is paused or empty, return false
2. Get event at read_index
3. Copy to ret_event
4. Advance read_index (circular buffer)
5. Return true

---

#### al_peek_next_event()

**Function Signature:**
```c
bool al_peek_next_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ret_event);
```

**Implementation:** Same as `al_get_next_event` but don't advance read_index

---

#### al_drop_next_event()

**Function Signature:**
```c
bool al_drop_next_event(ALLEGRO_EVENT_QUEUE* queue);
```

**Implementation:** Advance read_index without copying event

---

#### al_flush_event_queue()

**Function Signature:**
```c
void al_flush_event_queue(ALLEGRO_EVENT_QUEUE* queue);
```

**Implementation:** Set read_index = write_index = 0

---

#### al_wait_for_event()

**Function Signature:**
```c
void al_wait_for_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ret_event);
```

**SDL2 Equivalent:** `SDL_WaitEvent()` + pump to queue

**Implementation:**
1. While queue is empty:
   - Pump SDL events via `SDL_PumpEvents()`
   - Poll SDL events via `SDL_PollEvent(&sdl_event)`
   - Convert SDL event to Allegro event
   - Add to queue
2. Call `al_get_next_event(queue, ret_event)`

---

#### al_wait_for_event_timed()

**Function Signature:**
```c
bool al_wait_for_event_timed(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ret_event, float secs);
```

**SDL2 Equivalent:** `SDL_WaitEventTimeout()`

**Implementation:**
1. If queue not empty, get next event and return true
2. Calculate timeout in milliseconds
3. Call `SDL_WaitEventTimeout(&sdl_event, timeout)`
4. If event received, convert and add to queue, then return true
5. Return false on timeout

**Type Mappings:**
- `secs` (float, seconds) → `timeout_ms` (int, milliseconds)

---

#### al_wait_for_event_until()

**Function Signature:**
```c
bool al_wait_for_event_until(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ret_event, ALLEGRO_TIMEOUT* timeout);
```

**Implementation:** Similar to timed version but use ALLEGRO_TIMEOUT for deadline

---

### 9. User Events

#### al_init_user_event_source()

**Function Signature:**
```c
void al_init_user_event_source(ALLEGRO_EVENT_SOURCE* source);
```

**Implementation:** Initialize event source struct

---

#### al_destroy_user_event_source()

**Function Signature:**
```c
void al_destroy_user_event_source(ALLEGRO_EVENT_SOURCE* source);
```

**Implementation:** Clean up event source

---

#### al_emit_user_event()

**Function Signature:**
```c
bool al_emit_user_event(ALLEGRO_EVENT_SOURCE* source, ALLEGRO_EVENT* event, void (*dtor)(ALLEGRO_USER_EVENT*));
```

**Implementation:**
1. Set event->user.source = source
2. Set event->user.__internal__descr with dtor
3. Add to all queues that registered this source

---

#### al_unref_user_event()

**Function Signature:**
```c
void al_unref_user_event(ALLEGRO_USER_EVENT* event);
```

**Implementation:** Decrement event reference count

---

#### al_set_event_source_data()

**Function Signature:**
```c
void al_set_event_source_data(ALLEGRO_EVENT_SOURCE* source, intptr_t data);
```

**Implementation:** Set `source->user_data = data`

---

#### al_get_event_source_data()

**Function Signature:**
```c
intptr_t al_get_event_source_data(const ALLEGRO_EVENT_SOURCE* source);
```

**Implementation:** Return `source->user_data`

---

### 10. Window Constraints

#### al_set_window_constraints()

**Function Signature:**
```c
bool al_set_window_constraints(ALLEGRO_DISPLAY* display, int min_w, int min_h, int max_w, int max_h);
```

**Implementation:** Store constraints in display struct (SDL2 doesn't support min/max directly)

---

#### al_get_window_constraints()

**Function Signature:**
```c
bool al_get_window_constraints(ALLEGRO_DISPLAY* display, int* min_w, int* min_h, int* max_w, int* max_h);
```

**Implementation:** Return stored constraints

---

#### al_apply_window_constraints()

**Function Signature:**
```c
void al_apply_window_constraints(ALLEGRO_DISPLAY* display, bool onoff);
```

**Implementation:** Enable/disable constraint enforcement

---

### 11. Deferred Drawing

#### al_acknowledge_drawing_halt()

**Function Signature:**
```c
void al_acknowledge_drawing_halt(ALLEGRO_DISPLAY* display);
```

**Implementation:** Handle display lost event (for OpenGL context loss)

---

#### al_acknowledge_drawing_resume()

**Function Signature:**
```c
void al_acknowledge_drawing_resume(ALLEGRO_DISPLAY* display);
```

**Implementation:** Handle display found event

---

### 12. Display Orientation

#### al_get_display_orientation()

**Function Signature:**
```c
int al_get_display_orientation(ALLEGRO_DISPLAY* display);
```

**SDL2 Equivalent:** `SDL_GetWindowDisplayMode()` for refresh rate

**Implementation:** Return stored orientation (SDL2 has limited support)

---

## Event Mapping

### SDL to Allegro Event Conversion

```cpp
ALLEGRO_EVENT convert_sdl_event(SDL_Event& sdl_event) {
    ALLEGRO_EVENT event;
    event.any.timestamp = SDL_GetTicks() / 1000.0;
    
    switch (sdl_event.type) {
        case SDL_QUIT:
            event.any.type = ALLEGRO_EVENT_DISPLAY_CLOSE;
            break;
            
        case SDL_WINDOWEVENT:
            switch (sdl_event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    event.display.type = ALLEGRO_EVENT_DISPLAY_RESIZE;
                    event.display.width = sdl_event.window.data1;
                    event.display.height = sdl_event.window.data2;
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    event.display.type = ALLEGRO_EVENT_DISPLAY_EXPOSE;
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    event.display.type = ALLEGRO_EVENT_DISPLAY_SWITCH_OUT;
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    event.display.type = ALLEGRO_EVENT_DISPLAY_SWITCH_IN;
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    event.display.type = ALLEGRO_EVENT_DISPLAY_CLOSE;
                    break;
            }
            break;
            
        case SDL_KEYDOWN:
            event.keyboard.type = ALLEGRO_EVENT_KEY_DOWN;
            event.keyboard.keycode = convert_sdl_keycode(sdl_event.key.keysym.sym);
            event.keyboard.repeat = (sdl_event.key.repeat != 0);
            break;
            
        case SDL_KEYUP:
            event.keyboard.type = ALLEGRO_EVENT_KEY_UP;
            event.keyboard.keycode = convert_sdl_keycode(sdl_event.key.keysym.sym);
            break;
            
        case SDL_TEXTINPUT:
            event.keyboard.type = ALLEGRO_EVENT_KEY_CHAR;
            event.keyboard.unichar = sdl_event.text.text[0];
            break;
            
        case SDL_MOUSEMOTION:
            event.mouse.type = ALLEGRO_EVENT_MOUSE_AXES;
            event.mouse.x = sdl_event.motion.x;
            event.mouse.y = sdl_event.motion.y;
            event.mouse.dx = sdl_event.motion.xrel;
            event.mouse.dy = sdl_event.motion.yrel;
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            event.mouse.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
            event.mouse.button = sdl_event.button.button;
            break;
            
        case SDL_MOUSEBUTTONUP:
            event.mouse.type = ALLEGRO_EVENT_MOUSE_BUTTON_UP;
            event.mouse.button = sdl_event.button.button;
            break;
            
        case SDL_MOUSEWHEEL:
            event.mouse.type = ALLEGRO_EVENT_MOUSE_AXES;
            event.mouse.z = sdl_event.wheel.y;
            break;
            
        case SDL_JOYAXISMOTION:
            event.joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
            event.joystick.stick = sdl_event.jaxis.axis / 2;
            event.joystick.axis = sdl_event.jaxis.axis % 2;
            event.joystick.pos = sdl_event.jaxis.value / 32767.0f;
            break;
            
        case SDL_JOYBUTTONDOWN:
            event.joystick.type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
            event.joystick.button = sdl_event.jbutton.button;
            break;
            
        case SDL_JOYBUTTONUP:
            event.joystick.type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
            event.joystick.button = sdl_event.jbutton.button;
            break;
    }
    
    return event;
}
```

### Keycode Mapping (SDL to Allegro)

```cpp
int convert_sdl_keycode(SDL_Keycode sdl_key) {
    // SDL keycodes generally match Allegro keycodes for common keys
    // Map special keys:
    switch (sdl_key) {
        case SDLK_UNKNOWN: return 0;
        case SDLK_a: return ALLEGRO_KEY_A;
        // ... map all keys
        default: return sdl_key;  // Many keys match directly
    }
}
```

---

## File Organization

```
allegro_shim/
├── include/
│   └── allegro5/
│       └── allegro.h          # Main include
├── src/
│   ├── core.cpp               # Core module implementation
│   ├── shim_state.cpp         # Global state management
│   └── ...
└── spec/
    └── core_impl.md           # This document
```

---

## Implementation Order

1. **Phase 1: Core Display**
   - `al_init()`
   - `al_create_display()` / `al_destroy_display()`
   - `al_get_current_display()`
   - `al_flip_display()` / `al_clear_to_color()`
   - Display dimension functions

2. **Phase 2: Event Queue**
   - `al_create_event_queue()` / `al_destroy_event_queue()`
   - `al_register_event_source()`
   - Event polling functions

3. **Phase 3: Display Features**
   - Window management (title, position, resize)
   - Display flags
   - Target bitmap

4. **Phase 4: Advanced**
   - User events
   - Display options
   - Window constraints

---

## Notes

- SDL2 uses immediate-mode rendering vs Allegro's display-linked backbuffer
- Event timestamps in SDL are in milliseconds, Allegro uses seconds (double)
- SDL_PollEvent must be called to pump the event loop
- The backbuffer is represented as an ALLEGRO_BITMAP wrapping an SDL_Texture
- Display flags map imperfectly; some features may not have direct SDL2 equivalents

---

## New Details from SDL Source Analysis

### SDL2, Not SDL3

The repository contains **SDL2** (not SDL3). Key differences from my earlier analysis:

1. **Event System**: SDL2 event system:
   - `SDL_QUIT` - Application quit
   - `SDL_WINDOWEVENT_*` - Window events (resized, exposed, etc.)
   - `SDL_KEYDOWN` / `SDL_KEYUP` - Keyboard events
   - `SDL_MOUSEMOTION` / `SDL_MOUSEBUTTON_*` / `SDL_MOUSEWHEEL` - Mouse events
   - `SDL_JOY*` / `SDL_CONTROLLER_*` - Joystick/gamepad events

2. **Display/Video**: SDL2 includes:
   - `SDL_video.h` - Window management, display handling
   - `SDL_render.h` - 2D rendering API (texture, primitives)
   - `SDL_surface.h` - Pixel surface management

3. **Rendering Features** (from SDL2 render.h):
   - Points, lines, rectangles, textures
   - Renderer backends: Direct3D, Direct3D11, Metal, OpenGL, OpenGLES2, software

4. **Input Features**:
   - `SDL_gamepad.h` - Game controller API (SDL2.0.4+)
   - `SDL_joystick.h` - Joystick API
   - `SDL_keyboard.h` / `SDL_mouse.h` - Keyboard/mouse

### SDL2 Key Headers

| Header | Purpose |
|--------|---------|
| SDL2/SDL.h | Main include |
| SDL2/SDL_video.h | Window/display |
| SDL2/SDL_render.h | 2D rendering |
| SDL2/SDL_events.h | Event handling |
| SDL2/SDL_thread.h | Threading |
| SDL2/SDL_mutex.h | Synchronization |
| SDL2/SDL_filesystem.h | File paths |
