# Allegro Shim Specification

## Overview

This document specifies the design and implementation of an Allegro 5 compatibility shim that maps Allegro API calls to SDL2, enabling ZQuest Classic to compile and run on platforms that support SDL2 but not Allegro (such as PortMaster devices).

## Goals

1. **Binary compatibility** - Allow existing ZQuest code to compile with minimal changes
2. **Feature parity** - Provide equivalent functionality to Allegro 5 where possible
3. **Incremental implementation** - Can be built and tested piece by piece
4. **Maintainability** - Clear separation between shim and native code

## Non-Goals

- Full Allegro 4 compatibility (handled separately if needed)
- Perfect 1:1 API equivalence where SDL2 provides better alternatives
- Performance optimization (initial implementation favors correctness)

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ZQuest Classic Code                       │
│  (calls allegro5/allegro.h, uses key[] from allegro.h)      │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│              Allegro Shim Layer (allegro_shim/)              │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Header Files (allegro5/*.h) - API declarations         │ │
│  │  - Provide Allegro-compatible function signatures       │ │
│  │  - #define aliases for types and constants              │ │
│  └─────────────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Implementation Files (*.cpp) - SDL2 backend            │ │
│  │  - Implements Allegro functions using SDL2             │ │
│  │  - Manages SDL_Window, SDL_Renderer, SDL_Texture       │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                        SDL2 + Addons                         │
│  (SDL2, SDL2_image, SDL2_mixer, SDL2_ttf, SDL2_gfx)         │
└─────────────────────────────────────────────────────────────┘
```

---

## Module Structure

### 1. Core Module (`core.cpp`)

**Responsibilities:**
- Display/window management
- Event queue system
- Global state management
- Thread initialization

**Key Functions:**
- `al_init()` - Initialize all subsystems
- `al_create_display()` / `al_destroy_display()`
- `al_get_display()` - Get global display
- `al_create_event_queue()` / `al_destroy_event_queue()`
- `al_register_event_source()`
- `al_flip_display()` / `al_clear_to_color()`

### 2. Graphics Module (`graphics.cpp`)

**Responsibilities:**
- Bitmap creation and management
- Drawing primitives
- Color management
- Font rendering

**Key Functions:**
- `al_create_bitmap()` / `al_destroy_bitmap()`
- `al_set_target_bitmap()` / `al_get_target_bitmap()`
- `al_draw_bitmap()` / `al_draw_scaled_bitmap()`
- `al_draw_filled_rectangle()` / `al_draw_line()` / `al_draw_circle()`
- `al_map_rgb()` / `al_map_rgba()` / `al_premul_rgba()`
- `al_draw_text()` / `al_get_text_width()`
- `al_load_bitmap()` / `al_save_bitmap()`

### 3. Audio Module (`audio.cpp`)

**Responsibilities:**
- Audio device initialization
- Sample management
- Audio streaming
- Mixer control

**Key Functions:**
- `al_install_audio()` / `al_init_acodec_addon()`
- `al_create_sample()` / `al_destroy_sample()`
- `al_create_sample_instance()` / `al_destroy_sample_instance()`
- `al_attach_sample_instance_to_mixer()`
- `al_set_sample_instance_playing()` / `al_set_sample_instance_gain()`
- `al_load_audio_stream()` / `al_destroy_audio_stream()`

### 4. Input Module (`input.cpp`)

**Responsibilities:**
- Keyboard state tracking
- Mouse state tracking
- Joystick support

**Key Functions:**
- `install_keyboard()` / `install_mouse()` / `install_joystick()`
- Keyboard polling (maintains `key[]` array for legacy code)
- `al_get_mouse_state()` / `al_mouse_button_down()`

### 5. Threading Module (`thread.cpp`)

**Responsibilities:**
- Mutex operations
- Condition variables
- Thread creation

**Key Functions:**
- `al_create_mutex()` / `al_destroy_mutex()`
- `al_lock_mutex()` / `al_unlock_mutex()`
- `al_create_cond()` / `al_destroy_cond()`
- `al_wait_cond()` / `al_signal_cond()`

### 6. File I/O Module (`file.cpp`)

**Responsibilities:**
- Configuration files
- File system operations
- Memory files

**Key Functions:**
- `al_create_config()` / `al_load_config_file()`
- `al_get_config_value()` / `al_set_config_value()`
- `al_fopen()` / `al_open_memfile()`

### 7. Addons Module (`addons.cpp`)

**Responsibilities:**
- Initialize various addons
- Image loading
- Font loading
- Primitives

**Key Functions:**
- `al_init_image_addon()`
- `al_init_font_addon()`
- `al_init_primitives_addon()`
- `al_init_ttf_addon()`

---

## Type Mappings

### Core Types

| Allegro Type | SDL2 Equivalent | Notes |
|--------------|------------------|-------|
| `ALLEGRO_DISPLAY` | `SDL_Window*` + `SDL_Renderer*` | Window + rendering context |
| `ALLEGRO_BITMAP` | `SDL_Texture*` | Texture for rendering |
| `ALLEGRO_EVENT_QUEUE` | `SDL_Event*` + custom queue | Event management |
| `ALLEGRO_EVENT` | `SDL_Event` | Event structure |
| `ALLEGRO_COLOR` | `SDL_Color` | RGBA color |
| `ALLEGRO_MUTEX` | `SDL_mutex*` | Mutex |
| `ALLEGRO_COND` | `SDL_cond*` | Condition variable |
| `ALLEGRO_SAMPLE` | `Mix_Chunk*` | Audio sample |
| `ALLEGRO_SAMPLE_INSTANCE` | Custom struct | Sample with playback state |
| `ALLEGRO_AUDIO_STREAM` | Custom struct | Streaming audio |
| `ALLEGRO_CONFIG` | Custom struct | Configuration |
| `ALLEGRO_FS_ENTRY` | `DIR*` / `struct stat` | File system |
| `ALLEGRO_FONT` | `TTF_Font*` | Font |

### Color Functions

| Allegro Function | SDL2 Equivalent |
|------------------|------------------|
| `al_map_rgb(r,g,b)` | `{r,g,b,255}` |
| `al_map_rgba(r,g,b,a)` | `{r,g,b,a}` |
| `al_premul_rgba_f(r,g,b,a)` | Pre-multiply alpha |
| `al_map_rgb_f(r,g,b)` | `{r*255, g*255, b*255, 255}` |

### Drawing Primitives

| Allegro Function | SDL2 Equivalent |
|------------------|------------------|
| `al_draw_filled_rectangle(x1,y1,x2,y2,color)` | SDL_RenderFillRect |
| `al_draw_line(x1,y1,x2,y2,color,thickness)` | SDL_RenderDrawLine (with thickness workaround) |
| `al_draw_circle(x,y,r,color,thickness)` | SDL_RenderDrawPoint or custom |
| `al_draw_rectangle(x1,y1,x2,y2,color,thickness)` | SDL_RenderDrawRect |

---

## Global State Management

The shim must maintain global state for functions like `al_get_display()`:

```cpp
// Global state structure
struct AllegroShimState {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Renderer* current_target = nullptr;
    
    // Input state
    Uint8 key[512];           // Legacy key[] array
    Uint32 mouse_buttons = 0;
    int mouse_x = 0, mouse_y = 0;
    
    // Audio
    Mix_Mixer* mixer = nullptr;
    
    // Fonts
    std::unordered_map<int, TTF_Font*> fonts;
    
    // Threading
    SDL_mutex* init_mutex = nullptr;
    bool initialized = false;
};
```

---

## Implementation Priority

### Phase 1: Core Rendering (Critical Path)
1. Display creation/destruction
2. Bitmap management
3. Drawing primitives
4. Color functions
5. Target bitmap management

### Phase 2: Input (Gameplay Critical)
1. Keyboard state (`key[]` array)
2. Mouse state
3. Event queue basics

### Phase 3: Audio (Gameplay Critical)
1. Audio initialization
2. Sample playback
3. Streaming audio

### Phase 4: Addons
1. Image addon (PNG/JPEG loading)
2. Font addon
3. Primitives addon

### Phase 5: Extended Features
1. Configuration files
2. Threading
3. Native dialogs
4. Full event handling

---

## Build Configuration

### CMake Integration

```cmake
# allegro_shim/CMakeLists.txt
add_library(allegro_shim STATIC
    core.cpp
    graphics.cpp
    audio.cpp
    input.cpp
    thread.cpp
    file.cpp
    addons.cpp
)

target_link_libraries(allegro_shim
    SDL2::SDL2
    SDL2::SDL2_image
    SDL2::SDL2_mixer
    SDL2::SDL2_ttf
    SDL2::SDL2_gfx
)

target_include_directories(allegro_shim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

### Header Organization

```
allegro_shim/
├── include/
│   └── allegro5/
│       ├── allegro.h          # Main header (includes all others)
│       ├── allegro_base.h    # Base types and constants
│       ├── allegro_display.h
│       ├── allegro_bitmap.h
│       ├── allegro_color.h
│       ├── allegro_draw.h
│       ├── allegro_events.h
│       ├── allegro_audio.h
│       ├── allegro_keyboard.h
│       ├── allegro_mouse.h
│       ├── allegro_joystick.h
│       ├── allegro_timer.h
│       ├── allegro_config.h
│       ├── allegro_file.h
│       ├── allegro_font.h
│       ├── allegro_image.h
│       ├── allegro_primitives.h
│       ├── allegro_native_dialog.h
│       └──allegro_ttf.h
└── src/
    ├── core.cpp
    ├── graphics.cpp
    ├── audio.cpp
    ├── input.cpp
    ├── thread.cpp
    ├── file.cpp
    ├── addons.cpp
    └── shim_state.cpp
```

---

## Testing Strategy

### Unit Tests
- Test each function in isolation where possible
- Verify color mapping correctness
- Verify coordinate transformations

### Integration Tests
- Create minimal ZQuest-like test program
- Test display initialization and rendering
- Test audio playback
- Test keyboard/mouse input

### Compatibility Tests
- Compare output with native Allegro (where available)
- Verify exact pixel outputs for drawing primitives

---

## Known Limitations

1. **Performance** - SDL2 may have different performance characteristics
2. **Some advanced features** - Not all Allegro addons have SDL2 equivalents
3. **Thread safety** - SDL2 threading differs from Allegro
4. **Display modes** - Some display flags may not map directly
5. **Joystick** - SDL2 joystick API differs significantly

---

## Future Considerations

- **GPU acceleration** - SDL2_gpu for better performance
- **Vulkan** - Future-proofing with Vulkan backend
- **Direct3D** - Windows-specific backend option
- **EGL** - Embedded device support

---

## References

- [Allegro 5 API Reference](https://liballeg.org/a5docs/5.2.9/)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [SDL2_image](https://github.com/libsdl-org/SDL_image)
- [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer)
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf)
