// allegro_shim - Allegro 5 to SDL2 shim layer

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <allegro5/allegro_display.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_bitmap.h>
#include <allegro5/allegro_draw.h>
#include <allegro5/internal/allegro_display.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>

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
