# Allegro to SDL2 Implementation Plan

This document details the implementation plan for each module, mapping Allegro 5 functions to their SDL2 equivalents.

---

## 1. Core Module (`core.cpp`)

### Functions to Implement

| Allegro Function | SDL2 Equivalent | Implementation Notes |
|------------------|------------------|----------------------|
| `al_init()` | `SDL_Init()` + custom | Call SDL_Init with required subsystems (VIDEO, TIMER, EVENT). Initialize shim global state. |
| `al_create_display(w, h)` | `SDL_CreateWindow() + SDL_CreateRenderer()` | Create window with SDL_WINDOW_SHOWN. Create renderer with SDL_RENDERER_ACCELERATED \| SDL_RENDERER_PRESENTVSYNC. Store in shim state. |
| `al_destroy_display(display)` | `SDL_DestroyRenderer() + SDL_DestroyWindow()` | Clean up renderer first, then window. Clear shim state pointers. |
| `al_get_current_display()` | Access shim global `state.window` | Return stored window pointer from shim state. |
| `al_create_event_queue()` | Custom struct + `std::queue<SDL_Event>` | Create queue wrapper struct. Initialize event list. |
| `al_destroy_event_queue(queue)` | Delete queue struct | Unregister all event sources, free queue. |
| `al_register_event_source(queue, source)` | Add source to queue's source list | Store event source pointer, enable events. |
| `al_flip_display()` | `SDL_RenderPresent(renderer)` | Call SDL_RenderPresent on current renderer. |
| `al_clear_to_color(color)` | `SDL_SetRenderDrawColor() + SDL_RenderClear()` | Convert ALLEGRO_COLOR to SDL_Color, call SDL_RenderClear. |

### Types to Define

```cpp
struct AllegroDisplay {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width, height;
};

struct AllegroEventQueue {
    std::vector<ALLEGRO_EVENT_SOURCE*> sources;
    std::queue<SDL_Event> events;
    bool built;
};

struct ALLEGRO_EVENT_SOURCE {
    // Simplified - just stores enabled callback
};
```

### Header Files to Create
- `allegro_shim/include/allegro5/system.h` - al_init macro
- `allegro_shim/include/allegro5/display.h` - Display functions
- `allegro_shim/include/allegro5/events.h` - Event queue and event types

---

## 2. Graphics Module (`graphics.cpp`)

### Functions to Implement

| Allegro Function | SDL2 Equivalent | Implementation Notes |
|------------------|------------------|----------------------|
| `al_create_bitmap(w, h)` | `SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h)` | Create texture as render target. Store dimensions. |
| `al_destroy_bitmap(bitmap)` | `SDL_DestroyTexture(texture)` | Free texture resources. |
| `al_set_target_bitmap(bitmap)` | `SDL_SetRenderTarget(renderer, texture)` | If bitmap is NULL, set to display backbuffer. Store current target. |
| `al_get_target_bitmap()` | Return stored target | Return currently set target bitmap from thread-local state. |
| `al_draw_bitmap(bmp, x, y, flags)` | `SDL_RenderCopy(renderer, texture, NULL, &dest)` | Calculate dest rect from x,y and texture size. |
| `al_draw_scaled_bitmap(src, sx, sy, sw, sh, dx, dy, dw, dh, flags)` | `SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect)` | Create SDL_Rect for source/dest regions. |
| `al_draw_filled_rectangle(x1, y1, x2, y2, color)` | `SDL_Rect + SDL_SetRenderDrawColor + SDL_RenderFillRect` | Convert color, create rect, fill. |
| `al_draw_line(x1, y1, x2, y2, color, thickness)` | `SDL_SetRenderDrawColor + SDL_RenderDrawLine` | SDL2 doesn't support thickness - draw multiple lines for thickness > 1. |
| `al_draw_circle(cx, cy, r, color, thickness)` | Custom Bresenham or SDL_RenderDrawPoint loop | Implement circle algorithm, draw outline only. |
| `al_draw_filled_circle(cx, cy, r, color)` | Custom scanline algorithm | Fill circle using scanlines. |
| `al_map_rgb(r, g, b)` | `{r, g, b, 255}` | Return struct with float or uint8_t. Allegro uses float (0.0-1.0) but functions take uint8_t. |
| `al_map_rgba(r, g, b, a)` | `{r, g, b, a}` | Direct mapping. |
| `al_premul_rgba(r, g, b, a)` | `{r*a/255, g*a/255, b*a/255, a}` | Pre-multiply alpha. |
| `al_map_rgb_f(r, g, b)` | `{r*255, g*255, b*255, 255}` | Float to uint8 conversion. |
| `al_map_rgba_f(r, g, b, a)` | `{r*255, g*255, b*255, a*255}` | Float to uint8 conversion. |
| `al_draw_text(font, color, x, y, flags, text)` | `SDL_ttf: TTF_RenderUTF8_Blended + SDL_RenderCopy` | Render text to surface, convert to texture, draw, free temp surfaces. |
| `al_get_text_width(font, text)` | `TTF_SizeUTF8(font, text, &w, NULL)` | Get text width in pixels. |
| `al_load_bitmap(filename)` | `SDL_image: IMG_Load(filename)` | Load via SDL_image, convert to texture. |
| `al_save_bitmap(filename, bitmap)` | `SDL_image: IMG_SavePNG` | Save texture pixels to file. |

### Types to Define

```cpp
struct ALLEGRO_COLOR {
    float r, g, b, a;  // 0.0-1.0 range like Allegro
};

struct AllegroBitmap {
    SDL_Texture* texture;
    int width, height;
    bool is_backbuffer;
};
```

### Header Files to Create
- `allegro_shim/include/allegro5/bitmap.h` - Bitmap types and functions
- `allegro_shim/include/allegro5/color.h` - Color functions
- `allegro_shim/include/allegro5/drawing.h` - Basic drawing
- `allegro_shim/include/allegro5/bitmap_draw.h` - Bitmap drawing

---

## 3. Audio Module (`audio.cpp`)

### Functions to Implement

| Allegro Function | SDL2 Equivalent | Implementation Notes |
|------------------|------------------|----------------------|
| `al_install_audio()` | `Mix_OpenAudio(frequency, format, channels, chunksize)` | Initialize SDL_mixer. Call Mix_OpenAudio with default or specified params. |
| `al_init_acodec_addon()` | No-op (SDL_mixer handles this) | SDL_mixer auto-detects formats. Just set flag. |
| `al_create_sample(data, samples, freq, depth, conf, free_buf)` | `Mix_Chunk` + custom struct | Create struct wrapping Mix_Chunk. Store audio parameters. |
| `al_destroy_sample(sample)` | Free buffer if free_buf, free struct | Clean up sample data. |
| `al_create_sample_instance(sample)` | Custom struct | Create instance struct with reference to sample, state. playback |
| `al_destroy_sample_instance(instance)` | Free instance struct | Clean up instance. |
| `al_attach_sample_instance_to_mixer(instance, mixer)` | `Mix_PlayChannel(-1, chunk, loops)` | Start playback, store channel. |
| `al_set_sample_instance_playing(instance, play)` | `Mix_Pause(channel) / Mix_Resume(channel)` | Control playback. |
| `al_set_sample_instance_gain(instance, gain)` | `Mix_Volume(channel, (int)(gain * MIX_MAX_VOLUME))` | Set volume. |
| `al_load_audio_stream(filename, buffer_samples, freq, depth, conf)` | `Mix_Music` + streaming | Load music file, create streaming struct. |
| `al_destroy_audio_stream(stream)` | `Mix_FreeMusic()` | Free music data. |

### Types to Define

```cpp
struct AllegroSample {
    Uint8* buffer;
    Uint32 samples;
    ALLEGRO_AUDIO_DEPTH depth;
    ALLEGRO_CHANNEL_CONF conf;
    int frequency;
    bool free_buf;
    Mix_Chunk chunk;  // or just use chunk data
};

struct AllegroSampleInstance {
    AllegroSample* sample;
    float speed;
    float gain;
    float pan;
    ALLEGRO_LOOP_MODES loop;
    int channel;  // SDL_mixer channel
    bool playing;
};

struct AllegroAudioStream {
    Mix_Music* music;
    // streaming buffer management
};
```

### Header Files to Create
- `allegro_shim/include/allegro5/allegro_audio.h` - Audio types and functions

---

## 4. Input Module (`input.cpp`)

### Functions to Implement

| Allegro Function | SDL2 Equivalent | Implementation Notes |
|------------------|------------------|----------------------|
| `al_install_keyboard()` | `SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1")` | Just set flag indicating keyboard init. |
| `install_mouse()` | Enable mouse events via SDL | Set mouse capture mode if needed. |
| `install_joystick()` | `SDL_GameControllerOpen(0)` | Initialize joystick subsystem. |
| `al_get_keyboard_state()` | Access shim global `state.key[]` | Return pointer to key state array. |
| `al_key_down(state, keycode)` | `(state[keycode / 8] >> (keycode % 8)) & 1` | Check bit in key state. |
| `al_get_mouse_state()` | Fill ALLEGRO_MOUSE_STATE from shim state | Copy mouse position/buttons from SDL_Event state. |
| `al_mouse_button_down(state, button)` | `(state.buttons & (1 << (button-1)))` | Check button bit. |

### Key Array Mapping (Critical)

The `key[]` array from legacy code uses Allegro key codes. Map to SDL scancodes:

```cpp
// Global key state array - 512 bytes like Allegro
Uint8 key[512];

// Key code mapping table needed
// SDL_SCANCODE_* to ALLEGRO_KEY_* conversion
int SDLToAllegroKey[SDL_NUM_SCANCODES];
```

### Types to Define

```cpp
struct ALLEGRO_KEYBOARD_STATE {
    Uint8 key[ALLEGRO_KEY_MAX / 8 + 1];  // Bitmap
};

struct ALLEGRO_MOUSE_STATE {
    int x, y, z, w;
    Uint32 buttons;
    float pressure;
    ALLEGRO_DISPLAY* display;
};
```

### Header Files to Create
- `allegro_shim/include/allegro5/keyboard.h` - Keyboard functions
- `allegro_shim/include/allegro5/mouse.h` - Mouse functions
- `allegro_shim/include/allegro5/joystick.h` - Joystick functions

---

## 5. Threading Module (`thread.cpp`)

### Functions to Implement

| Allegro Function | SDL2 Equivalent | Implementation Notes |
|------------------|------------------|----------------------|
| `al_create_mutex()` | `SDL_CreateMutex()` | Simple wrapper. |
| `al_destroy_mutex(mutex)` | `SDL_DestroyMutex()` | Wrapper. |
| `al_lock_mutex(mutex)` | `SDL_LockMutex()` | Wrapper. |
| `al_unlock_mutex(mutex)` | `SDL_UnlockMutex()` | Wrapper. |
| `al_create_mutex_recursive()` | `SDL_CreateMutex()` (non-recursive) | Note: SDL mutex is not recursive. Consider using different approach if recursive needed. |
| `al_create_cond()` | `SDL_CreateCond()` | Wrapper. |
| `al_destroy_cond(cond)` | `SDL_DestroyCond()` | Wrapper. |
| `al_wait_cond(cond, mutex)` | `SDL_CondWait(cond, mutex)` | Wrapper. |
| `al_signal_cond(cond)` | `SDL_CondSignal(cond)` | Wake one waiter. |
| `al_broadcast_cond(cond)` | `SDL_CondBroadcast(cond)` | Wake all waiters. |
| `al_create_thread(proc, arg)` | `SDL_CreateThread()` | Create thread, don't start yet. |
| `al_start_thread(thread)` | N/A (Allegro separates create/start) | For SDL, combine into one call or store thread for later start. |
| `al_join_thread(thread, ret)` | `SDL_WaitThread()` | Wait for thread to finish. |
| `al_destroy_thread(thread)` | `SDL_DestroyThread()` or cleanup | Free thread resources. |

### Types to Define

```cpp
struct ALLEGRO_MUTEX {
    SDL_mutex* mutex;
    bool recursive;
    int lock_count;
    Uint32 owner_thread;
};

struct ALLEGRO_COND {
    SDL_cond* cond;
};

struct ALLEGRO_THREAD {
    SDL_Thread* thread;
    void (*proc)(ALLEGRO_THREAD*, void*);
    void* arg;
    bool started;
};
```

### Header Files to Create
- `allegro_shim/include/allegro5/threads.h` - Threading functions

---

## 6. File I/O Module (`file.cpp`)

### Functions to Implement

| Allegro Function | SDL2 Equivalent | Implementation Notes |
|------------------|------------------|----------------------|
| `al_create_config()` | Custom struct | Create new config with empty sections tree. |
| `al_load_config_file(filename)` | `fopen/fread` or custom parser | Parse INI-style file into config struct. |
| `al_save_config_file(config, filename)` | `fopen/fwrite` or custom writer | Write config to file in INI format. |
| `al_get_config_value(config, section, key, def)` | Look up in internal map | Return value or default. |
| `al_set_config_value(config, section, key, value)` | Insert into internal map | Add or update key-value. |
| `al_fopen(filename, mode)` | `fopen(filename, mode)` | Standard file open, wrap in ALLEGRO_FILE. |
| `al_open_memfile(data, size, mode)` | Custom implementation | Create file struct that reads from memory buffer. |

### Config Format

Allegro config uses INI-like format. Can implement with:
- `std::map<std::string, std::map<std::string, std::string>>`
- Or use INI parsing library

### Types to Define

```cpp
struct AllegroConfig {
    std::map<std::string, std::map<std::string, std::string>> sections;
    std::string filename;
};

struct AllegroFile {
    FILE* fp;  // or custom buffer for memfile
    bool is_memfile;
    Uint8* mem_buffer;
    size_t mem_size;
    size_t mem_pos;
};
```

### Header Files to Create
- `allegro_shim/include/allegro5/config.h` - Config functions
- `allegro_shim/include/allegro5/file.h` - File I/O functions

---

## 7. Addons Module (`addons.cpp`)

### Functions to Implement

| Allegro Function | SDL2 Equivalent | Implementation Notes |
|------------------|------------------|----------------------|
| `al_init_image_addon()` | `IMG_Init(IMG_INIT_PNG \| IMG_INIT_JPG \| IMG_INIT_TIF \| IMG_INIT_WEBP \| IMG_INIT_GIF)` | Initialize SDL_image with supported formats. |
| `al_init_font_addon()` | No-op | SDL_ttf doesn't need explicit init. Just set flag. |
| `al_init_primitives_addon()` | No-op | Drawing primitives implemented in graphics.cpp. Just set flag. |
| `al_init_ttf_addon()` | `TTF_Init()` | Initialize SDL_ttf. |
| `al_load_font(filename, size, flags)` | `TTF_OpenFont(filename, size)` | Load TTF font. |
| `al_load_ttf_font(filename, size, flags)` | `TTF_OpenFont(filename, size)` | Same as al_load_font. |
| `al_destroy_font(font)` | `TTF_CloseFont(font)` | Close font. |
| `al_load_bitmap(filename)` | `IMG_Load(filename)` | Use SDL_image. |
| `al_save_bitmap(filename, bitmap)` | `IMG_SavePNG(..., filename)` | Use SDL_image. |

### Header Files to Create
- `allegro_shim/include/allegro5/allegro_image.h` - Image addon
- `allegro_shim/include/allegro5/allegro_font.h` - Font addon
- `allegro_shim/include/allegro5/allegro_primitives.h` - Primitives addon
- `allegro_shim/include/allegro5/allegro_ttf.h` - TTF addon

---

## Global State Structure

```cpp
struct AllegroShimState {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    AllegroBitmap* current_target = nullptr;
    
    // Input state
    Uint8 key[512];           // Legacy key[] array
    Uint32 mouse_buttons = 0;
    int mouse_x = 0, mouse_y = 0;
    
    // Audio
    bool audio_initialized = false;
    int audio_frequency = 22050;
    Uint16 audio_format = AUDIO_S16SYS;
    int audio_channels = 2;
    int audio_chunksize = 1024;
    
    // Fonts
    std::unordered_map<void*, TTF_Font*> fonts;
    
    // Threading
    SDL_mutex* init_mutex = nullptr;
    bool initialized = false;
};

extern AllegroShimState g_state;
```

---

## Implementation Order (Priority)

### Phase 1: Core Rendering
1. `al_init()` / display functions
2. `al_clear_to_color()` / `al_flip_display()`
3. `al_create_bitmap()` / `al_destroy_bitmap()`
4. `al_set_target_bitmap()` / `al_get_target_bitmap()`
5. `al_draw_bitmap()` / `al_draw_scaled_bitmap()`

### Phase 2: Colors and Primitives
1. Color mapping functions
2. `al_draw_filled_rectangle()`
3. Line, circle, triangle primitives

### Phase 3: Input
1. Keyboard init and `key[]` array
2. Mouse state
3. Event queue basics

### Phase 4: Audio
1. Audio init
2. Sample playback
3. Streaming

### Phase 5: Addons
1. Image addon
2. Font/TTF addon

### Phase 6: File/Config and Threading
1. Config files
2. Threading primitives

---

## Build Configuration

Add to project CMakeLists.txt:

```cmake
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_link_libraries(allegro_shim
    SDL2::SDL2
    SDL2::SDL2_image
    SDL2::SDL2_mixer
    SDL2::SDL2_ttf
)
```

---

## Testing Checklist

- [ ] Display creates with correct size
- [ ] Clearing and flipping works
- [ ] Bitmap creation and destruction
- [ ] Drawing bitmaps at positions
- [ ] Drawing scaled bitmaps
- [ ] Color mapping functions produce correct values
- [ ] Drawing primitives render correctly
- [ ] Keyboard state updates from SDL events
- [ ] `key[]` array reflects key presses
- [ ] Mouse position updates
- [ ] Mouse button state
- [ ] Audio plays sample
- [ ] Volume control works
- [ ] Music streaming works
- [ ] Config file load/save
- [ ] Font loading and text rendering
- [ ] Image loading
