// allegro_shim - Allegro 5 to SDL2 shim layer

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include <allegro5/allegro_display.h>
#include <allegro5/allegro_keyboard.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_bitmap.h>
#include <allegro5/allegro_draw.h>
#include <allegro5/allegro_state.h>
#include <allegro5/allegro_transform.h>
#include <allegro5/allegro_blender.h>
#include <allegro5/allegro_events.h>
#include <allegro5/allegro_mouse.h>
#include <allegro5/allegro_joystick.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_timer.h>
#include <allegro5/internal/allegro_audio.h>
#include <allegro5/internal/allegro_config.h>
#include <allegro5/internal/allegro_display.h>
#include <allegro5/internal/allegro_joystick.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>

static ALLEGRO_DISPLAY* _current_display = nullptr;
static int _new_display_flags = 0;
static int _new_display_refresh_rate = 60;
static const char* _new_window_title = "Allegro SDL2 Shim";
static int _new_window_x = -1;
static int _new_window_y = -1;
static int _new_display_adapter = 0;

struct ALLEGRO_BITMAP {
    SDL_Texture* texture;
    SDL_Surface* surface;
    int width;
    int height;
    int format;
    int flags;
    bool is_backbuffer;
};

static ALLEGRO_BITMAP* _target_bitmap = nullptr;
static int _new_bitmap_flags = ALLEGRO_VIDEO_BITMAP;
static int _new_bitmap_format = ALLEGRO_PIXEL_FORMAT_ARGB_8888;

static float _clip_x = 0;
static float _clip_y = 0;
static float _clip_w = 0;
static float _clip_h = 0;
static bool _clipping_initialized = false;

static bool _keyboard_installed = false;
static Uint8 _key[512] = {0};
static unsigned int _key_down_bits[(ALLEGRO_KEY_MAX + 31) / 32] = {0};

static bool _mouse_installed = false;
static int _mouse_x = 0;
static int _mouse_y = 0;
static int _mouse_z = 0;
static int _mouse_w = 0;
static int _mouse_buttons = 0;
static unsigned int _mouse_num_buttons = 3;
static unsigned int _mouse_num_axes = 2;

static bool _joystick_installed = false;
static std::vector<ALLEGRO_JOYSTICK*> _joysticks;

static bool _audio_installed = false;
static int _audio_reserved_channels = 0;
static ALLEGRO_MIXER* _default_mixer = nullptr;

struct ALLEGRO_FILE {
    FILE* fp;
    bool close_on_destroy;
};

struct AllegroSampleInstance {
    Mix_Chunk* chunk;
    int channel;
    bool is_playing;
    ALLEGRO_PLAYMODE loop;
    float gain;
    float pan;
    float speed;
    unsigned int position;
    ALLEGRO_SAMPLE* sample;
};

struct AllegroAudioStream {
    Mix_Music* music;
    bool is_playing;
    ALLEGRO_PLAYMODE loop;
    float gain;
    float pan;
    float speed;
    unsigned int frequency;
    ALLEGRO_AUDIO_DEPTH depth;
    ALLEGRO_CHANNEL_CONF channels;
    unsigned int buffer_samples;
    size_t buffer_count;
};

struct MixerWrapper {
    int frequency;
    ALLEGRO_AUDIO_DEPTH depth;
    ALLEGRO_CHANNEL_CONF channels;
    ALLEGRO_MIXER_QUALITY quality;
    ALLEGRO_SAMPLE_INSTANCE* attached_sample_instance;
    ALLEGRO_AUDIO_STREAM* attached_audio_stream;
};

ALLEGRO_DISPLAY* al_create_display(int w, int h)
{
    ALLEGRO_DISPLAY* display = new ALLEGRO_DISPLAY;
    if (!display) {
        return nullptr;
    }
    
    Uint32 window_flags = SDL_WINDOW_SHOWN;
    
    if (_new_display_flags & ALLEGRO_FULLSCREEN) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    } else if (_new_display_flags & ALLEGRO_FULLSCREEN_WINDOW) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    if (_new_display_flags & ALLEGRO_RESIZABLE) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    
    if (_new_display_flags & ALLEGRO_FRAMELESS) {
        window_flags |= SDL_WINDOW_BORDERLESS;
    }
    
    if (_new_window_x >= 0 && _new_window_y >= 0) {
        display->window = SDL_CreateWindow(
            _new_window_title,
            _new_window_x, _new_window_y,
            w, h,
            window_flags
        );
    } else {
        display->window = SDL_CreateWindow(
            _new_window_title,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            w, h,
            window_flags
        );
    }
    
    if (!display->window) {
        delete display;
        return nullptr;
    }
    
    display->renderer = SDL_CreateRenderer(
        display->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!display->renderer) {
        SDL_DestroyWindow(display->window);
        delete display;
        return nullptr;
    }
    
    display->width = w;
    display->height = h;
    display->flags = _new_display_flags;
    display->refresh_rate = _new_display_refresh_rate;
    display->backbuffer = nullptr;
    
    _current_display = display;
    
    return display;
}

void al_destroy_display(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        return;
    }
    
    if (display->renderer) {
        SDL_DestroyRenderer(display->renderer);
    }
    
    if (display->window) {
        SDL_DestroyWindow(display->window);
    }
    
    if (_current_display == display) {
        _current_display = nullptr;
    }
    
    delete display;
}

ALLEGRO_DISPLAY* al_get_current_display(void)
{
    return _current_display;
}

void al_set_current_display(ALLEGRO_DISPLAY* display)
{
    _current_display = display;
}

int al_get_display_width(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        display = _current_display;
    }
    if (!display) {
        return 0;
    }
    return display->width;
}

int al_get_display_height(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        display = _current_display;
    }
    if (!display) {
        return 0;
    }
    return display->height;
}

int al_get_display_flags(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        display = _current_display;
    }
    if (!display) {
        return 0;
    }
    
    int flags = display->flags;
    Uint32 sdl_flags = SDL_GetWindowFlags(display->window);
    
    if (sdl_flags & SDL_WINDOW_FULLSCREEN) {
        flags |= ALLEGRO_FULLSCREEN;
    } else if (sdl_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        flags |= ALLEGRO_FULLSCREEN_WINDOW;
    }
    
    if (sdl_flags & SDL_WINDOW_RESIZABLE) {
        flags |= ALLEGRO_RESIZABLE;
    }
    
    if (sdl_flags & SDL_WINDOW_BORDERLESS) {
        flags |= ALLEGRO_FRAMELESS;
    }
    
    if (sdl_flags & SDL_WINDOW_MINIMIZED) {
        flags |= ALLEGRO_MINIMIZED;
    }
    
    if (sdl_flags & SDL_WINDOW_MAXIMIZED) {
        flags |= ALLEGRO_MAXIMIZED;
    }
    
    return flags;
}

bool al_set_display_flag(ALLEGRO_DISPLAY* display, int flag, bool onoff)
{
    if (!display) {
        display = _current_display;
    }
    if (!display) {
        return false;
    }
    
    switch (flag) {
        case ALLEGRO_FULLSCREEN:
            SDL_SetWindowFullscreen(display->window, 
                onoff ? SDL_WINDOW_FULLSCREEN : 0);
            break;
        case ALLEGRO_FULLSCREEN_WINDOW:
            SDL_SetWindowFullscreen(display->window, 
                onoff ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
            break;
        case ALLEGRO_RESIZABLE:
            SDL_SetWindowResizable(display->window, 
                onoff ? SDL_TRUE : SDL_FALSE);
            break;
        case ALLEGRO_MINIMIZED:
            if (onoff) {
                SDL_MinimizeWindow(display->window);
            } else {
                SDL_RestoreWindow(display->window);
            }
            break;
        case ALLEGRO_MAXIMIZED:
            if (onoff) {
                SDL_MaximizeWindow(display->window);
            } else {
                SDL_RestoreWindow(display->window);
            }
            break;
        default:
            return false;
    }
    
    if (onoff) {
        display->flags |= flag;
    } else {
        display->flags &= ~flag;
    }
    
    return true;
}

void al_set_window_title(ALLEGRO_DISPLAY* display, const char* title)
{
    if (!display) {
        display = _current_display;
    }
    if (!display || !display->window) {
        return;
    }
    SDL_SetWindowTitle(display->window, title);
}

bool al_resize_display(ALLEGRO_DISPLAY* display, int width, int height)
{
    if (!display) {
        display = _current_display;
    }
    if (!display) {
        return false;
    }
    
    SDL_SetWindowSize(display->window, width, height);
    display->width = width;
    display->height = height;
    
    return true;
}

bool al_acknowledge_resize(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        display = _current_display;
    }
    if (!display) {
        return false;
    }
    
    int w, h;
    SDL_GetWindowSize(display->window, &w, &h);
    display->width = w;
    display->height = h;
    
    return true;
}

void al_set_window_position(ALLEGRO_DISPLAY* display, int x, int y)
{
    if (!display) {
        display = _current_display;
    }
    if (!display || !display->window) {
        return;
    }
    SDL_SetWindowPosition(display->window, x, y);
}

void al_get_window_position(ALLEGRO_DISPLAY* display, int* x, int* y)
{
    if (!display) {
        display = _current_display;
    }
    if (!display || !display->window) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }
    SDL_GetWindowPosition(display->window, x, y);
}

void al_flip_display(void)
{
    if (_current_display && _current_display->renderer) {
        SDL_RenderPresent(_current_display->renderer);
    }
}

void al_clear_to_color(ALLEGRO_COLOR color)
{
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_SetRenderDrawColor(
        _current_display->renderer,
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    );
    SDL_RenderClear(_current_display->renderer);
}

void al_set_new_display_flags(int flags)
{
    _new_display_flags = flags;
}

int al_get_new_display_flags(void)
{
    return _new_display_flags;
}

void al_set_new_display_refresh_rate(int refresh_rate)
{
    _new_display_refresh_rate = refresh_rate;
}

int al_get_new_display_refresh_rate(void)
{
    return _new_display_refresh_rate;
}

void al_set_new_window_title(const char* title)
{
    _new_window_title = title;
}

const char* al_get_new_window_title(void)
{
    return _new_window_title;
}

void al_set_new_window_position(int x, int y)
{
    _new_window_x = x;
    _new_window_y = y;
}

void al_get_new_window_position(int* x, int* y)
{
    if (x) *x = _new_window_x;
    if (y) *y = _new_window_y;
}

void al_set_new_display_adapter(int adapter)
{
    _new_display_adapter = adapter;
}

int al_get_new_display_adapter(void)
{
    return _new_display_adapter;
}

int al_get_display_adapter(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        display = _current_display;
    }
    return _new_display_adapter;
}

static bool _bitmap_drawing_held = false;

void al_hold_bitmap_drawing(bool hold)
{
    _bitmap_drawing_held = hold;
}

bool al_is_bitmap_drawing_held(void)
{
    return _bitmap_drawing_held;
}

ALLEGRO_COLOR al_map_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    ALLEGRO_COLOR color;
    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = 1.0f;
    return color;
}

ALLEGRO_COLOR al_map_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    ALLEGRO_COLOR color;
    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = a / 255.0f;
    return color;
}

ALLEGRO_COLOR al_map_rgb_f(float r, float g, float b)
{
    ALLEGRO_COLOR color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 1.0f;
    return color;
}

ALLEGRO_COLOR al_map_rgba_f(float r, float g, float b, float a)
{
    ALLEGRO_COLOR color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

ALLEGRO_COLOR al_premul_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    float alpha = a / 255.0f;
    ALLEGRO_COLOR color;
    color.r = (r / 255.0f) * alpha;
    color.g = (g / 255.0f) * alpha;
    color.b = (b / 255.0f) * alpha;
    color.a = alpha;
    return color;
}

ALLEGRO_COLOR al_premul_rgba_f(float r, float g, float b, float a)
{
    ALLEGRO_COLOR color;
    color.r = r * a;
    color.g = g * a;
    color.b = b * a;
    color.a = a;
    return color;
}

void al_unmap_rgb(ALLEGRO_COLOR color, uint8_t* r, uint8_t* g, uint8_t* b)
{
    if (r) *r = (uint8_t)(color.r * 255);
    if (g) *g = (uint8_t)(color.g * 255);
    if (b) *b = (uint8_t)(color.b * 255);
}

void al_unmap_rgba(ALLEGRO_COLOR color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a)
{
    if (r) *r = (uint8_t)(color.r * 255);
    if (g) *g = (uint8_t)(color.g * 255);
    if (b) *b = (uint8_t)(color.b * 255);
    if (a) *a = (uint8_t)(color.a * 255);
}

void al_unmap_rgb_f(ALLEGRO_COLOR color, float* r, float* g, float* b)
{
    if (r) *r = color.r;
    if (g) *g = color.g;
    if (b) *b = color.b;
}

void al_unmap_rgba_f(ALLEGRO_COLOR color, float* r, float* g, float* b, float* a)
{
    if (r) *r = color.r;
    if (g) *g = color.g;
    if (b) *b = color.b;
    if (a) *a = color.a;
}

struct ColorNameEntry {
    const char* name;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static const ColorNameEntry _color_names[] = {
    {"black", 0, 0, 0},
    {"white", 255, 255, 255},
    {"red", 255, 0, 0},
    {"green", 0, 255, 0},
    {"blue", 0, 0, 255},
    {"yellow", 255, 255, 0},
    {"cyan", 0, 255, 255},
    {"magenta", 255, 0, 255},
    {"aqua", 0, 255, 255},
    {"azure", 0, 127, 255},
    {"beige", 245, 245, 220},
    {"bisque", 255, 228, 196},
    {"blanchedalmond", 255, 235, 205},
    {"blueviolet", 138, 43, 226},
    {"brown", 165, 42, 42},
    {"burlywood", 222, 184, 135},
    {"cadetblue", 95, 158, 160},
    {"chartreuse", 127, 255, 0},
    {"chocolate", 210, 105, 30},
    {"coral", 255, 127, 80},
    {"cornflowerblue", 100, 149, 237},
    {"cornsilk", 255, 248, 220},
    {"crimson", 220, 20, 60},
    {"darkblue", 0, 0, 139},
    {"darkcyan", 0, 139, 139},
    {"darkgoldenrod", 184, 134, 11},
    {"darkgray", 169, 169, 169},
    {"darkgreen", 0, 100, 0},
    {"darkkhaki", 189, 183, 107},
    {"darkmagenta", 139, 0, 139},
    {"darkolivegreen", 85, 107, 47},
    {"darkorange", 255, 140, 0},
    {"darkorchid", 153, 50, 204},
    {"darkred", 139, 0, 0},
    {"darksalmon", 233, 150, 122},
    {"darkseagreen", 143, 188, 143},
    {"darkslateblue", 72, 61, 139},
    {"darkslategray", 47, 79, 79},
    {"darkturquoise", 0, 206, 209},
    {"darkviolet", 148, 0, 211},
    {"deeppink", 255, 20, 147},
    {"deepskyblue", 0, 191, 255},
    {"dimgray", 105, 105, 105},
    {"dodgerblue", 30, 144, 255},
    {"firebrick", 178, 34, 34},
    {"floralwhite", 255, 250, 240},
    {"forestgreen", 34, 139, 34},
    {"fuchsia", 255, 0, 255},
    {"gainsboro", 220, 220, 220},
    {"ghostwhite", 248, 248, 255},
    {"gold", 255, 215, 0},
    {"goldenrod", 218, 165, 32},
    {"gray", 128, 128, 128},
    {"greenyellow", 173, 255, 47},
    {"honeydew", 240, 255, 240},
    {"hotpink", 255, 105, 180},
    {"indianred", 205, 92, 92},
    {"indigo", 75, 0, 130},
    {"ivory", 255, 255, 240},
    {"khaki", 240, 230, 140},
    {"lavender", 230, 230, 250},
    {"lavenderblush", 255, 240, 245},
    {"lawngreen", 124, 252, 0},
    {"lemonchiffon", 255, 250, 205},
    {"lightblue", 173, 216, 230},
    {"lightcoral", 240, 128, 128},
    {"lightcyan", 224, 255, 255},
    {"lightgoldenrodyellow", 250, 250, 210},
    {"lightgray", 211, 211, 211},
    {"lightgreen", 144, 238, 144},
    {"lightpink", 255, 182, 193},
    {"lightsalmon", 255, 160, 122},
    {"lightseagreen", 32, 178, 170},
    {"lightskyblue", 135, 206, 250},
    {"lightslategray", 119, 136, 153},
    {"lightsteelblue", 176, 196, 222},
    {"lightyellow", 255, 255, 224},
    {"lime", 0, 255, 0},
    {"limegreen", 50, 205, 50},
    {"linen", 250, 240, 230},
    {"maroon", 128, 0, 0},
    {"mediumaquamarine", 102, 205, 170},
    {"mediumblue", 0, 0, 205},
    {"mediumorchid", 186, 85, 211},
    {"mediumpurple", 147, 112, 219},
    {"mediumseagreen", 60, 179, 113},
    {"mediumslateblue", 123, 104, 238},
    {"mediumspringgreen", 0, 250, 154},
    {"mediumturquoise", 72, 209, 204},
    {"mediumvioletred", 199, 21, 133},
    {"midnightblue", 25, 25, 112},
    {"mintcream", 245, 255, 250},
    {"mistyrose", 255, 228, 225},
    {"moccasin", 255, 228, 181},
    {"navajowhite", 255, 222, 173},
    {"navy", 0, 0, 128},
    {"oldlace", 253, 245, 230},
    {"olive", 128, 128, 0},
    {"olivedrab", 107, 142, 35},
    {"orange", 255, 165, 0},
    {"orangered", 255, 69, 0},
    {"orchid", 218, 112, 214},
    {"palegoldenrod", 238, 232, 170},
    {"palegreen", 152, 251, 152},
    {"paleturquoise", 175, 238, 238},
    {"palevioletred", 219, 112, 147},
    {"papayawhip", 255, 239, 213},
    {"peachpuff", 255, 218, 185},
    {"peru", 205, 133, 63},
    {"pink", 255, 192, 203},
    {"plum", 221, 160, 221},
    {"powderblue", 176, 224, 230},
    {"purple", 128, 0, 128},
    {"rosybrown", 188, 143, 143},
    {"royalblue", 65, 105, 225},
    {"saddlebrown", 139, 69, 19},
    {"salmon", 250, 128, 114},
    {"sandybrown", 244, 164, 96},
    {"seagreen", 46, 139, 87},
    {"seashell", 255, 245, 238},
    {"sienna", 160, 82, 45},
    {"silver", 192, 192, 192},
    {"skyblue", 135, 206, 235},
    {"slateblue", 106, 90, 205},
    {"slategray", 112, 128, 144},
    {"snow", 255, 250, 250},
    {"springgreen", 0, 255, 127},
    {"steelblue", 70, 130, 180},
    {"tan", 210, 180, 140},
    {"teal", 0, 128, 128},
    {"thistle", 216, 191, 216},
    {"tomato", 255, 99, 71},
    {"turquoise", 64, 224, 208},
    {"violet", 238, 130, 238},
    {"wheat", 245, 222, 179},
    {"whitesmoke", 245, 245, 245},
    {"yellowgreen", 154, 205, 50},
};

bool al_color_name_to_rgb(const char* name, ALLEGRO_COLOR* color)
{
    if (!name || !color) {
        return false;
    }

    for (const auto& entry : _color_names) {
        if (strcasecmp(name, entry.name) == 0) {
            color->r = entry.r / 255.0f;
            color->g = entry.g / 255.0f;
            color->b = entry.b / 255.0f;
            color->a = 1.0f;
            return true;
        }
    }

    return false;
}

ALLEGRO_BITMAP* al_create_bitmap(int w, int h)
{
    ALLEGRO_BITMAP* bitmap = new ALLEGRO_BITMAP;
    if (!bitmap) {
        return nullptr;
    }
    
    bitmap->width = w;
    bitmap->height = h;
    bitmap->format = _new_bitmap_format;
    bitmap->flags = _new_bitmap_flags;
    bitmap->is_backbuffer = false;
    bitmap->surface = nullptr;
    bitmap->texture = nullptr;
    
    if (_current_display && _current_display->renderer) {
        bitmap->texture = SDL_CreateTexture(
            _current_display->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET,
            w, h
        );
    }
    
    if (!bitmap->texture) {
        bitmap->surface = SDL_CreateRGBSurface(
            0, w, h, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000
        );
    }
    
    return bitmap;
}

void al_destroy_bitmap(ALLEGRO_BITMAP* bitmap)
{
    if (!bitmap) {
        return;
    }
    
    if (bitmap->texture) {
        SDL_DestroyTexture(bitmap->texture);
    }
    
    if (bitmap->surface) {
        SDL_FreeSurface(bitmap->surface);
    }
    
    delete bitmap;
}

int al_get_bitmap_width(ALLEGRO_BITMAP* bitmap)
{
    if (!bitmap) {
        return 0;
    }
    return bitmap->width;
}

int al_get_bitmap_height(ALLEGRO_BITMAP* bitmap)
{
    if (!bitmap) {
        return 0;
    }
    return bitmap->height;
}

int al_get_bitmap_format(ALLEGRO_BITMAP* bitmap)
{
    if (!bitmap) {
        return 0;
    }
    return bitmap->format;
}

int al_get_bitmap_flags(ALLEGRO_BITMAP* bitmap)
{
    if (!bitmap) {
        return 0;
    }
    return bitmap->flags;
}

void al_set_target_bitmap(ALLEGRO_BITMAP* bitmap)
{
    _target_bitmap = bitmap;
    
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    if (bitmap && bitmap->texture) {
        SDL_SetRenderTarget(_current_display->renderer, bitmap->texture);
    } else {
        SDL_SetRenderTarget(_current_display->renderer, nullptr);
    }
}

ALLEGRO_BITMAP* al_get_target_bitmap(void)
{
    return _target_bitmap;
}

void al_set_new_bitmap_flags(int flags)
{
    _new_bitmap_flags = flags;
}

int al_get_new_bitmap_flags(void)
{
    return _new_bitmap_flags;
}

void al_set_new_bitmap_format(int format)
{
    _new_bitmap_format = format;
}

int al_get_new_bitmap_format(void)
{
    return _new_bitmap_format;
}

bool al_is_compatible_bitmap(ALLEGRO_BITMAP* bitmap)
{
    if (!bitmap || !_current_display) {
        return false;
    }
    
    return (bitmap->flags & _new_bitmap_flags) != 0 &&
           bitmap->format == _new_bitmap_format;
}

ALLEGRO_BITMAP* al_clone_bitmap(ALLEGRO_BITMAP* source)
{
    if (!source) {
        return nullptr;
    }
    
    ALLEGRO_BITMAP* bitmap = al_create_bitmap(source->width, source->height);
    if (!bitmap) {
        return nullptr;
    }
    
    if (source->texture && _current_display && _current_display->renderer) {
        SDL_SetRenderTarget(_current_display->renderer, bitmap->texture);
        SDL_RenderCopy(_current_display->renderer, source->texture, nullptr, nullptr);
        SDL_SetRenderTarget(_current_display->renderer, nullptr);
    }
    
    return bitmap;
}

void al_convert_bitmap(ALLEGRO_BITMAP* bitmap)
{
    (void)bitmap;
}

ALLEGRO_BITMAP* al_get_backbuffer(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        display = _current_display;
    }
    if (!display) {
        return nullptr;
    }
    
    if (!display->backbuffer) {
        display->backbuffer = al_create_bitmap(display->width, display->height);
        if (display->backbuffer) {
            static ALLEGRO_BITMAP* backbuffer_bitmap = static_cast<ALLEGRO_BITMAP*>(display->backbuffer);
            backbuffer_bitmap->is_backbuffer = true;
        }
    }
    
    return static_cast<ALLEGRO_BITMAP*>(display->backbuffer);
}

void al_set_target_backbuffer(ALLEGRO_DISPLAY* display)
{
    if (!display) {
        display = _current_display;
    }
    
    _target_bitmap = nullptr;
    
    if (display && display->renderer) {
        SDL_SetRenderTarget(display->renderer, nullptr);
    }
}

void al_draw_bitmap(ALLEGRO_BITMAP* bitmap, float dx, float dy, int flags)
{
    al_draw_bitmap_region(bitmap, 0, 0, 
        static_cast<float>(al_get_bitmap_width(bitmap)),
        static_cast<float>(al_get_bitmap_height(bitmap)),
        dx, dy, flags);
}

void al_draw_bitmap_region(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh, float dx, float dy, int flags)
{
    if (!bitmap || !_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_Rect src_rect = {static_cast<int>(sx), static_cast<int>(sy), 
                         static_cast<int>(sw), static_cast<int>(sh)};
    SDL_Rect dst_rect = {static_cast<int>(dx), static_cast<int>(dy), 
                         static_cast<int>(sw), static_cast<int>(sh)};
    
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flags & ALLEGRO_FLIP_HORIZONTAL) {
        flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    }
    if (flags & ALLEGRO_FLIP_VERTICAL) {
        flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
    }
    
    SDL_RenderCopyEx(_current_display->renderer, bitmap->texture, 
                     &src_rect, &dst_rect, 0, nullptr, flip);
}

void al_draw_scaled_bitmap(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh, 
                          float dx, float dy, float dw, float dh, int flags)
{
    if (!bitmap || !_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_Rect src_rect = {static_cast<int>(sx), static_cast<int>(sy), 
                         static_cast<int>(sw), static_cast<int>(sh)};
    SDL_Rect dst_rect = {static_cast<int>(dx), static_cast<int>(dy), 
                         static_cast<int>(dw), static_cast<int>(dh)};
    
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flags & ALLEGRO_FLIP_HORIZONTAL) {
        flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    }
    if (flags & ALLEGRO_FLIP_VERTICAL) {
        flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
    }
    
    SDL_RenderCopyEx(_current_display->renderer, bitmap->texture, 
                     &src_rect, &dst_rect, 0, nullptr, flip);
}

void al_draw_tinted_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float dx, float dy, int flags)
{
    if (!bitmap || !_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_SetTextureColorMod(bitmap->texture, 
        static_cast<Uint8>(tint.r * 255),
        static_cast<Uint8>(tint.g * 255),
        static_cast<Uint8>(tint.b * 255));
    SDL_SetTextureAlphaMod(bitmap->texture, static_cast<Uint8>(tint.a * 255));
    
    al_draw_bitmap(bitmap, dx, dy, flags);
    
    SDL_SetTextureColorMod(bitmap->texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(bitmap->texture, 255);
}

void al_draw_tinted_scaled_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, 
                                  float sx, float sy, float sw, float sh, 
                                  float dx, float dy, float dw, float dh, int flags)
{
    if (!bitmap || !_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_SetTextureColorMod(bitmap->texture, 
        static_cast<Uint8>(tint.r * 255),
        static_cast<Uint8>(tint.g * 255),
        static_cast<Uint8>(tint.b * 255));
    SDL_SetTextureAlphaMod(bitmap->texture, static_cast<Uint8>(tint.a * 255));
    
    al_draw_scaled_bitmap(bitmap, sx, sy, sw, sh, dx, dy, dw, dh, flags);
    
    SDL_SetTextureColorMod(bitmap->texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(bitmap->texture, 255);
}

void al_put_pixel(float x, float y, ALLEGRO_COLOR color)
{
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_SetRenderDrawColor(
        _current_display->renderer,
        static_cast<Uint8>(color.r * 255),
        static_cast<Uint8>(color.g * 255),
        static_cast<Uint8>(color.b * 255),
        static_cast<Uint8>(color.a * 255)
    );
    SDL_RenderDrawPoint(_current_display->renderer, static_cast<int>(x), static_cast<int>(y));
}

void al_put_blended_pixel(float x, float y, ALLEGRO_COLOR color)
{
    al_put_pixel(x, y, color);
}

void al_get_pixel(ALLEGRO_BITMAP* bitmap, float x, float y, ALLEGRO_COLOR* color)
{
    if (!color) {
        return;
    }
    
    color->r = 0;
    color->g = 0;
    color->b = 0;
    color->a = 1;
    
    if (!bitmap || !bitmap->surface) {
        return;
    }
    
    int px = static_cast<int>(x);
    int py = static_cast<int>(y);
    
    if (px < 0 || px >= bitmap->width || py < 0 || py >= bitmap->height) {
        return;
    }
    
    Uint32* pixels = static_cast<Uint32*>(bitmap->surface->pixels);
    Uint32 pixel = pixels[py * bitmap->surface->w + px];
    
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, bitmap->surface->format, &r, &g, &b, &a);
    
    color->r = r / 255.0f;
    color->g = g / 255.0f;
    color->b = b / 255.0f;
    color->a = a / 255.0f;
}

void al_set_clipping_rectangle(float x, float y, float w, float h)
{
    _clip_x = x;
    _clip_y = y;
    _clip_w = w;
    _clip_h = h;
    _clipping_initialized = true;
    
    if (_current_display && _current_display->renderer) {
        SDL_Rect rect = {static_cast<int>(x), static_cast<int>(y), 
                         static_cast<int>(w), static_cast<int>(h)};
        SDL_RenderSetClipRect(_current_display->renderer, &rect);
    }
}

void al_get_clipping_rectangle(float* x, float* y, float* w, float* h)
{
    if (!_clipping_initialized) {
        if (_current_display) {
            if (x) *x = 0;
            if (y) *y = 0;
            if (w) *w = _current_display->width;
            if (h) *h = _current_display->height;
        } else {
            if (x) *x = 0;
            if (y) *y = 0;
            if (w) *w = 0;
            if (h) *h = 0;
        }
        return;
    }
    
    if (x) *x = _clip_x;
    if (y) *y = _clip_y;
    if (w) *w = _clip_w;
    if (h) *h = _clip_h;
}

void al_reset_clipping_rectangle(void)
{
    _clipping_initialized = false;
    
    if (_current_display && _current_display->renderer) {
        SDL_RenderSetClipRect(_current_display->renderer, nullptr);
    }
}

static void _set_render_color(ALLEGRO_COLOR color)
{
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    SDL_SetRenderDrawColor(
        _current_display->renderer,
        static_cast<Uint8>(color.r * 255),
        static_cast<Uint8>(color.g * 255),
        static_cast<Uint8>(color.b * 255),
        static_cast<Uint8>(color.a * 255)
    );
}

void al_draw_filled_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color)
{
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    _set_render_color(color);
    
    int ix1 = static_cast<int>(x1);
    int iy1 = static_cast<int>(y1);
    int ix2 = static_cast<int>(x2);
    int iy2 = static_cast<int>(y2);
    
    SDL_Rect rect = {ix1, iy1, ix2 - ix1, iy2 - iy1};
    SDL_RenderFillRect(_current_display->renderer, &rect);
}

void al_draw_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness)
{
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    _set_render_color(color);
    
    int ix1 = static_cast<int>(x1);
    int iy1 = static_cast<int>(y1);
    int ix2 = static_cast<int>(x2);
    int iy2 = static_cast<int>(y2);
    
    SDL_Rect rect = {ix1, iy1, ix2 - ix1, iy2 - iy1};
    SDL_RenderDrawRect(_current_display->renderer, &rect);
}

void al_draw_line(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness)
{
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    _set_render_color(color);
    
    SDL_RenderDrawLine(_current_display->renderer,
        static_cast<int>(x1), static_cast<int>(y1),
        static_cast<int>(x2), static_cast<int>(y2));
}

void al_draw_circle(float cx, float cy, float r, ALLEGRO_COLOR color, float thickness)
{
    if (!_current_display || !_current_display->renderer || r <= 0) {
        return;
    }
    
    _set_render_color(color);
    
    const int segments = static_cast<int>(r * 6.28);
    const int steps = segments > 0 ? segments : 20;
    
    for (int i = 0; i < steps; i++) {
        float angle1 = (static_cast<float>(i) / steps) * 6.28318f;
        float angle2 = (static_cast<float>(i + 1) / steps) * 6.28318f;
        
        float x1 = cx + cosf(angle1) * r;
        float y1 = cy + sinf(angle1) * r;
        float x2 = cx + cosf(angle2) * r;
        float y2 = cy + sinf(angle2) * r;
        
        SDL_RenderDrawLine(_current_display->renderer,
            static_cast<int>(x1), static_cast<int>(y1),
            static_cast<int>(x2), static_cast<int>(y2));
    }
}

void al_draw_filled_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color)
{
    if (!_current_display || !_current_display->renderer || rx <= 0 || ry <= 0) {
        return;
    }
    
    _set_render_color(color);
    
    const int segments = static_cast<int>((rx + ry) * 3.14f);
    const int steps = segments > 0 ? segments : 20;
    
    for (int i = 0; i < steps; i++) {
        float angle1 = (static_cast<float>(i) / steps) * 6.28318f;
        float angle2 = (static_cast<float>(i + 1) / steps) * 6.28318f;
        
        SDL_Vertex verts[3] = {
            {cx, cy, 0, 0, 0, 0},
            {cx + cosf(angle1) * rx, cy + sinf(angle1) * ry, 0, 0, 0, 0},
            {cx + cosf(angle2) * rx, cy + sinf(angle2) * ry, 0, 0, 0, 0}
        };
        SDL_RenderGeometry(_current_display->renderer, nullptr, verts, 3, nullptr, 0);
    }
}

void al_draw_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color, float thickness)
{
    if (!_current_display || !_current_display->renderer || rx <= 0 || ry <= 0) {
        return;
    }
    
    _set_render_color(color);
    
    const int segments = static_cast<int>((rx + ry) * 3.14f);
    const int steps = segments > 0 ? segments : 20;
    
    for (int i = 0; i < steps; i++) {
        float angle1 = (static_cast<float>(i) / steps) * 6.28318f;
        float angle2 = (static_cast<float>(i + 1) / steps) * 6.28318f;
        
        float x1 = cx + cosf(angle1) * rx;
        float y1 = cy + sinf(angle1) * ry;
        float x2 = cx + cosf(angle2) * rx;
        float y2 = cy + sinf(angle2) * ry;
        
        SDL_RenderDrawLine(_current_display->renderer,
            static_cast<int>(x1), static_cast<int>(y1),
            static_cast<int>(x2), static_cast<int>(y2));
    }
}

void al_draw_arc(float cx, float cy, float r, float start_angle, float delta_angle, ALLEGRO_COLOR color, float thickness)
{
    if (!_current_display || !_current_display->renderer || r <= 0) {
        return;
    }
    
    _set_render_color(color);
    
    float end_angle = start_angle + delta_angle;
    const int segments = static_cast<int>(r * fabs(delta_angle));
    const int steps = segments > 2 ? segments : 10;
    
    for (int i = 0; i < steps; i++) {
        float angle1 = start_angle + (static_cast<float>(i) / steps) * delta_angle;
        float angle2 = start_angle + (static_cast<float>(i + 1) / steps) * delta_angle;
        
        float x1 = cx + cosf(angle1) * r;
        float y1 = cy + sinf(angle1) * r;
        float x2 = cx + cosf(angle2) * r;
        float y2 = cy + sinf(angle2) * r;
        
        SDL_RenderDrawLine(_current_display->renderer,
            static_cast<int>(x1), static_cast<int>(y1),
            static_cast<int>(x2), static_cast<int>(y2));
    }
}

void al_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR color, float thickness)
{
    _set_render_color(color);
    
    SDL_RenderDrawLine(_current_display->renderer, static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2));
    SDL_RenderDrawLine(_current_display->renderer, static_cast<int>(x2), static_cast<int>(y2), static_cast<int>(x3), static_cast<int>(y3));
    SDL_RenderDrawLine(_current_display->renderer, static_cast<int>(x3), static_cast<int>(y3), static_cast<int>(x1), static_cast<int>(y1));
}

void al_draw_filled_triangle(float x1, float y1, float x2, float y2, float x3, float y3, ALLEGRO_COLOR color)
{
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_Vertex verts[3] = {
        {x1, y1, 0, 0, 0, 0},
        {x2, y2, 0, 0, 0, 0},
        {x3, y3, 0, 0, 0, 0}
    };
    SDL_RenderGeometry(_current_display->renderer, nullptr, verts, 3, nullptr, 0);
}

void al_draw_polygon(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color, float thickness)
{
    if (!vertices || vertex_count < 3) {
        return;
    }
    
    _set_render_color(color);
    
    for (int i = 0; i < vertex_count; i++) {
        const float* v1 = reinterpret_cast<const float*>(reinterpret_cast<const char*>(vertices) + i * stride);
        const float* v2 = reinterpret_cast<const float*>(reinterpret_cast<const char*>(vertices) + ((i + 1) % vertex_count) * stride);
        
        SDL_RenderDrawLine(_current_display->renderer,
            static_cast<int>(v1[0]), static_cast<int>(v1[1]),
            static_cast<int>(v2[0]), static_cast<int>(v2[1]));
    }
}

void al_draw_filled_polygon(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color)
{
    if (!vertices || vertex_count < 3) {
        return;
    }
    
    if (!_current_display || !_current_display->renderer) {
        return;
    }
    
    SDL_Vertex* verts = new SDL_Vertex[vertex_count];
    int* indices = new int[vertex_count];
    
    for (int i = 0; i < vertex_count; i++) {
        const float* v = reinterpret_cast<const float*>(reinterpret_cast<const char*>(vertices) + i * stride);
        verts[i] = {v[0], v[1], 0, 0, 0, 0};
        indices[i] = i;
    }
    
    SDL_RenderGeometry(_current_display->renderer, nullptr, verts, vertex_count, indices, vertex_count);
    
    delete[] verts;
    delete[] indices;
}

void al_draw_polyline(const float* vertices, int vertex_count, int stride, ALLEGRO_COLOR color, float thickness, bool closed)
{
    if (!vertices || vertex_count < 2) {
        return;
    }
    
    _set_render_color(color);
    
    for (int i = 0; i < vertex_count - 1; i++) {
        const float* v1 = reinterpret_cast<const float*>(reinterpret_cast<const char*>(vertices) + i * stride);
        const float* v2 = reinterpret_cast<const float*>(reinterpret_cast<const char*>(vertices) + (i + 1) * stride);
        
        SDL_RenderDrawLine(_current_display->renderer,
            static_cast<int>(v1[0]), static_cast<int>(v1[1]),
            static_cast<int>(v2[0]), static_cast<int>(v2[1]));
    }
    
    if (closed && vertex_count > 2) {
        const float* v1 = reinterpret_cast<const float*>(reinterpret_cast<const char*>(vertices) + (vertex_count - 1) * stride);
        const float* v2 = reinterpret_cast<const float*>(reinterpret_cast<const char*>(vertices));
        
        SDL_RenderDrawLine(_current_display->renderer,
            static_cast<int>(v1[0]), static_cast<int>(v1[1]),
            static_cast<int>(v2[0]), static_cast<int>(v2[1]));
    }
}

void al_draw_pixel(float x, float y, ALLEGRO_COLOR color)
{
    al_put_pixel(x, y, color);
}

bool al_install_keyboard(void)
{
    if (_keyboard_installed) {
        return true;
    }
    _keyboard_installed = true;
    memset(_key, 0, sizeof(_key));
    memset(_key_down_bits, 0, sizeof(_key_down_bits));
    return true;
}

void al_uninstall_keyboard(void)
{
    _keyboard_installed = false;
    memset(_key, 0, sizeof(_key));
    memset(_key_down_bits, 0, sizeof(_key_down_bits));
}

bool al_is_keyboard_installed(void)
{
    return _keyboard_installed;
}

void al_get_keyboard_state(ALLEGRO_KEYBOARD_STATE* ret_state)
{
    if (!ret_state) {
        return;
    }
    
    const Uint8* sdl_keyboard = SDL_GetKeyboardState(NULL);
    
    memset(ret_state->__key_down__internal__, 0, sizeof(ret_state->__key_down__internal__));
    
    for (int i = 0; i < SDL_NUM_SCANCODES && i < 512; i++) {
        if (sdl_keyboard[i]) {
            _key[i] = 1;
            _key_down_bits[i / 32] |= (1 << (i % 32));
        } else {
            _key[i] = 0;
            _key_down_bits[i / 32] &= ~(1 << (i % 32));
        }
    }
    
    memcpy(ret_state->__key_down__internal__, _key_down_bits, sizeof(_key_down_bits));
}

bool al_key_down(const ALLEGRO_KEYBOARD_STATE* state, int keycode)
{
    if (!state || keycode < 0 || keycode > ALLEGRO_KEY_MAX) {
        return false;
    }
    return (state->__key_down__internal__[keycode / 32] & (1 << (keycode % 32))) != 0;
}

static const char* _keycode_names[] = {
    "UNKNOWN", "ESCAPE", "1", "2", "3", "4", "5", "6", "7", "8",
    "9", "0", "MINUS", "EQUALS", "BACKSPACE", "TAB", "Q", "W", "E", "R",
    "T", "Y", "U", "I", "O", "P", "OPENBRACE", "CLOSEBRACE", "ENTER", "LCTRL",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", "SEMICOLON",
    "QUOTE", "TILDE", "LSHIFT", "BACKSLASH", "Z", "X", "C", "V", "B", "N",
    "M", "COMMA", "FULLSTOP", "SLASH", "RSHIFT", "PAD_ASTERISK", "LALT", "SPACE", "CAPSLOCK", "F1",
    "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK",
    "SCROLLLOCK", "PAD_7", "PAD_8", "PAD_9", "PAD_MINUS", "PAD_4", "PAD_5", "PAD_6", "PAD_PLUS", "PAD_1",
    "PAD_2", "PAD_3", "PAD_0", "PAD_DELETE", "F11", "F12", "PAD_ENTER", "RCTRL", "PAD_SLASH", "ALTGR",
    "PAUSE", "HOME", "UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT",
    "DELETE", "LWIN", "RWIN", "MENU"
};

const char* al_keycode_to_name(int keycode)
{
    if (keycode >= 0 && keycode <= ALLEGRO_KEY_MAX && keycode < (int)(sizeof(_keycode_names) / sizeof(_keycode_names[0]))) {
        return _keycode_names[keycode];
    }
    return "UNKNOWN";
}

bool al_can_set_keyboard_leds(void)
{
    return false;
}

bool al_set_keyboard_leds(int leds)
{
    (void)leds;
    return false;
}

void* al_get_keyboard_event_source(void)
{
    return nullptr;
}

static int _blender_op = ALLEGRO_ADD;
static int _blender_src = ALLEGRO_ALPHA;
static int _blender_dst = ALLEGRO_INVERSE_ALPHA;
static int _blender_alpha_op = ALLEGRO_ADD;
static int _blender_alpha_src = ALLEGRO_ALPHA;
static int _blender_alpha_dst = ALLEGRO_INVERSE_ALPHA;

void al_init_state(ALLEGRO_STATE* state, int flags)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(ALLEGRO_STATE));

    if (flags & ALLEGRO_STATE_NEW_DISPLAY_FLAGS) {
        state->new_display_flags = _new_display_flags;
    }
    if (flags & ALLEGRO_STATE_NEW_BITMAP_FLAGS) {
        state->new_bitmap_flags = _new_bitmap_flags;
    }
    if (flags & ALLEGRO_STATE_NEW_BITMAP_FORMAT) {
        state->new_bitmap_format = _new_bitmap_format;
    }
    if (flags & ALLEGRO_STATE_DISPLAY) {
        state->current_display = _current_display;
    }
    if (flags & ALLEGRO_STATE_TARGET_BITMAP) {
        state->target_bitmap = _target_bitmap;
    }
    if (flags & ALLEGRO_STATE_BLENDER) {
        state->blender_op = _blender_op;
        state->blender_src = _blender_src;
        state->blender_dst = _blender_dst;
        state->blender_alpha_op = _blender_alpha_op;
        state->blender_alpha_src = _blender_alpha_src;
        state->blender_alpha_dst = _blender_alpha_dst;
    }
    if (flags & ALLEGRO_STATE_TRANSFORM) {
        memset(state->transform, 0, sizeof(state->transform));
        state->transform[0] = 1.0f;
        state->transform[5] = 1.0f;
        state->transform[10] = 1.0f;
        state->transform[15] = 1.0f;
    }
}

void al_store_state(ALLEGRO_STATE* state, int flags)
{
    al_init_state(state, flags);
}

void al_restore_state(ALLEGRO_STATE* state)
{
    if (!state) {
        return;
    }

    if (state->new_display_flags) {
        _new_display_flags = state->new_display_flags;
    }
    if (state->new_bitmap_flags) {
        _new_bitmap_flags = state->new_bitmap_flags;
    }
    if (state->new_bitmap_format) {
        _new_bitmap_format = state->new_bitmap_format;
    }
    if (state->current_display) {
        _current_display = state->current_display;
    }
    if (state->target_bitmap) {
        al_set_target_bitmap(state->target_bitmap);
    }
    if (state->blender_op) {
        _blender_op = state->blender_op;
        _blender_src = state->blender_src;
        _blender_dst = state->blender_dst;
        _blender_alpha_op = state->blender_alpha_op;
        _blender_alpha_src = state->blender_alpha_src;
        _blender_alpha_dst = state->blender_alpha_dst;
    }
}

static ALLEGRO_TRANSFORM _current_transform;
static bool _transform_initialized = false;

void al_identity_transform(ALLEGRO_TRANSFORM* trans)
{
    if (!trans) {
        return;
    }
    memset(trans->m, 0, sizeof(trans->m));
    trans->m[0] = 1.0f;
    trans->m[5] = 1.0f;
    trans->m[10] = 1.0f;
    trans->m[15] = 1.0f;
}

void al_copy_transform(ALLEGRO_TRANSFORM* dest, const ALLEGRO_TRANSFORM* src)
{
    if (!dest || !src) {
        return;
    }
    memcpy(dest->m, src->m, sizeof(dest->m));
}

void al_use_transform(ALLEGRO_TRANSFORM* trans)
{
    if (!trans) {
        return;
    }
    al_copy_transform(&_current_transform, trans);
    _transform_initialized = true;
}

ALLEGRO_TRANSFORM* al_get_current_transform(void)
{
    if (!_transform_initialized) {
        al_identity_transform(&_current_transform);
        _transform_initialized = true;
    }
    return &_current_transform;
}

void al_invert_transform(ALLEGRO_TRANSFORM* trans)
{
    if (!trans) {
        return;
    }

    float m00 = trans->m[0], m01 = trans->m[1], m02 = trans->m[2], m03 = trans->m[3];
    float m10 = trans->m[4], m11 = trans->m[5], m12 = trans->m[6], m13 = trans->m[7];
    float m20 = trans->m[8], m21 = trans->m[9], m22 = trans->m[10], m23 = trans->m[11];
    float m30 = trans->m[12], m31 = trans->m[13], m32 = trans->m[14], m33 = trans->m[15];

    float det = m00 * (m11 * m22 * m33 + m12 * m23 * m31 + m13 * m21 * m32 - m13 * m22 * m31 - m12 * m21 * m33 - m11 * m23 * m32)
              - m01 * (m10 * m22 * m33 + m12 * m23 * m30 + m13 * m20 * m32 - m13 * m22 * m30 - m12 * m20 * m33 - m10 * m23 * m32)
              + m02 * (m10 * m21 * m33 + m11 * m23 * m30 + m13 * m20 * m31 - m13 * m21 * m30 - m11 * m20 * m33 - m10 * m23 * m31)
              - m03 * (m10 * m21 * m32 + m11 * m22 * m30 + m12 * m20 * m31 - m12 * m21 * m30 - m11 * m20 * m32 - m10 * m22 * m31);

    if (fabsf(det) < 1e-10) {
        return;
    }

    float inv_det = 1.0f / det;

    trans->m[0] = (m11 * m22 * m33 + m12 * m23 * m31 + m13 * m21 * m32 - m13 * m22 * m31 - m12 * m21 * m33 - m11 * m23 * m32) * inv_det;
    trans->m[1] = (m01 * m22 * m33 + m02 * m23 * m31 + m03 * m21 * m32 - m01 * m23 * m32 - m02 * m21 * m33 - m03 * m22 * m31) * inv_det;
    trans->m[2] = (m01 * m12 * m33 + m02 * m13 * m31 + m03 * m11 * m32 - m01 * m13 * m32 - m02 * m11 * m33 - m03 * m12 * m31) * inv_det;
    trans->m[3] = (m01 * m13 * m22 + m02 * m11 * m23 + m03 * m12 * m21 - m01 * m12 * m23 - m02 * m13 * m21 - m03 * m11 * m22) * inv_det;
    trans->m[4] = (m10 * m23 * m32 + m12 * m20 * m33 + m13 * m22 * m30 - m10 * m22 * m33 - m12 * m23 * m30 - m13 * m20 * m32) * inv_det;
    trans->m[5] = (m00 * m22 * m33 + m02 * m23 * m30 + m03 * m20 * m32 - m00 * m23 * m32 - m02 * m20 * m33 - m03 * m22 * m30) * inv_det;
    trans->m[6] = (m00 * m13 * m32 + m02 * m10 * m33 + m03 * m12 * m30 - m00 * m12 * m33 - m02 * m13 * m30 - m03 * m10 * m32) * inv_det;
    trans->m[7] = (m00 * m12 * m23 + m02 * m13 * m20 + m03 * m10 * m22 - m00 * m13 * m22 - m02 * m10 * m23 - m03 * m12 * m20) * inv_det;
    trans->m[8] = (m10 * m21 * m33 + m11 * m23 * m30 + m13 * m20 * m31 - m10 * m23 * m31 - m11 * m20 * m33 - m13 * m21 * m30) * inv_det;
    trans->m[9] = (m00 * m23 * m31 + m01 * m20 * m33 + m03 * m21 * m30 - m00 * m21 * m33 - m01 * m23 * m30 - m03 * m20 * m31) * inv_det;
    trans->m[10] = (m00 * m11 * m33 + m01 * m13 * m30 + m03 * m10 * m31 - m00 * m13 * m31 - m01 * m10 * m33 - m03 * m11 * m30) * inv_det;
    trans->m[11] = (m00 * m13 * m21 + m01 * m10 * m23 + m03 * m11 * m20 - m00 * m11 * m23 - m01 * m13 * m20 - m03 * m10 * m21) * inv_det;
    trans->m[12] = (m10 * m22 * m31 + m11 * m20 * m32 + m12 * m21 * m30 - m10 * m21 * m32 - m11 * m22 * m30 - m12 * m20 * m31) * inv_det;
    trans->m[13] = (m00 * m21 * m32 + m01 * m22 * m30 + m02 * m20 * m31 - m00 * m22 * m31 - m01 * m20 * m32 - m02 * m21 * m30) * inv_det;
    trans->m[14] = (m00 * m12 * m31 + m01 * m10 * m32 + m02 * m11 * m30 - m00 * m11 * m32 - m01 * m12 * m30 - m02 * m10 * m31) * inv_det;
    trans->m[15] = (m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 - m00 * m12 * m21 - m01 * m10 * m22 - m02 * m11 * m20) * inv_det;
}

int al_check_inverse(const ALLEGRO_TRANSFORM* trans)
{
    if (!trans) {
        return 0;
    }

    float det = trans->m[0] * (trans->m[5] * trans->m[10] * trans->m[15] + trans->m[6] * trans->m[11] * trans->m[13] + trans->m[7] * trans->m[9] * trans->m[14]
                           - trans->m[7] * trans->m[10] * trans->m[13] - trans->m[6] * trans->m[9] * trans->m[15] - trans->m[5] * trans->m[11] * trans->m[14])
              - trans->m[1] * (trans->m[4] * trans->m[10] * trans->m[15] + trans->m[6] * trans->m[11] * trans->m[12] + trans->m[7] * trans->m[9] * trans->m[14]
                           - trans->m[7] * trans->m[10] * trans->m[12] - trans->m[6] * trans->m[9] * trans->m[15] - trans->m[4] * trans->m[11] * trans->m[14])
              + trans->m[2] * (trans->m[4] * trans->m[9] * trans->m[15] + trans->m[5] * trans->m[11] * trans->m[12] + trans->m[7] * trans->m[8] * trans->m[13]
                           - trans->m[7] * trans->m[9] * trans->m[12] - trans->m[5] * trans->m[8] * trans->m[15] - trans->m[4] * trans->m[11] * trans->m[13])
              - trans->m[3] * (trans->m[4] * trans->m[9] * trans->m[14] + trans->m[5] * trans->m[10] * trans->m[12] + trans->m[6] * trans->m[8] * trans->m[13]
                           - trans->m[6] * trans->m[9] * trans->m[12] - trans->m[5] * trans->m[8] * trans->m[14] - trans->m[4] * trans->m[10] * trans->m[13]);

    if (fabsf(det) < 1e-10) {
        return 0;
    }

    return 1;
}

void al_transform_coordinates(const ALLEGRO_TRANSFORM* trans, float* x, float* y)
{
    if (!trans || !x || !y) {
        return;
    }

    float x_val = *x;
    float y_val = *y;
    float w = trans->m[3] * x_val + trans->m[7] * y_val + trans->m[15];

    if (fabsf(w) < 1e-10) {
        w = 1.0f;
    }

    *x = (trans->m[0] * x_val + trans->m[4] * y_val + trans->m[12]) / w;
    *y = (trans->m[1] * x_val + trans->m[5] * y_val + trans->m[13]) / w;
}

void al_compose_transform(ALLEGRO_TRANSFORM* dest, const ALLEGRO_TRANSFORM* src)
{
    if (!dest || !src) {
        return;
    }

    ALLEGRO_TRANSFORM result;
    const float* a = dest->m;
    const float* b = src->m;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i * 4 + j] = a[i * 4 + 0] * b[0 * 4 + j] +
                                   a[i * 4 + 1] * b[1 * 4 + j] +
                                   a[i * 4 + 2] * b[2 * 4 + j] +
                                   a[i * 4 + 3] * b[3 * 4 + j];
        }
    }

    memcpy(dest->m, result.m, sizeof(dest->m));
}

void al_translate_transform(ALLEGRO_TRANSFORM* trans, float x, float y, float z)
{
    if (!trans) {
        return;
    }

    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    t.m[12] = x;
    t.m[13] = y;
    t.m[14] = z;

    al_compose_transform(trans, &t);
}

void al_rotate_transform(ALLEGRO_TRANSFORM* trans, float angle)
{
    if (!trans) {
        return;
    }

    float c = cosf(angle);
    float s = sinf(angle);

    ALLEGRO_TRANSFORM r;
    al_identity_transform(&r);
    r.m[0] = c;
    r.m[1] = s;
    r.m[4] = -s;
    r.m[5] = c;

    al_compose_transform(trans, &r);
}

void al_scale_transform(ALLEGRO_TRANSFORM* trans, float sx, float sy, float sz)
{
    if (!trans) {
        return;
    }

    ALLEGRO_TRANSFORM s;
    al_identity_transform(&s);
    s.m[0] = sx;
    s.m[5] = sy;
    s.m[10] = sz;

    al_compose_transform(trans, &s);
}

void al_translate_transform_f(ALLEGRO_TRANSFORM* trans, float x, float y, float z)
{
    al_translate_transform(trans, x, y, z);
}

void al_rotate_transform_f(ALLEGRO_TRANSFORM* trans, float angle, float x, float y, float z)
{
    if (!trans) {
        return;
    }

    if (x == 0 && y == 0 && z == 0) {
        return;
    }

    float len = sqrtf(x * x + y * y + z * z);
    x /= len;
    y /= len;
    z /= len;

    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    ALLEGRO_TRANSFORM r;
    al_identity_transform(&r);
    r.m[0] = t * x * x + c;
    r.m[1] = t * x * y + s * z;
    r.m[2] = t * x * z - s * y;
    r.m[4] = t * x * y - s * z;
    r.m[5] = t * y * y + c;
    r.m[6] = t * y * z + s * x;
    r.m[8] = t * x * z + s * y;
    r.m[9] = t * y * z - s * x;
    r.m[10] = t * z * z + c;

    al_compose_transform(trans, &r);
}

void al_scale_transform_f(ALLEGRO_TRANSFORM* trans, float sx, float sy, float sz)
{
    al_scale_transform(trans, sx, sy, sz);
}

void al_set_blender(int op, int src, int dst)
{
    _blender_op = op;
    _blender_src = src;
    _blender_dst = dst;
    _blender_alpha_op = op;
    _blender_alpha_src = src;
    _blender_alpha_dst = dst;
}

void al_get_blender(int* op, int* src, int* dst)
{
    if (op) *op = _blender_op;
    if (src) *src = _blender_src;
    if (dst) *dst = _blender_dst;
}

void al_set_separate_blender(int op, int src, int dst, int alpha_op, int src_alpha, int dst_alpha)
{
    _blender_op = op;
    _blender_src = src;
    _blender_dst = dst;
    _blender_alpha_op = alpha_op;
    _blender_alpha_src = src_alpha;
    _blender_alpha_dst = dst_alpha;
}

void al_get_separate_blender(int* op, int* src, int* dst, int* alpha_op, int* src_alpha, int* dst_alpha)
{
    if (op) *op = _blender_op;
    if (src) *src = _blender_src;
    if (dst) *dst = _blender_dst;
    if (alpha_op) *alpha_op = _blender_alpha_op;
    if (src_alpha) *src_alpha = _blender_alpha_src;
    if (dst_alpha) *dst_alpha = _blender_alpha_dst;
}

struct ALLEGRO_EVENT_QUEUE {
    std::vector<ALLEGRO_EVENT> events;
    size_t read_index;
};

ALLEGRO_EVENT_QUEUE* al_create_event_queue(void)
{
    ALLEGRO_EVENT_QUEUE* queue = new ALLEGRO_EVENT_QUEUE;
    if (!queue) {
        return nullptr;
    }
    queue->read_index = 0;
    return queue;
}

void al_destroy_event_queue(ALLEGRO_EVENT_QUEUE* queue)
{
    if (!queue) {
        return;
    }
    delete queue;
}

bool al_is_event_queue_empty(ALLEGRO_EVENT_QUEUE* queue)
{
    if (!queue) {
        return true;
    }
    return queue->events.empty() || queue->read_index >= queue->events.size();
}

bool al_get_next_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event)
{
    if (!queue || !event) {
        return false;
    }
    if (queue->events.empty() || queue->read_index >= queue->events.size()) {
        return false;
    }
    *event = queue->events[queue->read_index];
    queue->read_index++;
    if (queue->read_index >= queue->events.size() / 2 && !queue->events.empty()) {
        size_t remaining = queue->events.size() - queue->read_index;
        std::vector<ALLEGRO_EVENT> new_events(queue->events.begin() + queue->read_index, queue->events.end());
        queue->events = new_events;
        queue->read_index = 0;
    }
    return true;
}

bool al_peek_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event)
{
    if (!queue || !event) {
        return false;
    }
    if (queue->events.empty() || queue->read_index >= queue->events.size()) {
        return false;
    }
    *event = queue->events[queue->read_index];
    return true;
}

void al_drop_next_event(ALLEGRO_EVENT_QUEUE* queue)
{
    if (!queue) {
        return;
    }
    if (!queue->events.empty() && queue->read_index < queue->events.size()) {
        queue->read_index++;
    }
}

void al_flush_event_queue(ALLEGRO_EVENT_QUEUE* queue)
{
    if (!queue) {
        return;
    }
    queue->events.clear();
    queue->read_index = 0;
}

void al_wait_for_event(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event)
{
    if (!queue || !event) {
        return;
    }
    while (al_is_event_queue_empty(queue)) {
        SDL_Event sdl_event;
        if (SDL_WaitEvent(&sdl_event)) {
            ALLEGRO_EVENT al_event;
            al_event.type = 0;
            al_event.display = _current_display;
            al_event.timestamp = SDL_GetTicks() / 1000.0;
            
            if (sdl_event.type == SDL_KEYDOWN) {
                al_event.type = ALLEGRO_EVENT_KEY_DOWN;
                al_event.keyboard.keycode = sdl_event.key.keysym.sym;
                al_event.keyboard.modifiers = sdl_event.key.keysym.mod;
            } else if (sdl_event.type == SDL_KEYUP) {
                al_event.type = ALLEGRO_EVENT_KEY_UP;
                al_event.keyboard.keycode = sdl_event.key.keysym.sym;
                al_event.keyboard.modifiers = sdl_event.key.keysym.mod;
            } else if (sdl_event.type == SDL_MOUSEBUTTONDOWN) {
                al_event.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
                al_event.mouse.button = sdl_event.button.button;
                al_event.mouse.x = sdl_event.button.x;
                al_event.mouse.y = sdl_event.button.y;
            } else if (sdl_event.type == SDL_MOUSEBUTTONUP) {
                al_event.type = ALLEGRO_EVENT_MOUSE_BUTTON_UP;
                al_event.mouse.button = sdl_event.button.button;
                al_event.mouse.x = sdl_event.button.x;
                al_event.mouse.y = sdl_event.button.y;
            } else if (sdl_event.type == SDL_MOUSEMOTION) {
                al_event.type = ALLEGRO_EVENT_MOUSE_AXES;
                al_event.mouse.x = sdl_event.motion.x;
                al_event.mouse.y = sdl_event.motion.y;
                al_event.mouse.dx = sdl_event.motion.xrel;
                al_event.mouse.dy = sdl_event.motion.yrel;
            } else if (sdl_event.type == SDL_WINDOWEVENT) {
                if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    al_event.type = ALLEGRO_EVENT_DISPLAY_RESIZE;
                    al_event.display_expose.width = sdl_event.window.data1;
                    al_event.display_expose.height = sdl_event.window.data2;
                } else if (sdl_event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    al_event.type = ALLEGRO_EVENT_DISPLAY_CLOSE;
                }
            }
            
            if (al_event.type != 0) {
                queue->events.push_back(al_event);
            }
        }
    }
    al_get_next_event(queue, event);
}

bool al_wait_for_event_timed(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event, float secs)
{
    if (!queue || !event) {
        return false;
    }
    
    if (!al_is_event_queue_empty(queue)) {
        return al_get_next_event(queue, event);
    }
    
    SDL_Event sdl_event;
    int timeout_ms = static_cast<int>(secs * 1000);
    if (SDL_WaitEventTimeout(&sdl_event, timeout_ms)) {
        event->type = 0;
        event->display = _current_display;
        event->timestamp = SDL_GetTicks() / 1000.0;
        
        if (sdl_event.type == SDL_KEYDOWN) {
            event->type = ALLEGRO_EVENT_KEY_DOWN;
            event->keyboard.keycode = sdl_event.key.keysym.sym;
        } else if (sdl_event.type == SDL_KEYUP) {
            event->type = ALLEGRO_EVENT_KEY_UP;
            event->keyboard.keycode = sdl_event.key.keysym.sym;
        }
        
        return event->type != 0;
    }
    
    return false;
}

bool al_wait_for_event_until(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* event, void* timeout)
{
    (void)timeout;
    return al_wait_for_event_timed(queue, event, 1.0f);
}

void al_init_event_source(ALLEGRO_EVENT_SOURCE* source)
{
    (void)source;
}

void al_destroy_event_source(ALLEGRO_EVENT_SOURCE* source)
{
    (void)source;
}

void al_register_event_source(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT_SOURCE* source)
{
    (void)queue;
    (void)source;
}

void al_unregister_event_source(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT_SOURCE* source)
{
    (void)queue;
    (void)source;
}

bool al_install_mouse(void)
{
    if (_mouse_installed) {
        return true;
    }
    _mouse_installed = true;
    _mouse_x = 0;
    _mouse_y = 0;
    _mouse_z = 0;
    _mouse_w = 0;
    _mouse_buttons = 0;
    return true;
}

void al_uninstall_mouse(void)
{
    _mouse_installed = false;
    _mouse_x = 0;
    _mouse_y = 0;
    _mouse_z = 0;
    _mouse_w = 0;
    _mouse_buttons = 0;
}

bool al_is_mouse_installed(void)
{
    return _mouse_installed;
}

int install_mouse(void)
{
    return al_install_mouse() ? 0 : -1;
}

int remove_mouse(void)
{
    al_uninstall_mouse();
    return 0;
}

void* al_get_mouse_event_source(void)
{
    return nullptr;
}

void al_get_mouse_state(ALLEGRO_MOUSE_STATE* ret_state)
{
    if (!ret_state) {
        return;
    }

    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    
    ret_state->x = x;
    ret_state->y = y;
    ret_state->z = _mouse_z;
    ret_state->w = _mouse_w;
    ret_state->pressure = 0;
    ret_state->button = 0;
    ret_state->buttons = 0;
    ret_state->display = _current_display;

    if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        ret_state->buttons |= (1 << 0);
    }
    if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        ret_state->buttons |= (1 << 1);
    }
    if (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
        ret_state->buttons |= (1 << 2);
    }
    if (buttons & SDL_BUTTON(SDL_BUTTON_X1)) {
        ret_state->buttons |= (1 << 3);
    }
    if (buttons & SDL_BUTTON(SDL_BUTTON_X2)) {
        ret_state->buttons |= (1 << 4);
    }
}

bool al_mouse_button_down(const ALLEGRO_MOUSE_STATE* state, int button)
{
    if (!state || button < 1 || button > 32) {
        return false;
    }
    return (state->buttons & (1 << (button - 1))) != 0;
}

int al_get_mouse_state_axis(const ALLEGRO_MOUSE_STATE* state, int axis)
{
    if (!state) {
        return 0;
    }

    switch (axis) {
        case 0: return state->x;
        case 1: return state->y;
        case 2: return state->z;
        case 3: return state->w;
        default: return 0;
    }
}

int al_get_mouse_num_axes(void)
{
    return _mouse_num_axes;
}

int al_get_mouse_num_buttons(void)
{
    return _mouse_num_buttons;
}

bool al_set_mouse_xy(ALLEGRO_DISPLAY* display, float x, float y)
{
    (void)display;
    SDL_WarpMouseInWindow(nullptr, (int)x, (int)y);
    _mouse_x = (int)x;
    _mouse_y = (int)y;
    return true;
}

bool al_set_mouse_z(float z)
{
    _mouse_z = (int)z;
    return true;
}

bool al_set_mouse_w(float w)
{
    _mouse_w = (int)w;
    return true;
}

bool al_get_mouse_cursor_position(int* x, int* y)
{
    if (!x || !y) {
        return false;
    }
    SDL_GetMouseState(x, y);
    return true;
}

bool al_install_joystick(void)
{
    if (_joystick_installed) {
        return true;
    }

    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        return false;
    }

    _joystick_installed = true;

    int num_joysticks = SDL_NumJoysticks();
    for (int i = 0; i < num_joysticks; i++) {
        ALLEGRO_JOYSTICK* joy = new ALLEGRO_JOYSTICK;
        joy->index = i;
        joy->is_controller = false;
        joy->controller = nullptr;
        joy->joystick = nullptr;

        if (SDL_IsGameController(i)) {
            joy->controller = SDL_GameControllerOpen(i);
            if (joy->controller) {
                joy->is_controller = true;
                const char* name = SDL_GameControllerName(joy->controller);
                if (name) {
                    strncpy(joy->name, name, sizeof(joy->name) - 1);
                    joy->name[sizeof(joy->name) - 1] = '\0';
                } else {
                    snprintf(joy->name, sizeof(joy->name), "Controller %d", i);
                }
            }
        }

        if (!joy->controller) {
            joy->joystick = SDL_JoystickOpen(i);
            if (joy->joystick) {
                const char* name = SDL_JoystickName(joy->joystick);
                if (name) {
                    strncpy(joy->name, name, sizeof(joy->name) - 1);
                    joy->name[sizeof(joy->name) - 1] = '\0';
                } else {
                    snprintf(joy->name, sizeof(joy->name), "Joystick %d", i);
                }
            }
        }

        if (joy->controller || joy->joystick) {
            _joysticks.push_back(joy);
        } else {
            delete joy;
        }
    }

    return true;
}

void al_uninstall_joystick(void)
{
    for (size_t i = 0; i < _joysticks.size(); i++) {
        ALLEGRO_JOYSTICK* joy = _joysticks[i];
        if (joy->controller) {
            SDL_GameControllerClose(joy->controller);
        }
        if (joy->joystick) {
            SDL_JoystickClose(joy->joystick);
        }
        delete joy;
    }
    _joysticks.clear();
    _joystick_installed = false;
}

bool al_is_joystick_installed(void)
{
    return _joystick_installed;
}

bool al_reconfigure_joysticks(void)
{
    al_uninstall_joystick();
    return al_install_joystick();
}

int al_get_num_joysticks(void)
{
    return (int)_joysticks.size();
}

ALLEGRO_JOYSTICK* al_get_joystick(int joyn)
{
    if (joyn < 0 || joyn >= (int)_joysticks.size()) {
        return nullptr;
    }
    return _joysticks[joyn];
}

void al_release_joystick(ALLEGRO_JOYSTICK* joystick)
{
    (void)joystick;
}

bool al_get_joystick_active(ALLEGRO_JOYSTICK* joystick)
{
    if (!joystick) {
        return false;
    }
    return (joystick->controller && SDL_GameControllerGetAttached(joystick->controller)) ||
           (joystick->joystick && SDL_JoystickGetAttached(joystick->joystick));
}

const char* al_get_joystick_name(ALLEGRO_JOYSTICK* joystick)
{
    if (!joystick) {
        return nullptr;
    }
    return joystick->name;
}

int al_get_joystick_num_sticks(ALLEGRO_JOYSTICK* joystick)
{
    if (!joystick) {
        return 0;
    }
    if (joystick->is_controller) {
        return 3;
    }
    if (joystick->joystick) {
        int axes = SDL_JoystickNumAxes(joystick->joystick);
        return (axes + 1) / 2;
    }
    return 0;
}

int al_get_joystick_stick_flags(ALLEGRO_JOYSTICK* joystick, int stick)
{
    (void)joystick;
    (void)stick;
    return ALLEGRO_JOYFLAG_ANALOGUE;
}

const char* al_get_joystick_stick_name(ALLEGRO_JOYSTICK* joystick, int stick)
{
    (void)joystick;
    switch (stick) {
        case 0: return "Left Stick";
        case 1: return "Right Stick";
        case 2: return "D-Pad";
        default: return "Unknown";
    }
}

int al_get_joystick_num_axes(ALLEGRO_JOYSTICK* joystick, int stick)
{
    if (!joystick) {
        return 0;
    }
    if (joystick->is_controller) {
        if (stick < 2) {
            return 2;
        }
        return 2;
    }
    if (joystick->joystick) {
        int axes = SDL_JoystickNumAxes(joystick->joystick);
        if (stick * 2 + 1 < axes) {
            return 2;
        }
        return axes - stick * 2;
    }
    return 0;
}

const char* al_get_joystick_axis_name(ALLEGRO_JOYSTICK* joystick, int stick, int axis)
{
    (void)joystick;
    if (stick < 2) {
        return (axis == 0) ? "X" : "Y";
    }
    return (axis == 0) ? "X" : "Y";
}

int al_get_joystick_num_buttons(ALLEGRO_JOYSTICK* joystick)
{
    if (!joystick) {
        return 0;
    }
    if (joystick->is_controller) {
        return SDL_CONTROLLER_BUTTON_MAX;
    }
    if (joystick->joystick) {
        return SDL_JoystickNumButtons(joystick->joystick);
    }
    return 0;
}

const char* al_get_joystick_button_name(ALLEGRO_JOYSTICK* joystick, int button)
{
    (void)joystick;
    static const char* controller_buttons[] = {
        "A", "B", "X", "Y", "BACK", "GUIDE", "START",
        "LS", "RS", "LB", "RB", "DPAD_UP", "DPAD_DOWN",
        "DPAD_LEFT", "DPAD_RIGHT"
    };
    if (button >= 0 && button < 15) {
        return controller_buttons[button];
    }
    static char buf[16];
    snprintf(buf, sizeof(buf), "Button %d", button);
    return buf;
}

void al_get_joystick_state(ALLEGRO_JOYSTICK* joystick, ALLEGRO_JOYSTICK_STATE* ret_state)
{
    if (!joystick || !ret_state) {
        return;
    }

    memset(ret_state, 0, sizeof(ALLEGRO_JOYSTICK_STATE));

    if (joystick->is_controller && joystick->controller) {
        ret_state->stick[0][0] = SDL_GameControllerGetAxis(joystick->controller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
        ret_state->stick[0][1] = SDL_GameControllerGetAxis(joystick->controller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
        ret_state->stick[1][0] = SDL_GameControllerGetAxis(joystick->controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
        ret_state->stick[1][1] = SDL_GameControllerGetAxis(joystick->controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;

        int lt = SDL_GameControllerGetAxis(joystick->controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        int rt = SDL_GameControllerGetAxis(joystick->controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        ret_state->stick[2][0] = (lt + 32768) / 65535.0f;
        ret_state->stick[2][1] = (rt + 32768) / 65535.0f;

        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX && i < 32; i++) {
            ret_state->button[i] = SDL_GameControllerGetButton(joystick->controller, (SDL_GameControllerButton)i) ? 32767 : 0;
        }

        ret_state->button[SDL_CONTROLLER_BUTTON_DPAD_UP] = SDL_GameControllerGetButton(joystick->controller, SDL_CONTROLLER_BUTTON_DPAD_UP) ? 32767 : 0;
        ret_state->button[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = SDL_GameControllerGetButton(joystick->controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ? 32767 : 0;
        ret_state->button[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = SDL_GameControllerGetButton(joystick->controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ? 32767 : 0;
        ret_state->button[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = SDL_GameControllerGetButton(joystick->controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ? 32767 : 0;
    } else if (joystick->joystick) {
        int num_axes = SDL_JoystickNumAxes(joystick->joystick);
        for (int i = 0; i < num_axes && i < ALLEGRO_JOYSTICK_MAX_AXES; i++) {
            ret_state->stick[0][i] = SDL_JoystickGetAxis(joystick->joystick, i) / 32767.0f;
        }

        int num_buttons = SDL_JoystickNumButtons(joystick->joystick);
        for (int i = 0; i < num_buttons && i < 32; i++) {
            ret_state->button[i] = SDL_JoystickGetButton(joystick->joystick, i) ? 32767 : 0;
        }
    }
}

void* al_get_joystick_event_source(void)
{
    return nullptr;
}

int install_joystick(void)
{
    return al_install_joystick() ? 0 : -1;
}

int remove_joystick(void)
{
    al_uninstall_joystick();
    return 0;
}

bool al_install_audio(void)
{
    if (_audio_installed) {
        return true;
    }
    
    if (Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG) == 0) {
        return false;
    }
    
    if (Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 1024) < 0) {
        Mix_Quit();
        return false;
    }
    
    _audio_installed = true;
    return true;
}

void al_uninstall_audio(void)
{
    if (!_audio_installed) {
        return;
    }
    
    if (_default_mixer) {
        al_destroy_mixer(_default_mixer);
        _default_mixer = nullptr;
    }
    
    Mix_CloseAudio();
    Mix_Quit();
    _audio_installed = false;
}

bool al_init_acodec_addon(void)
{
    return (Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG) != 0);
}

bool al_is_audio_installed(void)
{
    return _audio_installed;
}

uint32_t al_get_allegro_audio_version(void)
{
    return ALLEGRO_AUDIO_VERSION;
}

bool al_reserve_samples(int reserve_samples)
{
    if (!_audio_installed) {
        return false;
    }
    
    _audio_reserved_channels = reserve_samples;
    return true;
}

ALLEGRO_SAMPLE* al_create_sample(void* buf, unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf, bool free_buf)
{
    ALLEGRO_SAMPLE* sample = new ALLEGRO_SAMPLE;
    if (!sample) {
        return nullptr;
    }
    
    sample->num_samples = samples;
    sample->frequency = freq;
    sample->depth = depth;
    sample->chan_conf = chan_conf;
    sample->data = buf;
    sample->free_buffer = free_buf;
    
    return sample;
}

void al_destroy_sample(ALLEGRO_SAMPLE* spl)
{
    if (!spl) {
        return;
    }
    
    if (spl->free_buffer && spl->data) {
        free(spl->data);
    }
    
    delete spl;
}

unsigned int al_get_sample_frequency(const ALLEGRO_SAMPLE* spl)
{
    return spl ? spl->frequency : 0;
}

unsigned int al_get_sample_length(const ALLEGRO_SAMPLE* spl)
{
    return spl ? spl->num_samples : 0;
}

ALLEGRO_AUDIO_DEPTH al_get_sample_depth(const ALLEGRO_SAMPLE* spl)
{
    return spl ? static_cast<ALLEGRO_AUDIO_DEPTH>(spl->depth) : ALLEGRO_AUDIO_DEPTH_INT16;
}

ALLEGRO_CHANNEL_CONF al_get_sample_channels(const ALLEGRO_SAMPLE* spl)
{
    return spl ? static_cast<ALLEGRO_CHANNEL_CONF>(spl->chan_conf) : ALLEGRO_CHANNEL_CONF_2;
}

void* al_get_sample_data(const ALLEGRO_SAMPLE* spl)
{
    return spl ? spl->data : nullptr;
}

ALLEGRO_SAMPLE* al_load_sample(const char* filename)
{
    if (!filename || !_audio_installed) {
        return nullptr;
    }
    
    Mix_Chunk* chunk = Mix_LoadWAV(filename);
    if (!chunk) {
        return nullptr;
    }
    
    ALLEGRO_SAMPLE* sample = new ALLEGRO_SAMPLE;
    if (!sample) {
        Mix_FreeChunk(chunk);
        return nullptr;
    }
    
    sample->num_samples = chunk->alen / 2;
    sample->frequency = 44100;
    sample->depth = ALLEGRO_AUDIO_DEPTH_INT16;
    sample->chan_conf = ALLEGRO_CHANNEL_CONF_2;
    sample->data = chunk;
    sample->free_buffer = true;
    
    return sample;
}

bool al_save_sample(const char* filename, ALLEGRO_SAMPLE* spl)
{
    return false;
}

ALLEGRO_SAMPLE* al_load_sample_f(ALLEGRO_FILE* fp, const char* ident)
{
    if (!fp || !fp->fp || !_audio_installed) {
        return nullptr;
    }
    
    SDL_RWops* rw = SDL_RWFromFP(fp->fp, SDL_FALSE);
    if (!rw) {
        return nullptr;
    }
    
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 0);
    if (!chunk) {
        return nullptr;
    }
    
    ALLEGRO_SAMPLE* sample = new ALLEGRO_SAMPLE;
    if (!sample) {
        Mix_FreeChunk(chunk);
        return nullptr;
    }
    
    sample->num_samples = chunk->alen / 2;
    sample->frequency = 44100;
    sample->depth = ALLEGRO_AUDIO_DEPTH_INT16;
    sample->chan_conf = ALLEGRO_CHANNEL_CONF_2;
    sample->data = chunk;
    sample->free_buffer = true;
    
    return sample;
}

ALLEGRO_SAMPLE_INSTANCE* al_create_sample_instance(ALLEGRO_SAMPLE* data)
{
    if (!data) {
        return nullptr;
    }
    
    AllegroSampleInstance* instance = new AllegroSampleInstance;
    if (!instance) {
        return nullptr;
    }
    
    instance->chunk = static_cast<Mix_Chunk*>(data->data);
    instance->channel = -1;
    instance->is_playing = false;
    instance->loop = ALLEGRO_PLAYMODE_ONCE;
    instance->gain = 1.0f;
    instance->pan = 0.0f;
    instance->speed = 1.0f;
    instance->position = 0;
    instance->sample = data;
    
    return reinterpret_cast<ALLEGRO_SAMPLE_INSTANCE*>(instance);
}

void al_destroy_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    if (instance->channel >= 0) {
        Mix_HaltChannel(instance->channel);
    }
    
    delete instance;
}

bool al_play_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    
    if (instance->channel >= 0) {
        Mix_HaltChannel(instance->channel);
    }
    
    int loops = (instance->loop == ALLEGRO_PLAYMODE_LOOP) ? -1 : 0;
    int channel = Mix_PlayChannel(-1, instance->chunk, loops);
    
    if (channel < 0) {
        return false;
    }
    
    instance->channel = channel;
    instance->is_playing = true;
    
    Mix_Volume(channel, static_cast<int>(instance->gain * 128.0f));
    
    return true;
}

bool al_stop_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    
    if (instance->channel >= 0) {
        Mix_HaltChannel(instance->channel);
        instance->channel = -1;
    }
    
    instance->is_playing = false;
    return true;
}

bool al_get_sample_instance_playing(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(const_cast<ALLEGRO_SAMPLE_INSTANCE*>(spl));
    
    if (instance->channel >= 0 && Mix_Playing(instance->channel)) {
        return true;
    }
    
    return false;
}

bool al_set_sample_instance_playing(ALLEGRO_SAMPLE_INSTANCE* spl, bool val)
{
    if (!spl) {
        return false;
    }
    
    if (val) {
        return al_play_sample_instance(spl);
    } else {
        return al_stop_sample_instance(spl);
    }
}

unsigned int al_get_sample_instance_position(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return 0;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(const_cast<ALLEGRO_SAMPLE_INSTANCE*>(spl));
    
    if (instance->channel >= 0 && instance->chunk) {
        return static_cast<unsigned int>(SDL_GetTicks() * 44);
    }
    
    return 0;
}

bool al_set_sample_instance_position(ALLEGRO_SAMPLE_INSTANCE* spl, unsigned int pos)
{
    return false;
}

unsigned int al_get_sample_instance_length(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return 0;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(const_cast<ALLEGRO_SAMPLE_INSTANCE*>(spl));
    
    if (instance->chunk) {
        return instance->chunk->alen / 2;
    }
    
    return 0;
}

bool al_set_sample_instance_length(ALLEGRO_SAMPLE_INSTANCE* spl, unsigned int len)
{
    return false;
}

float al_get_sample_instance_speed(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return 1.0f;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(const_cast<ALLEGRO_SAMPLE_INSTANCE*>(spl));
    return instance->speed;
}

bool al_set_sample_instance_speed(ALLEGRO_SAMPLE_INSTANCE* spl, float val)
{
    if (!spl) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    instance->speed = val;
    return true;
}

float al_get_sample_instance_gain(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return 1.0f;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(const_cast<ALLEGRO_SAMPLE_INSTANCE*>(spl));
    return instance->gain;
}

bool al_set_sample_instance_gain(ALLEGRO_SAMPLE_INSTANCE* spl, float val)
{
    if (!spl) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    instance->gain = val;
    
    if (instance->channel >= 0) {
        Mix_Volume(instance->channel, static_cast<int>(val * 128.0f));
    }
    
    return true;
}

float al_get_sample_instance_pan(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return 0.0f;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(const_cast<ALLEGRO_SAMPLE_INSTANCE*>(spl));
    return instance->pan;
}

bool al_set_sample_instance_pan(ALLEGRO_SAMPLE_INSTANCE* spl, float val)
{
    if (!spl) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    instance->pan = val;
    return true;
}

ALLEGRO_PLAYMODE al_get_sample_instance_playmode(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return ALLEGRO_PLAYMODE_ONCE;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(const_cast<ALLEGRO_SAMPLE_INSTANCE*>(spl));
    return instance->loop;
}

bool al_set_sample_instance_playmode(ALLEGRO_SAMPLE_INSTANCE* spl, ALLEGRO_PLAYMODE val)
{
    if (!spl) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    instance->loop = val;
    return true;
}

ALLEGRO_AUDIO_DEPTH al_get_sample_instance_depth(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    return ALLEGRO_AUDIO_DEPTH_INT16;
}

ALLEGRO_CHANNEL_CONF al_get_sample_instance_channels(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    return ALLEGRO_CHANNEL_CONF_2;
}

bool al_get_sample_instance_attached(const ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return false;
    }
    
    const AllegroSampleInstance* instance = reinterpret_cast<const AllegroSampleInstance*>(spl);
    return instance->channel >= 0;
}

bool al_detach_sample_instance(ALLEGRO_SAMPLE_INSTANCE* spl)
{
    return al_stop_sample_instance(spl);
}

bool al_set_sample(ALLEGRO_SAMPLE_INSTANCE* spl, ALLEGRO_SAMPLE* data)
{
    if (!spl || !data) {
        return false;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    instance->chunk = static_cast<Mix_Chunk*>(data->data);
    instance->sample = data;
    return true;
}

ALLEGRO_SAMPLE* al_get_sample(ALLEGRO_SAMPLE_INSTANCE* spl)
{
    if (!spl) {
        return nullptr;
    }
    
    AllegroSampleInstance* instance = reinterpret_cast<AllegroSampleInstance*>(spl);
    return instance->sample;
}

ALLEGRO_MIXER* al_create_mixer(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf)
{
    if (!_audio_installed) {
        return nullptr;
    }
    
    MixerWrapper* mixer = new MixerWrapper;
    if (!mixer) {
        return nullptr;
    }
    
    mixer->frequency = freq;
    mixer->depth = depth;
    mixer->channels = chan_conf;
    mixer->quality = ALLEGRO_MIXER_QUALITY_MEDIUM;
    
    return reinterpret_cast<ALLEGRO_MIXER*>(mixer);
}

void al_destroy_mixer(ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return;
    }
    
    MixerWrapper* wrapper = reinterpret_cast<MixerWrapper*>(mixer);
    delete wrapper;
}

bool al_attach_sample_instance_to_mixer(ALLEGRO_SAMPLE_INSTANCE* stream, ALLEGRO_MIXER* mixer)
{
    if (!mixer || !stream) {
        return false;
    }
    
    MixerWrapper* wrapper = reinterpret_cast<MixerWrapper*>(mixer);
    wrapper->attached_sample_instance = stream;
    
    return true;
}

bool al_mixer_attach_sample(ALLEGRO_MIXER* mixer, ALLEGRO_SAMPLE* sample)
{
    if (!mixer || !sample) {
        return false;
    }
    
    ALLEGRO_SAMPLE_INSTANCE* instance = al_create_sample_instance(sample);
    if (!instance) {
        return false;
    }
    
    return al_attach_sample_instance_to_mixer(instance, mixer);
}

bool al_mixer_detach_sample(ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return false;
    }
    
    MixerWrapper* wrapper = reinterpret_cast<MixerWrapper*>(mixer);
    
    if (!wrapper->attached_sample_instance) {
        return false;
    }
    
    al_stop_sample_instance(wrapper->attached_sample_instance);
    al_destroy_sample_instance(wrapper->attached_sample_instance);
    wrapper->attached_sample_instance = nullptr;
    
    return true;
}

bool al_attach_audio_stream_to_mixer(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_MIXER* mixer)
{
    if (!mixer || !stream) {
        return false;
    }
    
    MixerWrapper* wrapper = reinterpret_cast<MixerWrapper*>(mixer);
    wrapper->attached_audio_stream = stream;
    
    return true;
}

bool al_mixer_detach_audio_stream(ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return false;
    }
    
    MixerWrapper* wrapper = reinterpret_cast<MixerWrapper*>(mixer);
    
    if (!wrapper->attached_audio_stream) {
        return false;
    }
    
    al_set_audio_stream_playing(wrapper->attached_audio_stream, false);
    al_destroy_audio_stream(wrapper->attached_audio_stream);
    wrapper->attached_audio_stream = nullptr;
    
    return true;
}

unsigned int al_get_mixer_frequency(const ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return 0;
    }
    
    const MixerWrapper* wrapper = reinterpret_cast<const MixerWrapper*>(mixer);
    return wrapper->frequency;
}

ALLEGRO_CHANNEL_CONF al_get_mixer_channels(const ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return ALLEGRO_CHANNEL_CONF_2;
    }
    
    const MixerWrapper* wrapper = reinterpret_cast<const MixerWrapper*>(mixer);
    return wrapper->channels;
}

ALLEGRO_AUDIO_DEPTH al_get_mixer_depth(const ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return ALLEGRO_AUDIO_DEPTH_INT16;
    }
    
    const MixerWrapper* wrapper = reinterpret_cast<const MixerWrapper*>(mixer);
    return wrapper->depth;
}

ALLEGRO_MIXER_QUALITY al_get_mixer_quality(const ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return ALLEGRO_MIXER_QUALITY_MEDIUM;
    }
    
    const MixerWrapper* wrapper = reinterpret_cast<const MixerWrapper*>(mixer);
    return wrapper->quality;
}

float al_get_mixer_gain(const ALLEGRO_MIXER* mixer)
{
    return 1.0f;
}

bool al_get_mixer_playing(const ALLEGRO_MIXER* mixer)
{
    return true;
}

bool al_get_mixer_attached(const ALLEGRO_MIXER* mixer)
{
    return false;
}

bool al_set_mixer_frequency(ALLEGRO_MIXER* mixer, unsigned int val)
{
    if (!mixer) {
        return false;
    }
    
    MixerWrapper* wrapper = reinterpret_cast<MixerWrapper*>(mixer);
    wrapper->frequency = val;
    return true;
}

bool al_set_mixer_quality(ALLEGRO_MIXER* mixer, ALLEGRO_MIXER_QUALITY val)
{
    if (!mixer) {
        return false;
    }
    
    MixerWrapper* wrapper = reinterpret_cast<MixerWrapper*>(mixer);
    wrapper->quality = val;
    return true;
}

bool al_set_mixer_gain(ALLEGRO_MIXER* mixer, float gain)
{
    return true;
}

bool al_set_mixer_playing(ALLEGRO_MIXER* mixer, bool val)
{
    return true;
}

bool al_detach_mixer(ALLEGRO_MIXER* mixer)
{
    return false;
}

ALLEGRO_VOICE* al_create_voice(unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf)
{
    if (!_audio_installed) {
        return nullptr;
    }
    
    ALLEGRO_VOICE* voice = new ALLEGRO_VOICE;
    if (!voice) {
        return nullptr;
    }
    
    voice->frequency = freq;
    voice->depth = depth;
    voice->chan_conf = chan_conf;
    voice->is_playing = false;
    voice->position = 0;
    voice->source = nullptr;
    voice->source_type = ALLEGRO_VOICE::SOURCE_NONE;
    
    return voice;
}

void al_destroy_voice(ALLEGRO_VOICE* voice)
{
    if (!voice) {
        return;
    }
    
    if (voice->source_type == ALLEGRO_VOICE::SOURCE_SAMPLE) {
        AllegroSampleInstance* spl = reinterpret_cast<AllegroSampleInstance*>(voice->source);
        if (spl && spl->channel >= 0) {
            Mix_HaltChannel(spl->channel);
            spl->channel = -1;
            spl->is_playing = false;
        }
    } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_STREAM) {
        AllegroAudioStream* stream = reinterpret_cast<AllegroAudioStream*>(voice->source);
        if (stream && stream->music) {
            Mix_HaltMusic();
        }
    }
    
    delete voice;
}

bool al_attach_sample_instance_to_voice(ALLEGRO_SAMPLE_INSTANCE* stream, ALLEGRO_VOICE* voice)
{
    if (!stream || !voice) {
        return false;
    }
    
    AllegroSampleInstance* spl = reinterpret_cast<AllegroSampleInstance*>(stream);
    
    if (voice->source_type != ALLEGRO_VOICE::SOURCE_NONE) {
        if (voice->source_type == ALLEGRO_VOICE::SOURCE_SAMPLE) {
            AllegroSampleInstance* old = reinterpret_cast<AllegroSampleInstance*>(voice->source);
            if (old && old->channel >= 0) {
                Mix_HaltChannel(old->channel);
                old->channel = -1;
                old->is_playing = false;
            }
        } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_STREAM) {
            Mix_HaltMusic();
        }
    }
    
    voice->source = spl;
    voice->source_type = ALLEGRO_VOICE::SOURCE_SAMPLE;
    voice->position = 0;
    voice->is_playing = spl->is_playing;
    
    return true;
}

bool al_attach_sample_to_voice(ALLEGRO_SAMPLE* sample, ALLEGRO_VOICE* voice)
{
    if (!sample || !voice) {
        return false;
    }
    
    Mix_Chunk* chunk = static_cast<Mix_Chunk*>(sample->data);
    if (!chunk) {
        return false;
    }
    
    if (voice->source_type != ALLEGRO_VOICE::SOURCE_NONE) {
        if (voice->source_type == ALLEGRO_VOICE::SOURCE_SAMPLE) {
            AllegroSampleInstance* old = reinterpret_cast<AllegroSampleInstance*>(voice->source);
            if (old && old->channel >= 0) {
                Mix_HaltChannel(old->channel);
                old->channel = -1;
                old->is_playing = false;
            }
        } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_STREAM) {
            Mix_HaltMusic();
        }
    }
    
    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel < 0) {
        return false;
    }
    
    static AllegroSampleInstance temp_instance;
    temp_instance.chunk = chunk;
    temp_instance.channel = channel;
    temp_instance.is_playing = true;
    temp_instance.loop = ALLEGRO_PLAYMODE_ONCE;
    temp_instance.sample = sample;
    
    voice->source = &temp_instance;
    voice->source_type = ALLEGRO_VOICE::SOURCE_SAMPLE;
    voice->position = 0;
    voice->is_playing = true;
    
    return true;
}

bool al_attach_audio_stream_to_voice(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_VOICE* voice)
{
    if (!stream || !voice) {
        return false;
    }
    
    AllegroAudioStream* audio_stream = reinterpret_cast<AllegroAudioStream*>(stream);
    
    if (voice->source_type != ALLEGRO_VOICE::SOURCE_NONE) {
        if (voice->source_type == ALLEGRO_VOICE::SOURCE_SAMPLE) {
            AllegroSampleInstance* old = reinterpret_cast<AllegroSampleInstance*>(voice->source);
            if (old && old->channel >= 0) {
                Mix_HaltChannel(old->channel);
                old->channel = -1;
                old->is_playing = false;
            }
        } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_STREAM) {
            Mix_HaltMusic();
        }
    }
    
    if (audio_stream->music) {
        Mix_HaltMusic();
        int loops = (audio_stream->loop == ALLEGRO_PLAYMODE_LOOP) ? -1 : 0;
        Mix_PlayMusic(audio_stream->music, loops);
        audio_stream->is_playing = true;
    }
    
    voice->source = audio_stream;
    voice->source_type = ALLEGRO_VOICE::SOURCE_STREAM;
    voice->position = 0;
    voice->is_playing = audio_stream->is_playing;
    
    return true;
}

bool al_attach_mixer_to_voice(ALLEGRO_MIXER* mixer, ALLEGRO_VOICE* voice)
{
    return false;
}

void al_detach_voice(ALLEGRO_VOICE* voice)
{
}

unsigned int al_get_voice_frequency(const ALLEGRO_VOICE* voice)
{
    if (!voice) {
        return 0;
    }
    return voice->frequency;
}

unsigned int al_get_voice_position(const ALLEGRO_VOICE* voice)
{
    if (!voice) {
        return 0;
    }
    return voice->position;
}

ALLEGRO_CHANNEL_CONF al_get_voice_channels(const ALLEGRO_VOICE* voice)
{
    if (!voice) {
        return ALLEGRO_CHANNEL_CONF_2;
    }
    return static_cast<ALLEGRO_CHANNEL_CONF>(voice->chan_conf);
}

ALLEGRO_AUDIO_DEPTH al_get_voice_depth(const ALLEGRO_VOICE* voice)
{
    if (!voice) {
        return ALLEGRO_AUDIO_DEPTH_INT16;
    }
    return static_cast<ALLEGRO_AUDIO_DEPTH>(voice->depth);
}

bool al_get_voice_playing(const ALLEGRO_VOICE* voice)
{
    if (!voice) {
        return false;
    }
    
    if (voice->source_type == ALLEGRO_VOICE::SOURCE_SAMPLE) {
        AllegroSampleInstance* spl = reinterpret_cast<AllegroSampleInstance*>(voice->source);
        if (spl && spl->channel >= 0) {
            return Mix_Playing(spl->channel) != 0;
        }
    } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_STREAM) {
        return Mix_PlayingMusic() != 0;
    }
    
    return voice->is_playing;
}

bool al_set_voice_position(ALLEGRO_VOICE* voice, unsigned int pos)
{
    if (!voice) {
        return false;
    }
    voice->position = pos;
    return true;
}

bool al_set_voice_playing(ALLEGRO_VOICE* voice, bool val)
{
    if (!voice) {
        return false;
    }
    
    if (voice->source_type == ALLEGRO_VOICE::SOURCE_SAMPLE) {
        AllegroSampleInstance* spl = reinterpret_cast<AllegroSampleInstance*>(voice->source);
        if (spl && spl->channel >= 0) {
            if (val && !Mix_Playing(spl->channel)) {
                Mix_PlayChannel(spl->channel, spl->chunk, spl->loop == ALLEGRO_PLAYMODE_LOOP ? -1 : 0);
                spl->is_playing = true;
            } else if (!val && Mix_Playing(spl->channel)) {
                Mix_HaltChannel(spl->channel);
                spl->is_playing = false;
            }
        }
    } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_STREAM) {
        if (val) {
            Mix_ResumeMusic();
        } else {
            Mix_PauseMusic();
        }
    }
    
    voice->is_playing = val;
    return true;
}

void al_voice_stop(ALLEGRO_VOICE* voice)
{
    if (!voice) {
        return;
    }
    
    if (voice->source_type == ALLEGRO_VOICE::SOURCE_SAMPLE) {
        AllegroSampleInstance* spl = reinterpret_cast<AllegroSampleInstance*>(voice->source);
        if (spl && spl->channel >= 0) {
            Mix_HaltChannel(spl->channel);
            spl->channel = -1;
            spl->is_playing = false;
        }
    } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_STREAM) {
        Mix_HaltMusic();
    } else if (voice->source_type == ALLEGRO_VOICE::SOURCE_MIXER) {
        Mix_HaltMusic();
    }
    
    voice->is_playing = false;
}

ALLEGRO_AUDIO_STREAM* al_create_audio_stream(size_t buffer_count, unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf)
{
    return nullptr;
}

void al_destroy_audio_stream(ALLEGRO_AUDIO_STREAM* stream)
{
}

void al_drain_audio_stream(ALLEGRO_AUDIO_STREAM* stream)
{
}

ALLEGRO_AUDIO_STREAM* al_load_audio_stream(const char* filename, size_t buffer_count, unsigned int samples)
{
    if (!filename || !_audio_installed) {
        return nullptr;
    }
    
    Mix_Music* music = Mix_LoadMUS(filename);
    if (!music) {
        return nullptr;
    }
    
    AllegroAudioStream* stream = new AllegroAudioStream;
    if (!stream) {
        Mix_FreeMusic(music);
        return nullptr;
    }
    
    stream->music = music;
    stream->is_playing = false;
    stream->loop = ALLEGRO_PLAYMODE_ONCE;
    stream->gain = 1.0f;
    stream->pan = 0.0f;
    stream->speed = 1.0f;
    stream->frequency = 44100;
    stream->depth = ALLEGRO_AUDIO_DEPTH_INT16;
    stream->channels = ALLEGRO_CHANNEL_CONF_2;
    stream->buffer_samples = samples;
    stream->buffer_count = buffer_count;
    
    return reinterpret_cast<ALLEGRO_AUDIO_STREAM*>(stream);
}

ALLEGRO_AUDIO_STREAM* al_load_audio_stream_f(ALLEGRO_FILE* fp, const char* ident, size_t buffer_count, unsigned int samples)
{
    if (!fp || !fp->fp || !_audio_installed) {
        return nullptr;
    }
    
    SDL_RWops* rw = SDL_RWFromFP(fp->fp, SDL_FALSE);
    if (!rw) {
        return nullptr;
    }
    
    Mix_Music* music = Mix_LoadMUS_RW(rw, 0);
    if (!music) {
        return nullptr;
    }
    
    AllegroAudioStream* stream = new AllegroAudioStream;
    if (!stream) {
        Mix_FreeMusic(music);
        return nullptr;
    }
    
    stream->music = music;
    stream->is_playing = false;
    stream->loop = ALLEGRO_PLAYMODE_ONCE;
    stream->gain = 1.0f;
    stream->pan = 0.0f;
    stream->speed = 1.0f;
    stream->frequency = 44100;
    stream->depth = ALLEGRO_AUDIO_DEPTH_INT16;
    stream->channels = ALLEGRO_CHANNEL_CONF_2;
    stream->buffer_samples = samples;
    stream->buffer_count = buffer_count;
    
    return reinterpret_cast<ALLEGRO_AUDIO_STREAM*>(stream);
}

unsigned int al_get_audio_stream_frequency(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return 0;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->frequency;
}

unsigned int al_get_audio_stream_length(const ALLEGRO_AUDIO_STREAM* stream)
{
    return 0;
}

unsigned int al_get_audio_stream_fragments(const ALLEGRO_AUDIO_STREAM* stream)
{
    return 0;
}

unsigned int al_get_available_audio_stream_fragments(const ALLEGRO_AUDIO_STREAM* stream)
{
    return 0;
}

float al_get_audio_stream_speed(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return 1.0f;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->speed;
}

bool al_set_audio_stream_speed(ALLEGRO_AUDIO_STREAM* stream, float val)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    s->speed = val;
    return true;
}

float al_get_audio_stream_gain(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return 1.0f;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->gain;
}

bool al_set_audio_stream_gain(ALLEGRO_AUDIO_STREAM* stream, float val)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    s->gain = val;
    return true;
}

float al_get_audio_stream_pan(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return 0.0f;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->pan;
}

bool al_set_audio_stream_pan(ALLEGRO_AUDIO_STREAM* stream, float val)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    s->pan = val;
    return true;
}

ALLEGRO_CHANNEL_CONF al_get_audio_stream_channels(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return ALLEGRO_CHANNEL_CONF_2;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->channels;
}

ALLEGRO_AUDIO_DEPTH al_get_audio_stream_depth(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return ALLEGRO_AUDIO_DEPTH_INT16;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->depth;
}

ALLEGRO_PLAYMODE al_get_audio_stream_playmode(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return ALLEGRO_PLAYMODE_ONCE;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->loop;
}

bool al_set_audio_stream_playmode(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_PLAYMODE val)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    s->loop = val;
    return true;
}

bool al_get_audio_stream_playing(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return false;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    return s->is_playing;
}

bool al_set_audio_stream_playing(ALLEGRO_AUDIO_STREAM* stream, bool val)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    
    if (val) {
        if (s->music && !s->is_playing) {
            int loops = (s->loop == ALLEGRO_PLAYMODE_LOOP) ? -1 : 0;
            if (Mix_PlayMusic(s->music, loops) == 0) {
                s->is_playing = true;
                return true;
            }
        }
        return false;
    } else {
        if (s->is_playing) {
            Mix_PauseMusic();
            s->is_playing = false;
            return true;
        }
        return true;
    }
}

bool al_get_audio_stream_attached(const ALLEGRO_AUDIO_STREAM* stream)
{
    return false;
}

bool al_detach_audio_stream(ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    
    if (s->music) {
        Mix_HaltMusic();
        s->is_playing = false;
    }
    
    return true;
}

uint64_t al_get_audio_stream_played_samples(const ALLEGRO_AUDIO_STREAM* stream)
{
    return 0;
}

void* al_get_audio_stream_fragment(const ALLEGRO_AUDIO_STREAM* stream)
{
    return nullptr;
}

bool al_set_audio_stream_fragment(ALLEGRO_AUDIO_STREAM* stream, void* val)
{
    return false;
}

bool al_rewind_audio_stream(ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    
    if (s->music) {
        Mix_RewindMusic();
        return true;
    }
    
    return false;
}

bool al_seek_audio_stream_secs(ALLEGRO_AUDIO_STREAM* stream, double time)
{
    if (!stream) {
        return false;
    }
    
    AllegroAudioStream* s = reinterpret_cast<AllegroAudioStream*>(stream);
    
    if (s->music) {
        Mix_RewindMusic();
        return true;
    }
    
    return false;
}

double al_get_audio_stream_position_secs(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return 0.0;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    
    if (s->music) {
        return Mix_GetMusicPosition(s->music) / 1000.0;
    }
    
    return 0.0;
}

double al_get_audio_stream_length_secs(const ALLEGRO_AUDIO_STREAM* stream)
{
    if (!stream) {
        return 0.0;
    }
    
    const AllegroAudioStream* s = reinterpret_cast<const AllegroAudioStream*>(stream);
    
    if (s->music) {
        return Mix_MusicDuration(s->music);
    }
    
    return 0.0;
}

ALLEGRO_MIXER* al_get_default_mixer(void)
{
    if (!_audio_installed) {
        return nullptr;
    }
    
    if (!_default_mixer) {
        _default_mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
    }
    
    return _default_mixer;
}

bool al_set_default_mixer(ALLEGRO_MIXER* mixer)
{
    if (!mixer) {
        return false;
    }
    
    _default_mixer = mixer;
    return true;
}

bool al_restore_default_mixer(void)
{
    if (_default_mixer) {
        al_destroy_mixer(_default_mixer);
        _default_mixer = nullptr;
    }
    
    _default_mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
    return _default_mixer != nullptr;
}

bool al_play_sample(ALLEGRO_SAMPLE* data, float gain, float pan, float speed, ALLEGRO_PLAYMODE loop, ALLEGRO_SAMPLE_ID* ret_id)
{
    if (!data || !_audio_installed) {
        return false;
    }
    
    Mix_Chunk* chunk = static_cast<Mix_Chunk*>(data->data);
    if (!chunk) {
        return false;
    }
    
    int loops = (loop == ALLEGRO_PLAYMODE_LOOP) ? -1 : 0;
    int channel = Mix_PlayChannel(-1, chunk, loops);
    
    if (channel < 0) {
        return false;
    }
    
    Mix_Volume(channel, static_cast<int>(gain * 128.0f));
    
    if (ret_id) {
        ret_id->_index = channel;
        ret_id->_id = 0;
    }
    
    return true;
}

void al_stop_sample(ALLEGRO_SAMPLE_ID* spl_id)
{
    if (!spl_id) {
        return;
    }
    
    Mix_HaltChannel(spl_id->_index);
}

void al_stop_samples(void)
{
    Mix_HaltChannel(-1);
}

static bool _timer_installed = false;
static std::vector<ALLEGRO_TIMER*> _timers;

struct ALLEGRO_TIMER {
    double speed;
    long long count;
    bool started;
    bool should_stop;
    SDL_Thread* thread;
    SDL_TimerID sdl_timer_id;
    ALLEGRO_EVENT_SOURCE event_source;
    SDL_mutex* mutex;
};

static Uint32 timer_callback(Uint32 interval, void* param)
{
    ALLEGRO_TIMER* timer = static_cast<ALLEGRO_TIMER*>(param);
    if (!timer) {
        return 0;
    }
    
    SDL_LockMutex(timer->mutex);
    timer->count++;
    SDL_UnlockMutex(timer->mutex);
    
    return static_cast<Uint32>(timer->speed * 1000.0);
}

bool al_install_timer(void)
{
    if (_timer_installed) {
        return true;
    }
    
    if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0) {
        return false;
    }
    
    _timer_installed = true;
    return true;
}

void al_uninstall_timer(void)
{
    if (!_timer_installed) {
        return;
    }
    
    for (ALLEGRO_TIMER* timer : _timers) {
        if (timer->started) {
            al_stop_timer(timer);
        }
        al_destroy_timer(timer);
    }
    
    _timers.clear();
    
    SDL_QuitSubSystem(SDL_INIT_TIMER);
    _timer_installed = false;
}

ALLEGRO_TIMER* al_create_timer(double speed_secs)
{
    if (speed_secs <= 0) {
        return nullptr;
    }
    
    ALLEGRO_TIMER* timer = new ALLEGRO_TIMER;
    if (!timer) {
        return nullptr;
    }
    
    timer->speed = speed_secs;
    timer->count = 0;
    timer->started = false;
    timer->should_stop = false;
    timer->thread = nullptr;
    timer->sdl_timer_id = 0;
    timer->mutex = SDL_CreateMutex();
    
    al_init_event_source(&timer->event_source);
    
    _timers.push_back(timer);
    
    return timer;
}

void al_destroy_timer(ALLEGRO_TIMER* timer)
{
    if (!timer) {
        return;
    }
    
    if (timer->started) {
        al_stop_timer(timer);
    }
    
    for (auto it = _timers.begin(); it != _timers.end(); ++it) {
        if (*it == timer) {
            _timers.erase(it);
            break;
        }
    }
    
    al_destroy_event_source(&timer->event_source);
    
    if (timer->mutex) {
        SDL_DestroyMutex(timer->mutex);
    }
    
    delete timer;
}

void al_start_timer(ALLEGRO_TIMER* timer)
{
    if (!timer || timer->started) {
        return;
    }
    
    timer->started = true;
    timer->should_stop = false;
    
    Uint32 interval = static_cast<Uint32>(timer->speed * 1000.0);
    if (interval < 1) {
        interval = 1;
    }
    
    timer->sdl_timer_id = SDL_AddTimer(interval, timer_callback, timer);
}

void al_stop_timer(ALLEGRO_TIMER* timer)
{
    if (!timer || !timer->started) {
        return;
    }
    
    timer->should_stop = true;
    
    if (timer->sdl_timer_id != 0) {
        SDL_RemoveTimer(timer->sdl_timer_id);
        timer->sdl_timer_id = 0;
    }
    
    timer->started = false;
}

bool al_get_timer_started(ALLEGRO_TIMER* timer)
{
    if (!timer) {
        return false;
    }
    return timer->started;
}

double al_get_timer_speed(ALLEGRO_TIMER* timer)
{
    if (!timer) {
        return 0.0;
    }
    return timer->speed;
}

void al_set_timer_speed(ALLEGRO_TIMER* timer, double speed_secs)
{
    if (!timer || speed_secs <= 0) {
        return;
    }
    
    bool was_started = timer->started;
    
    if (was_started) {
        al_stop_timer(timer);
    }
    
    timer->speed = speed_secs;
    
    if (was_started) {
        al_start_timer(timer);
    }
}

long long al_get_timer_count(ALLEGRO_TIMER* timer)
{
    if (!timer) {
        return 0;
    }
    
    SDL_LockMutex(timer->mutex);
    long long count = timer->count;
    SDL_UnlockMutex(timer->mutex);
    
    return count;
}

void al_set_timer_count(ALLEGRO_TIMER* timer, long long count)
{
    if (!timer) {
        return;
    }
    
    SDL_LockMutex(timer->mutex);
    timer->count = count;
    SDL_UnlockMutex(timer->mutex);
}

void al_add_timer_count(ALLEGRO_TIMER* timer, long long diff)
{
    if (!timer) {
        return;
    }
    
    SDL_LockMutex(timer->mutex);
    timer->count += diff;
    SDL_UnlockMutex(timer->mutex);
}

ALLEGRO_EVENT_SOURCE* al_get_timer_event_source(ALLEGRO_TIMER* timer)
{
    if (!timer) {
        return nullptr;
    }
    return &timer->event_source;
}

ALLEGRO_CONFIG* al_create_config(void)
{
    AllegroConfig* config = new AllegroConfig;
    if (!config) {
        return nullptr;
    }
    
    AllegroConfigSection* general = new AllegroConfigSection;
    if (!general) {
        delete config;
        return nullptr;
    }
    
    general->name = "general";
    config->sections["general"] = general;
    
    return reinterpret_cast<ALLEGRO_CONFIG*>(config);
}

void al_destroy_config(ALLEGRO_CONFIG* config)
{
    if (!config) {
        return;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(config);
    for (auto& pair : cfg->sections) {
        delete pair.second;
    }
    cfg->sections.clear();
    delete cfg;
}

const char* al_get_config_value(const ALLEGRO_CONFIG* config, const char* section, const char* key, const char* default_value)
{
    if (!config || !key) {
        return default_value;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(const_cast<ALLEGRO_CONFIG*>(config));
    
    std::string section_name = section ? section : "general";
    auto section_it = cfg->sections.find(section_name);
    if (section_it == cfg->sections.end()) {
        return default_value;
    }
    
    AllegroConfigSection* sec = section_it->second;
    auto entry_it = sec->entries.find(key);
    if (entry_it == sec->entries.end()) {
        return default_value;
    }
    
    return entry_it->second.c_str();
}

void al_set_config_value(ALLEGRO_CONFIG* config, const char* section, const char* key, const char* value)
{
    if (!config || !key) {
        return;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(config);
    std::string section_name = section ? section : "general";
    
    auto section_it = cfg->sections.find(section_name);
    if (section_it == cfg->sections.end()) {
        AllegroConfigSection* new_section = new AllegroConfigSection;
        new_section->name = section_name;
        cfg->sections[section_name] = new_section;
        section_it = cfg->sections.find(section_name);
    }
    
    AllegroConfigSection* sec = section_it->second;
    std::string value_str = value ? value : "";
    sec->entries[key] = value_str;
}

void al_add_config_section(ALLEGRO_CONFIG* config, const char* name)
{
    if (!config || !name) {
        return;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(config);
    
    if (cfg->sections.find(name) != cfg->sections.end()) {
        return;
    }
    
    AllegroConfigSection* section = new AllegroConfigSection;
    section->name = name;
    cfg->sections[name] = section;
}

const char* al_get_first_config_section(const ALLEGRO_CONFIG* config, ALLEGRO_CONFIG_SECTION** iterator)
{
    if (!config || !iterator) {
        return nullptr;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(const_cast<ALLEGRO_CONFIG*>(config));
    
    if (cfg->sections.empty()) {
        *iterator = nullptr;
        return nullptr;
    }
    
    auto it = cfg->sections.begin();
    *iterator = reinterpret_cast<ALLEGRO_CONFIG_SECTION*>(it->second);
    return it->first.c_str();
}

const char* al_get_next_config_section(ALLEGRO_CONFIG_SECTION** iterator)
{
    if (!iterator || !*iterator) {
        return nullptr;
    }
    
    return nullptr;
}

const char* al_get_first_config_entry(const ALLEGRO_CONFIG* config, const char* section, ALLEGRO_CONFIG_ENTRY** iterator)
{
    if (!config || !section || !iterator) {
        return nullptr;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(const_cast<ALLEGRO_CONFIG*>(config));
    auto section_it = cfg->sections.find(section);
    
    if (section_it == cfg->sections.end()) {
        *iterator = nullptr;
        return nullptr;
    }
    
    AllegroConfigSection* sec = section_it->second;
    
    if (sec->entries.empty()) {
        *iterator = nullptr;
        return nullptr;
    }
    
    auto it = sec->entries.begin();
    *iterator = reinterpret_cast<ALLEGRO_CONFIG_ENTRY*>(const_cast<std::string*>(&it->first));
    return it->first.c_str();
}

const char* al_get_next_config_entry(ALLEGRO_CONFIG_ENTRY** iterator)
{
    (void)iterator;
    return nullptr;
}

static void _trim_string(std::string& str)
{
    size_t start = 0;
    while (start < str.length() && (str[start] == ' ' || str[start] == '\t')) {
        start++;
    }
    size_t end = str.length();
    while (end > start && (str[end-1] == ' ' || str[end-1] == '\t' || str[end-1] == '\r' || str[end-1] == '\n')) {
        end--;
    }
    if (start > 0 || end < str.length()) {
        str = str.substr(start, end - start);
    }
}

ALLEGRO_CONFIG* al_load_config_file(const char* filename)
{
    if (!filename) {
        return nullptr;
    }
    
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return nullptr;
    }
    
    AllegroConfig* config = new AllegroConfig;
    if (!config) {
        fclose(fp);
        return nullptr;
    }
    
    config->filename = filename;
    
    AllegroConfigSection* current_section = new AllegroConfigSection;
    current_section->name = "general";
    config->sections["general"] = current_section;
    
    char line[4096];
    while (fgets(line, sizeof(line), fp)) {
        std::string s = line;
        _trim_string(s);
        
        if (s.empty()) {
            continue;
        }
        
        if (s[0] == ';' || s[0] == '#') {
            continue;
        }
        
        if (s[0] == '[') {
            size_t end_bracket = s.find(']');
            if (end_bracket != std::string::npos) {
                std::string section_name = s.substr(1, end_bracket - 1);
                _trim_string(section_name);
                
                if (config->sections.find(section_name) == config->sections.end()) {
                    AllegroConfigSection* new_section = new AllegroConfigSection;
                    new_section->name = section_name;
                    config->sections[section_name] = new_section;
                }
                current_section = config->sections[section_name];
            }
            continue;
        }
        
        size_t eq_pos = s.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = s.substr(0, eq_pos);
            std::string value = s.substr(eq_pos + 1);
            _trim_string(key);
            _trim_string(value);
            
            if (!key.empty()) {
                current_section->entries[key] = value;
            }
        }
    }
    
    fclose(fp);
    return reinterpret_cast<ALLEGRO_CONFIG*>(config);
}

ALLEGRO_CONFIG* al_load_config_f(ALLEGRO_FILE* fp, const char* origin)
{
    (void)origin;
    
    if (!fp || !fp->fp) {
        return nullptr;
    }
    
    AllegroConfig* config = new AllegroConfig;
    if (!config) {
        return nullptr;
    }
    
    if (origin) {
        config->filename = origin;
    }
    
    AllegroConfigSection* current_section = new AllegroConfigSection;
    current_section->name = "general";
    config->sections["general"] = current_section;
    
    char line[4096];
    while (fgets(line, sizeof(line), fp->fp)) {
        std::string s = line;
        _trim_string(s);
        
        if (s.empty()) {
            continue;
        }
        
        if (s[0] == ';' || s[0] == '#') {
            continue;
        }
        
        if (s[0] == '[') {
            size_t end_bracket = s.find(']');
            if (end_bracket != std::string::npos) {
                std::string section_name = s.substr(1, end_bracket - 1);
                _trim_string(section_name);
                
                if (config->sections.find(section_name) == config->sections.end()) {
                    AllegroConfigSection* new_section = new AllegroConfigSection;
                    new_section->name = section_name;
                    config->sections[section_name] = new_section;
                }
                current_section = config->sections[section_name];
            }
            continue;
        }
        
        size_t eq_pos = s.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = s.substr(0, eq_pos);
            std::string value = s.substr(eq_pos + 1);
            _trim_string(key);
            _trim_string(value);
            
            if (!key.empty()) {
                current_section->entries[key] = value;
            }
        }
    }
    
    return reinterpret_cast<ALLEGRO_CONFIG*>(config);
}

bool al_save_config_file(const char* filename, const ALLEGRO_CONFIG* config)
{
    if (!filename || !config) {
        return false;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(const_cast<ALLEGRO_CONFIG*>(config));
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        return false;
    }
    
    for (auto& section_pair : cfg->sections) {
        AllegroConfigSection* section = section_pair.second;
        
        fprintf(fp, "[%s]\n", section->name.c_str());
        
        for (auto& entry : section->entries) {
            fprintf(fp, "%s=%s\n", entry.first.c_str(), entry.second.c_str());
        }
        
        fprintf(fp, "\n");
    }
    
    fclose(fp);
    return true;
}

bool al_save_config_f(ALLEGRO_FILE* fp, const ALLEGRO_CONFIG* config)
{
    if (!fp || !fp->fp || !config) {
        return false;
    }
    
    AllegroConfig* cfg = reinterpret_cast<AllegroConfig*>(const_cast<ALLEGRO_CONFIG*>(config));
    
    for (auto& section_pair : cfg->sections) {
        AllegroConfigSection* section = section_pair.second;
        
        fprintf(fp->fp, "[%s]\n", section->name.c_str());
        
        for (auto& entry : section->entries) {
            fprintf(fp->fp, "%s=%s\n", entry.first.c_str(), entry.second.c_str());
        }
        
        fprintf(fp->fp, "\n");
    }
    
    return true;
}
