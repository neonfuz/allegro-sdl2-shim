# Graphics Module Implementation Plan

This document details the SDL2 implementation approach for the Allegro 5 Graphics Module.

## Type Mappings

| Allegro Type | SDL2 Equivalent | Description |
|--------------|-----------------|-------------|
| `ALLEGRO_BITMAP` | `SDL_Texture*` | GPU texture for rendering |
| `ALLEGRO_COLOR` | `struct { float r, g, b, a; }` | RGBA color with float components |
| `ALLEGRO_FONT` | `TTF_Font*` | TrueType font handle |
| `ALLEGRO_DISPLAY` | `SDL_Window*` + `SDL_Renderer*` | Window and rendering context |

## Global State

The graphics module requires access to global state managed in core.cpp:

```cpp
struct GraphicsState {
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* current_target = nullptr;  // Current render target (null = screen)
    int new_bitmap_format = ALLEGRO_PIXEL_FORMAT_ANY;
    int new_bitmap_flags = 0;
    SDL_Rect clipping_rect = {0, 0, 0, 0};
    bool clipping_enabled = false;
};
```

---

## Bitmap Management

### al_create_bitmap

**Function Signature:**
```c
ALLEGRO_BITMAP* al_create_bitmap(int w, int h);
```

**SDL2 Implementation:**
```c
ALLEGRO_BITMAP* al_create_bitmap(int w, int h) {
    SDL_Texture* texture = SDL_CreateTexture(
        shim.state.renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    return texture;
}
```

**Implementation Details:**
- Create SDL_Texture with TARGET access for render-to-texture capability
- Set RGBA8888 format for 32-bit color with alpha
- Enable blending for alpha transparency
- Store in wrapper struct if additional metadata needed

---

### al_destroy_bitmap

**Function Signature:**
```c
void al_destroy_bitmap(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
void al_destroy_bitmap(ALLEGRO_BITMAP* bitmap) {
    if (bitmap) {
        SDL_DestroyTexture(bitmap);
    }
}
```

**Implementation Details:**
- Simply destroy the underlying SDL_Texture
- Handle null pointer gracefully

---

### al_get_bitmap_width

**Function Signature:**
```c
int al_get_bitmap_width(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
int al_get_bitmap_width(ALLEGRO_BITMAP* bitmap) {
    int w, h;
    SDL_QueryTexture(bitmap, nullptr, nullptr, &w, &h);
    return w;
}
```

---

### al_get_bitmap_height

**Function Signature:**
```c
int al_get_bitmap_height(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
int al_get_bitmap_height(ALLEGRO_BITMAP* bitmap) {
    int w, h;
    SDL_QueryTexture(bitmap, nullptr, nullptr, &w, &h);
    return h;
}
```

---

### al_set_new_bitmap_format / al_get_new_bitmap_format

**Function Signatures:**
```c
void al_set_new_bitmap_format(int format);
int al_get_new_bitmap_format(void);
```

**SDL2 Implementation:**
```c
void al_set_new_bitmap_format(int format) {
    shim.state.new_bitmap_format = format;
}

int al_get_new_bitmap_format(void) {
    return shim.state.new_bitmap_format;
}
```

**Implementation Details:**
- Store format setting for new bitmap creation
- Convert Allegro format to SDL pixel format when creating textures

---

### al_set_new_bitmap_flags / al_get_new_bitmap_flags

**Function Signatures:**
```c
void al_set_new_bitmap_flags(int flags);
int al_get_new_bitmap_flags(void);
```

**SDL2 Implementation:**
```c
void al_set_new_bitmap_flags(int flags) {
    shim.state.new_bitmap_flags = flags;
}

int al_get_new_bitmap_flags(void) {
    return shim.state.new_bitmap_flags;
}
```

---

### al_add_new_bitmap_flag

**Function Signature:**
```c
void al_add_new_bitmap_flag(int flag);
```

**SDL2 Implementation:**
```c
void al_add_new_bitmap_flag(int flag) {
    shim.state.new_bitmap_flags |= flag;
}
```

---

### al_set_target_bitmap

**Function Signature:**
```c
void al_set_target_bitmap(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
void al_set_target_bitmap(ALLEGRO_BITMAP* bitmap) {
    shim.state.current_target = bitmap;
    if (bitmap) {
        SDL_SetRenderTarget(shim.state.renderer, bitmap);
    } else {
        SDL_SetRenderTarget(shim.state.renderer, nullptr);
    }
}
```

**Implementation Details:**
- Set SDL_RenderTarget to the specified texture
- If nullptr, restore rendering to the screen (default display)

---

### al_get_target_bitmap

**Function Signature:**
```c
ALLEGRO_BITMAP* al_get_target_bitmap(void);
```

**SDL2 Implementation:**
```c
ALLEGRO_BITMAP* al_get_target_bitmap(void) {
    return shim.state.current_target;
}
```

---

### al_convert_mask_to_alpha

**Function Signature:**
```c
void al_convert_mask_to_alpha(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR mask_color);
```

**SDL2 Implementation:**
```c
void al_convert_mask_to_alpha(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR mask_color) {
    // Requires locking texture and iterating pixels
    // This is complex in SDL2 - would need SDL_RenderReadPixels
    // For basic implementation, this could be a no-op or use software surface
}
```

**Implementation Details:**
- Convert pixels matching mask_color to transparent
- Requires reading pixels back from GPU, expensive operation
- Consider using SDL_RenderReadPixels with a temporary surface

---

### al_create_sub_bitmap

**Function Signature:**
```c
ALLEGRO_BITMAP* al_create_sub_bitmap(ALLEGRO_BITMAP* parent, int x, int y, int w, int h);
```

**SDL2 Implementation:**
```c
// SDL2 doesn't support true sub-textures natively
// Need to wrap in a struct that tracks parent and offset
struct SubBitmap {
    SDL_Texture* texture;  // Actually points to parent
    int x, y;              // Offset into parent
    int w, h;
    bool is_sub_bitmap;
};
```

**Implementation Details:**
- Create wrapper struct storing parent reference and offset coordinates
- For drawing, adjust coordinates when blitting
- Track sub-bitmap status for proper destruction (don't destroy parent)

---

### al_is_sub_bitmap

**Function Signature:**
```c
bool al_is_sub_bitmap(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
bool al_is_sub_bitmap(ALLEGRO_BITMAP* bitmap) {
    // Check if bitmap wrapper indicates sub-bitmap
    SubBitmap* sb = (SubBitmap*)bitmap;
    return sb && sb->is_sub_bitmap;
}
```

---

### al_get_parent_bitmap

**Function Signature:**
```c
ALLEGRO_BITMAP* al_get_parent_bitmap(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
ALLEGRO_BITMAP* al_get_parent_bitmap(ALLEGRO_BITMAP* bitmap) {
    SubBitmap* sb = (SubBitmap*)bitmap;
    if (sb && sb->is_sub_bitmap) {
        return (ALLEGRO_BITMAP*)sb->texture;
    }
    return nullptr;
}
```

---

### al_get_bitmap_x / al_get_bitmap_y

**Function Signatures:**
```c
int al_get_bitmap_x(ALLEGRO_BITMAP* bitmap);
int al_get_bitmap_y(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
int al_get_bitmap_x(ALLEGRO_BITMAP* bitmap) {
    SubBitmap* sb = (SubBitmap*)bitmap;
    return sb ? sb->x : 0;
}

int al_get_bitmap_y(ALLEGRO_BITMAP* bitmap) {
    SubBitmap* sb = (SubBitmap*)bitmap;
    return sb ? sb->y : 0;
}
```

---

### al_clone_bitmap

**Function Signature:**
```c
ALLEGRO_BITMAP* al_clone_bitmap(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
ALLEGRO_BITMAP* al_clone_bitmap(ALLEGRO_BITMAP* bitmap) {
    int w = al_get_bitmap_width(bitmap);
    int h = al_get_bitmap_height(bitmap);
    SDL_Texture* new_tex = SDL_CreateTexture(
        shim.state.renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );
    
    SDL_Texture* old_target = shim.state.current_target;
    SDL_SetRenderTarget(shim.state.renderer, new_tex);
    SDL_RenderCopy(shim.state.renderer, bitmap, nullptr, nullptr);
    SDL_SetRenderTarget(shim.state.renderer, old_target);
    
    return new_tex;
}
```

---

## Pixel Operations

### al_put_pixel

**Function Signature:**
```c
void al_put_pixel(int x, int y, ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
void al_put_pixel(int x, int y, ALLEGRO_COLOR color) {
    SDL_SetRenderDrawColor(
        shim.state.renderer,
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    );
    SDL_RenderDrawPoint(shim.state.renderer, x, y);
}
```

---

### al_put_blended_pixel

**Function Signature:**
```c
void al_put_blended_pixel(int x, int y, ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
void al_put_blended_pixel(int x, int y, ALLEGRO_COLOR color) {
    // Enable blending for single pixel
    SDL_BlendMode mode;
    SDL_GetRenderDrawBlendMode(shim.state.renderer, &mode);
    SDL_SetRenderDrawBlendMode(shim.state.renderer, SDL_BLENDMODE_BLEND);
    
    al_put_pixel(x, y, color);
    
    SDL_SetRenderDrawBlendMode(shim.state.renderer, mode);
}
```

---

### al_get_pixel

**Function Signature:**
```c
ALLEGRO_COLOR al_get_pixel(ALLEGRO_BITMAP* bitmap, int x, int y);
```

**SDL2 Implementation:**
```c
ALLEGRO_COLOR al_get_pixel(ALLEGRO_BITMAP* bitmap, int x, int y) {
    ALLEGRO_COLOR color = {0, 0, 0, 255};
    
    SDL_Texture* old_target = shim.state.current_target;
    SDL_SetRenderTarget(shim.state.renderer, bitmap);
    
    Uint32 pixels[1];
    SDL_Rect rect = {x, y, 1, 1};
    if (SDL_RenderReadPixels(shim.state.renderer, &rect, 
                             SDL_PIXELFORMAT_RGBA8888, 
                             pixels, sizeof(Uint32)) == 0) {
        color.r = ((pixels[0] >> 24) & 0xFF) / 255.0f;
        color.g = ((pixels[0] >> 16) & 0xFF) / 255.0f;
        color.b = ((pixels[0] >> 8) & 0xFF) / 255.0f;
        color.a = (pixels[0] & 0xFF) / 255.0f;
    }
    
    SDL_SetRenderTarget(shim.state.renderer, old_target);
    return color;
}
```

**Implementation Details:**
- Requires render target switching, expensive operation
- Read 1x1 pixel region via SDL_RenderReadPixels

---

## Clipping

### al_set_clipping_rectangle

**Function Signature:**
```c
void al_set_clipping_rectangle(int x, int y, int width, int height);
```

**SDL2 Implementation:**
```c
void al_set_clipping_rectangle(int x, int y, int width, int height) {
    shim.state.clipping_rect = {x, y, width, height};
    shim.state.clipping_enabled = true;
    SDL_RenderSetClipRect(shim.state.renderer, &shim.state.clipping_rect);
}
```

---

### al_reset_clipping_rectangle

**Function Signature:**
```c
void al_reset_clipping_rectangle(void);
```

**SDL2 Implementation:**
```c
void al_reset_clipping_rectangle(void) {
    shim.state.clipping_enabled = false;
    SDL_RenderSetClipRect(shim.state.renderer, nullptr);
}
```

---

### al_get_clipping_rectangle

**Function Signature:**
```c
void al_get_clipping_rectangle(int* x, int* y, int* w, int* h);
```

**SDL2 Implementation:**
```c
void al_get_clipping_rectangle(int* x, int* y, int* w, int* h) {
    if (shim.state.clipping_enabled) {
        *x = shim.state.clipping_rect.x;
        *y = shim.state.clipping_rect.y;
        *w = shim.state.clipping_rect.w;
        *h = shim.state.clipping_rect.h;
    } else {
        // Return full display size
        int w, h;
        SDL_GetRendererOutputSize(shim.state.renderer, &w, &h);
        *x = *y = 0;
        *w = w;
        *h = h;
    }
}
```

---

## Color Functions

### al_map_rgb

**Function Signature:**
```c
ALLEGRO_COLOR al_map_rgb(unsigned char r, unsigned char g, unsigned char b);
```

**SDL2 Implementation:**
```c
ALLEGRO_COLOR al_map_rgb(unsigned char r, unsigned char g, unsigned char b) {
    ALLEGRO_COLOR color;
    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = 1.0f;
    return color;
}
```

---

### al_map_rgba

**Function Signature:**
```c
ALLEGRO_COLOR al_map_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
```

**SDL2 Implementation:**
```c
ALLEGRO_COLOR al_map_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    ALLEGRO_COLOR color;
    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = a / 255.0f;
    return color;
}
```

---

### al_map_rgb_f

**Function Signature:**
```c
ALLEGRO_COLOR al_map_rgb_f(float r, float g, float b);
```

**SDL2 Implementation:**
```c
ALLEGRO_COLOR al_map_rgb_f(float r, float g, float b) {
    ALLEGRO_COLOR color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 1.0f;
    return color;
}
```

---

### al_map_rgba_f

**Function Signature:**
```c
ALLEGRO_COLOR al_map_rgba_f(float r, float g, float b, float a);
```

**SDL2 Implementation:**
```c
ALLEGRO_COLOR al_map_rgba_f(float r, float g, float b, float a) {
    ALLEGRO_COLOR color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}
```

---

### al_premul_rgba

**Function Signature:**
```c
ALLEGRO_COLOR al_premul_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
```

**SDL2 Implementation:**
```c
ALLEGRO_COLOR al_premul_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    ALLEGRO_COLOR color;
    float alpha = a / 255.0f;
    color.r = (r / 255.0f) * alpha;
    color.g = (g / 255.0f) * alpha;
    color.b = (b / 255.0f) * alpha;
    color.a = alpha;
    return color;
}
```

**Implementation Details:**
- Pre-multiplies color components by alpha value
- Required for correct blending with SDL_BLENDMODE_BLEND

---

### al_premul_rgba_f

**Function Signature:**
```c
ALLEGRO_COLOR al_premul_rgba_f(float r, float g, float b, float a);
```

**SDL2 Implementation:**
```c
ALLEGRO_COLOR al_premul_rgba_f(float r, float g, float b, float a) {
    ALLEGRO_COLOR color;
    color.r = r * a;
    color.g = g * a;
    color.b = b * a;
    color.a = a;
    return color;
}
```

---

### al_unmap_rgb

**Function Signature:**
```c
void al_unmap_rgb(ALLEGRO_COLOR color, unsigned char* r, unsigned char* g, unsigned char* b);
```

**SDL2 Implementation:**
```c
void al_unmap_rgb(ALLEGRO_COLOR color, unsigned char* r, unsigned char* g, unsigned char* b) {
    *r = (unsigned char)(color.r * 255);
    *g = (unsigned char)(color.g * 255);
    *b = (unsigned char)(color.b * 255);
}
```

---

### al_unmap_rgba

**Function Signature:**
```c
void al_unmap_rgba(ALLEGRO_COLOR color, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a);
```

**SDL2 Implementation:**
```c
void al_unmap_rgba(ALLEGRO_COLOR color, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a) {
    *r = (unsigned char)(color.r * 255);
    *g = (unsigned char)(color.g * 255);
    *b = (unsigned char)(color.b * 255);
    *a = (unsigned char)(color.a * 255);
}
```

---

### al_unmap_rgb_f / al_unmap_rgba_f

**Function Signatures:**
```c
void al_unmap_rgb_f(ALLEGRO_COLOR color, float* r, float* g, float* b);
void al_unmap_rgba_f(ALLEGRO_COLOR color, float* r, float* g, float* b, float* a);
```

**SDL2 Implementation:**
```c
void al_unmap_rgb_f(ALLEGRO_COLOR color, float* r, float* g, float* b) {
    *r = color.r;
    *g = color.g;
    *b = color.b;
}

void al_unmap_rgba_f(ALLEGRO_COLOR color, float* r, float* g, float* b, float* a) {
    *r = color.r;
    *g = color.g;
    *b = color.b;
    *a = color.a;
}
```

---

## Drawing Primitives

### al_clear_to_color

**Function Signature:**
```c
void al_clear_to_color(ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
void al_clear_to_color(ALLEGRO_COLOR color) {
    SDL_SetRenderDrawColor(
        shim.state.renderer,
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    );
    SDL_RenderClear(shim.state.renderer);
}
```

---

### al_draw_pixel

**Function Signature:**
```c
void al_draw_pixel(float x, float y, ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
void al_draw_pixel(float x, float y, ALLEGRO_COLOR color) {
    al_put_pixel((int)x, (int)y, color);
}
```

---

### al_draw_filled_rectangle

**Function Signature:**
```c
void al_draw_filled_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
void al_draw_filled_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color) {
    SDL_Rect rect = {
        (int)floorf(x1),
        (int)floorf(y1),
        (int)ceilf(x2 - x1),
        (int)ceilf(y2 - y1)
    };
    SDL_SetRenderDrawColor(
        shim.state.renderer,
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    );
    SDL_RenderFillRect(shim.state.renderer, &rect);
}
```

**Implementation Details:**
- Use floor/ceil to ensure proper rectangle bounds
- SDL2 doesn't support filled rectangles with alpha directly; requires blending mode

---

### al_draw_line

**Function Signature:**
```c
void al_draw_line(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness);
```

**SDL2 Implementation:**
```c
void al_draw_line(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness) {
    SDL_SetRenderDrawColor(
        shim.state.renderer,
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    );
    
    if (thickness <= 1.0f) {
        SDL_RenderDrawLine(shim.state.renderer, 
            (int)x1, (int)y1, (int)x2, (int)y2);
    } else {
        // Implement thick lines using multiple offset lines or geometry
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = sqrtf(dx*dx + dy*dy);
        if (len > 0) {
            float nx = -dy / len * thickness / 2;
            float ny = dx / len * thickness / 2;
            
            // Draw filled quad (two triangles)
            // For simplicity, could draw multiple offset lines
            SDL_RenderDrawLine(shim.state.renderer,
                (int)(x1 + nx), (int)(y1 + ny),
                (int)(x2 + nx), (int)(y2 + ny));
            SDL_RenderDrawLine(shim.state.renderer,
                (int)(x1 - nx), (int)(y1 - ny),
                (int)(x2 - nx), (int)(y2 - ny));
        }
    }
}
```

**Implementation Details:**
- SDL2 only supports 1-pixel thick lines
- For thicker lines, implement using geometry or multiple passes
- For anti-aliased lines, would need SDL2_gfx library

---

### al_draw_circle

**Function Signature:**
```c
void al_draw_circle(float cx, float cy, float r, ALLEGRO_COLOR color, float thickness);
```

**SDL2 Implementation:**
```c
void al_draw_circle(float cx, float cy, float r, ALLEGRO_COLOR color, float thickness) {
    SDL_SetRenderDrawColor(
        shim.state.renderer,
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    );
    
    // Use midpoint circle algorithm
    int ix = (int)r;
    int iy = 0;
    int err = 0;
    
    while (ix >= iy) {
        SDL_RenderDrawPoint(shim.state.renderer, cx + ix, cy + iy);
        SDL_RenderDrawPoint(shim.state.renderer, cx + iy, cy + ix);
        SDL_RenderDrawPoint(shim.state.renderer, cx - iy, cy + ix);
        SDL_RenderDrawPoint(shim.state.renderer, cx - ix, cy + iy);
        SDL_RenderDrawPoint(shim.state.renderer, cx - ix, cy - iy);
        SDL_RenderDrawPoint(shim.state.renderer, cx - iy, cy - ix);
        SDL_RenderDrawPoint(shim.state.renderer, cx + iy, cy - ix);
        SDL_RenderDrawPoint(shim.state.renderer, cx + ix, cy - iy);
        
        if (err <= 0) {
            iy += 1;
            err += 2*iy + 1;
        }
        if (err > 0) {
            ix -= 1;
            err -= 2*ix + 1;
        }
    }
}
```

**Implementation Details:**
- Uses Bresenham's circle algorithm
- For thick circles, draw multiple concentric circles or use SDL2_gfx

---

### al_draw_rectangle

**Function Signature:**
```c
void al_draw_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness);
```

**SDL2 Implementation:**
```c
void al_draw_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color, float thickness) {
    SDL_Rect rect = {
        (int)floorf(x1),
        (int)floorf(y1),
        (int)ceilf(x2 - x1),
        (int)ceilf(y2 - y1)
    };
    SDL_SetRenderDrawColor(
        shim.state.renderer,
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    );
    SDL_RenderDrawRect(shim.state.renderer, &rect);
}
```

---

### al_draw_triangle

**Function Signature:**
```c
void al_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, 
                      ALLEGRO_COLOR color, float thickness);
```

**SDL2 Implementation:**
```c
void al_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3,
                      ALLEGRO_COLOR color, float thickness) {
    al_draw_line(x1, y1, x2, y2, color, thickness);
    al_draw_line(x2, y2, x3, y3, color, thickness);
    al_draw_line(x3, y3, x1, y1, color, thickness);
}
```

---

### al_draw_filled_triangle

**Function Signature:**
```c
void al_draw_filled_triangle(float x1, float y1, float x2, float y2, float x3, float y3, 
                              ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
void al_draw_filled_triangle(float x1, float y1, float x2, float y2, float x3, float y3,
                              ALLEGRO_COLOR color) {
    // Use triangle rasterization
    // Sort vertices by y coordinate
    // Fill using horizontal scanlines
    // For simplicity, could use SDL2_gfx primitives
    
    // Simplified: find bounding box and fill polygon
    int miny = (int)floorf(MIN(MIN(y1, y2), y3));
    int maxy = (int)ceilf(MAX(MAX(y1, y2), y3));
    
    for (int y = miny; y <= maxy; y++) {
        // Find intersections with edges
        // Fill horizontal line between intersections
    }
}
```

---

### al_draw_filled_ellipse / al_draw_ellipse

**Function Signatures:**
```c
void al_draw_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color, float thickness);
void al_draw_filled_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
// Use midpoint ellipse algorithm similar to circle
// For filled: fill scanlines between left and right edges

void al_draw_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color, float thickness) {
    // Bresenham-like ellipse drawing
    // Or use SDL2_gfx: ellipseRGBA
}

void al_draw_filled_ellipse(float cx, float cy, float rx, float ry, ALLEGRO_COLOR color) {
    // Fill using scanlines
}
```

---

### al_draw_arc / al_draw_pieslice

**Function Signatures:**
```c
void al_draw_arc(float cx, float cy, float r, float start_theta, float delta_theta,
                 ALLEGRO_COLOR color, float thickness);
void al_draw_pieslice(float cx, float cy, float r, float start_theta, float delta_theta,
                      ALLEGRO_COLOR color, float thickness);
```

**SDL2 Implementation:**
```c
void al_draw_arc(float cx, float cy, float r, float start_theta, float delta_theta,
                 ALLEGRO_COLOR color, float thickness) {
    int num_points = (int)(r * fabs(delta_theta) / 2) + 10;
    float angle = start_theta;
    float step = delta_theta / num_points;
    
    for (int i = 0; i < num_points; i++) {
        float x1 = cx + r * cosf(angle);
        float y1 = cy + r * sinf(angle);
        angle += step;
        float x2 = cx + r * cosf(angle);
        float y2 = cy + r * sinf(angle);
        al_draw_line(x1, y1, x2, y2, color, thickness);
    }
}

void al_draw_pieslice(float cx, float cy, float r, float start_theta, float delta_theta,
                      ALLEGRO_COLOR color, float thickness) {
    al_draw_arc(cx, cy, r, start_theta, delta_theta, color, thickness);
    al_draw_line(cx, cy, cx + r * cosf(start_theta), cy + r * sinf(start_theta), color, thickness);
    al_draw_line(cx, cy, cx + r * cosf(start_theta + delta_theta), 
                 cy + r * sinf(start_theta + delta_theta), color, thickness);
}
```

---

### al_draw_filled_pieslice

**Function Signature:**
```c
void al_draw_filled_pieslice(float cx, float cy, float r, float start_theta, float delta_theta,
                             ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
void al_draw_filled_pieslice(float cx, float cy, float r, float start_theta, float delta_theta,
                             ALLEGRO_COLOR color) {
    // Draw triangle fan from center
    // Or use polygon filling algorithm
}
```

---

### al_draw_rounded_rectangle / al_draw_filled_rounded_rectangle

**Function Signatures:**
```c
void al_draw_rounded_rectangle(float x1, float y1, float x2, float y2, float rx, float ry,
                               ALLEGRO_COLOR color, float thickness);
void al_draw_filled_rounded_rectangle(float x1, float y1, float x2, float y2, float rx, float ry,
                                       ALLEGRO_COLOR color);
```

**SDL2 Implementation:**
```c
// Draw as combination of:
// - 4 quarter circles at corners
// - 4 rectangles for edges
// - 1 rectangle for center

void al_draw_rounded_rectangle(float x1, float y1, float x2, float y2, float rx, float ry,
                               ALLEGRO_COLOR color, float thickness) {
    // Implementation using arc and line drawing
}
```

---

## Bitmap Drawing

### al_draw_bitmap

**Function Signature:**
```c
void al_draw_bitmap(ALLEGRO_BITMAP* bitmap, float dx, float dy, int flags);
```

**SDL2 Implementation:**
```c
void al_draw_bitmap(ALLEGRO_BITMAP* bitmap, float dx, float dy, int flags) {
    SDL_Rect src_rect = {0, 0, al_get_bitmap_width(bitmap), al_get_bitmap_height(bitmap)};
    SDL_Rect dst_rect = {(int)dx, (int)dy, src_rect.w, src_rect.h};
    
    SDL_RenderCopy(shim.state.renderer, bitmap, &src_rect, &dst_rect);
}
```

**Implementation Details:**
- Apply flip flags via SDL_RenderCopyEx if needed

---

### al_draw_bitmap_region

**Function Signature:**
```c
void al_draw_bitmap_region(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh,
                           float dx, float dy, int flags);
```

**SDL2 Implementation:**
```c
void al_draw_bitmap_region(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh,
                           float dx, float dy, int flags) {
    SDL_Rect src_rect = {(int)sx, (int)sy, (int)sw, (int)sh};
    SDL_Rect dst_rect = {(int)dx, (int)dy, (int)sw, (int)sh};
    
    SDL_RenderCopy(shim.state.renderer, bitmap, &src_rect, &dst_rect);
}
```

---

### al_draw_scaled_bitmap

**Function Signature:**
```c
void al_draw_scaled_bitmap(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh,
                           float dx, float dy, float dw, float dh, int flags);
```

**SDL2 Implementation:**
```c
void al_draw_scaled_bitmap(ALLEGRO_BITMAP* bitmap, float sx, float sy, float sw, float sh,
                           float dx, float dy, float dw, float dh, int flags) {
    SDL_Rect src_rect = {(int)sx, (int)sy, (int)sw, (int)sh};
    SDL_Rect dst_rect = {(int)dx, (int)dy, (int)dw, (int)dh};
    
    SDL_RenderCopy(shim.state.renderer, bitmap, &src_rect, &dst_rect);
}
```

---

### al_draw_rotated_bitmap

**Function Signature:**
```c
void al_draw_rotated_bitmap(ALLEGRO_BITMAP* bitmap, float cx, float cy, float dx, float dy,
                            float angle, int flags);
```

**SDL2 Implementation:**
```c
void al_draw_rotated_bitmap(ALLEGRO_BITMAP* bitmap, float cx, float cy, float dx, float dy,
                            float angle, int flags) {
    SDL_Point center = {(int)cx, (int)cy};
    SDL_Rect dst_rect = {
        (int)(dx - cx),
        (int)(dy - cy),
        al_get_bitmap_width(bitmap),
        al_get_bitmap_height(bitmap)
    };
    
    SDL_RenderCopyEx(shim.state.renderer, bitmap, nullptr, &dst_rect,
                     angle * 180.0f / ALLEGRO_PI, &center, SDL_FLIP_NONE);
}
```

**Implementation Details:**
- SDL2 uses degrees, Allegro uses radians
- Convert angle from radians to degrees

---

### al_draw_scaled_rotated_bitmap

**Function Signature:**
```c
void al_draw_scaled_rotated_bitmap(ALLEGRO_BITMAP* bitmap, float cx, float cy, float dx, float dy,
                                    float xscale, float yscale, float angle, int flags);
```

**SDL2 Implementation:**
```c
void al_draw_scaled_rotated_bitmap(ALLEGRO_BITMAP* bitmap, float cx, float cy, float dx, float dy,
                                    float xscale, float yscale, float angle, int flags) {
    int w = al_get_bitmap_width(bitmap);
    int h = al_get_bitmap_height(bitmap);
    
    SDL_Rect dst_rect = {
        (int)(dx - cx * xscale),
        (int)(dy - cy * yscale),
        (int)(w * xscale),
        (int)(h * yscale)
    };
    
    SDL_Point center = {(int)cx, (int)cy};
    SDL_RenderCopyEx(shim.state.renderer, bitmap, nullptr, &dst_rect,
                     angle * 180.0f / ALLEGRO_PI, &center, SDL_FLIP_NONE);
}
```

---

### Tinted Bitmap Drawing

**Functions:**
```c
void al_draw_tinted_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float dx, float dy, int flags);
void al_draw_tinted_bitmap_region(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float sx, float sy,
                                  float sw, float sh, float dx, float dy, int flags);
void al_draw_tinted_scaled_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float sx, float sy,
                                  float sw, float sh, float dx, float dy, float dw, float dh, int flags);
void al_draw_tinted_rotated_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float cx, float cy,
                                    float dx, float dy, float angle, int flags);
void al_draw_tinted_scaled_rotated_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float cx, float cy,
                                          float dx, float dy, float xscale, float yscale, float angle, int flags);
```

**SDL2 Implementation:**
```c
void al_draw_tinted_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float dx, float dy, int flags) {
    // SDL2 doesn't support per-texture tinting directly
    // Approach 1: Use SDL_SetTextureColorMod and SDL_SetTextureAlphaMod
    SDL_SetTextureColorMod(bitmap, 
        (Uint8)(tint.r * 255),
        (Uint8)(tint.g * 255),
        (Uint8)(tint.b * 255));
    SDL_SetTextureAlphaMod(bitmap, (Uint8)(tint.a * 255));
    
    al_draw_bitmap(bitmap, dx, dy, flags);
    
    // Reset modulation
    SDL_SetTextureColorMod(bitmap, 255, 255, 255);
    SDL_SetTextureAlphaMod(bitmap, 255);
}
```

**Implementation Details:**
- Use SDL_SetTextureColorMod and SDL_SetTextureAlphaMod for tinting
- Reset after drawing to avoid affecting subsequent draws
- For complex tinted operations, may need shader support

---

## Font Rendering

### al_load_font

**Function Signature:**
```c
ALLEGRO_FONT* al_load_font(const char* filename, int size, int flags);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_load_font(const char* filename, int size, int flags) {
    return TTF_OpenFont(filename, size);
}
```

**Implementation Details:**
- Uses SDL2_ttf for TrueType font loading
- flags parameter can include ALLEGRO_TTF_NO_KERNING, ALLEGRO_TTF_MONOCHROME, ALLEGRO_TTF_NO_AUTOHINT

---

### al_load_bitmap_font

**Function Signature:**
```c
ALLEGRO_FONT* al_load_bitmap_font(const char* filename);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_load_bitmap_font(const char* filename) {
    // Bitmap fonts not natively supported in SDL2_ttf
    // Would need custom implementation or fallback to TTF
    return nullptr;
}
```

---

### al_create_builtin_font

**Function Signature:**
```c
ALLEGRO_FONT* al_create_builtin_font(void);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_create_builtin_font(void) {
    // SDL2 doesn't have built-in font
    // Return nullptr or load a default system font
    return nullptr;
}
```

---

### al_destroy_font

**Function Signature:**
```c
void al_destroy_font(ALLEGRO_FONT* font);
```

**SDL2 Implementation:**
```c
void al_destroy_font(ALLEGRO_FONT* font) {
    if (font) {
        TTF_CloseFont(font);
    }
}
```

---

### al_draw_text

**Function Signature:**
```c
void al_draw_text(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags,
                  const char* text);
```

**SDL2 Implementation:**
```c
void al_draw_text(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags,
                  const char* text) {
    if (!font || !text) return;
    
    SDL_Color sdl_color = {
        (Uint8)(color.r * 255),
        (Uint8)(color.g * 255),
        (Uint8)(color.b * 255),
        (Uint8)(color.a * 255)
    };
    
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, sdl_color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(shim.state.renderer, surface);
    
    SDL_Rect dst_rect = {(int)x, (int)y, surface->w, surface->h};
    
    // Handle alignment
    if (flags & ALLEGRO_ALIGN_CENTER) {
        dst_rect.x -= surface->w / 2;
    } else if (flags & ALLEGRO_ALIGN_RIGHT) {
        dst_rect.x -= surface->w;
    }
    
    SDL_RenderCopy(shim.state.renderer, texture, nullptr, &dst_rect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
```

**Implementation Details:**
- Create temporary SDL_Surface from text using TTF_RenderUTF8_Blended
- Convert to texture and render
- Handle alignment flags (LEFT, CENTER, RIGHT)
- For better performance with many draws, consider caching textures

---

### al_draw_ustr

**Function Signature:**
```c
void al_draw_ustr(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags,
                  const ALLEGRO_USTR* ustr);
```

**SDL2 Implementation:**
```c
void al_draw_ustr(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags,
                  const ALLEGRO_USTR* ustr) {
    // Convert ALLEGRO_USTR to UTF-8 string
    // Then call al_draw_text
    const char* str = al_cstr(ustr);  // If such function exists
    if (str) {
        al_draw_text(font, color, x, y, flags, str);
    }
}
```

---

### al_draw_justified_text

**Function Signature:**
```c
void al_draw_justified_text(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x1, float x2,
                            float y, float diff, int flags, const char* text);
```

**SDL2 Implementation:**
```c
void al_draw_justified_text(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x1, float x2,
                            float y, float diff, int flags, const char* text) {
    // Calculate line width and draw each line with appropriate spacing
    // More complex implementation needed for proper justification
}
```

---

### al_get_text_width

**Function Signature:**
```c
int al_get_text_width(const ALLEGRO_FONT* f, const char* str);
```

**SDL2 Implementation:**
```c
int al_get_text_width(const ALLEGRO_FONT* f, const char* str) {
    if (!f || !str) return 0;
    
    int w, h;
    TTF_SizeUTF8(f, str, &w, &h);
    return w;
}
```

---

### al_get_ustr_width

**Function Signature:**
```c
int al_get_ustr_width(const ALLEGRO_FONT* f, const ALLEGRO_USTR* ustr);
```

**SDL2 Implementation:**
```c
int al_get_ustr_width(const ALLEGRO_FONT* f, const ALLEGRO_USTR* ustr) {
    // Convert to UTF-8 and call al_get_text_width
    return 0;  // Implementation depends on ALLEGRO_USTR handling
}
```

---

### al_get_font_line_height

**Function Signature:**
```c
int al_get_font_line_height(const ALLEGRO_FONT* f);
```

**SDL2 Implementation:**
```c
int al_get_font_line_height(const ALLEGRO_FONT* f) {
    if (!f) return 0;
    return TTF_FontHeight(f);
}
```

---

### al_get_font_ascent

**Function Signature:**
```c
int al_get_font_ascent(const ALLEGRO_FONT* f);
```

**SDL2 Implementation:**
```c
int al_get_font_ascent(const ALLEGRO_FONT* f) {
    if (!f) return 0;
    return TTF_FontAscent(f);
}
```

---

### al_get_font_descent

**Function Signature:**
```c
int al_get_font_descent(const ALLEGRO_FONT* f);
```

**SDL2 Implementation:**
```c
int al_get_font_descent(const ALLEGRO_FONT* f) {
    if (!f) return 0;
    return TTF_FontDescent(f);
}
```

---

### al_draw_glyph

**Function Signature:**
```c
void al_draw_glyph(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int codepoint);
```

**SDL2 Implementation:**
```c
void al_draw_glyph(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int codepoint) {
    char utf8[8];
    // Convert codepoint to UTF-8
    // Render and draw
}
```

---

### al_get_glyph_width

**Function Signature:**
```c
int al_get_glyph_width(const ALLEGRO_FONT* f, int codepoint);
```

**SDL2 Implementation:**
```c
int al_get_glyph_width(const ALLEGRO_FONT* f, int codepoint) {
    // Use TTF_GlyphMetrics if available
    return 0;
}
```

---

### al_draw_textf / al_draw_justified_textf

**Function Signatures:**
```c
void al_draw_textf(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags,
                   const char* format, ...);
void al_draw_justified_textf(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x1, float x2,
                             float y, float diff, int flags, const char* format, ...);
```

**SDL2 Implementation:**
```c
void al_draw_textf(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags,
                   const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    al_draw_text(font, color, x, y, flags, buffer);
}
```

---

### al_draw_multiline_text

**Function Signature:**
```c
void al_draw_multiline_text(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y,
                            float max_width, float line_height, int flags, const char* text);
```

**SDL2 Implementation:**
```c
void al_draw_multiline_text(const ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y,
                            float max_width, float line_height, int flags, const char* text) {
    // Parse text for newlines
    // For each line, calculate width and draw
    // Advance y by line_height
}
```

---

### al_init_font_addon

**Function Signature:**
```c
bool al_init_font_addon(void);
```

**SDL2 Implementation:**
```c
bool al_init_font_addon(void) {
    return TTF_Init() == 0;
}
```

---

### al_is_font_addon_initialized

**Function Signature:**
```c
bool al_is_font_addon_initialized(void);
```

**SDL2 Implementation:**
```c
bool al_is_font_addon_initialized(void) {
    // Track initialization state
    return shim.font_initialized;
}
```

---

## Image Loading / Saving

### al_load_bitmap

**Function Signature:**
```c
ALLEGRO_BITMAP* al_load_bitmap(const char* filename);
```

**SDL2 Implementation:**
```c
ALLEGRO_BITMAP* al_load_bitmap(const char* filename) {
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) return nullptr;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(shim.state.renderer, surface);
    SDL_FreeSurface(surface);
    
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    
    return texture;
}
```

**Implementation Details:**
- Uses SDL2_image (IMG_Load) for image loading
- Supports PNG, JPEG, BMP, GIF, and other formats via SDL2_image

---

### al_save_bitmap

**Function Signature:**
```c
bool al_save_bitmap(const char* filename, ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
bool al_save_bitmap(const char* filename, ALLEGRO_BITMAP* bitmap) {
    // Get texture info
    Uint32 format;
    int access, w, h;
    SDL_QueryTexture(bitmap, &format, &access, &w, &h);
    
    // Read pixels from texture
    SDL_Texture* old_target = shim.state.current_target;
    SDL_SetRenderTarget(shim.state.renderer, bitmap);
    
    SDL_Surface* surface = SDL_CreateRGBSurface(0, w, h, 32, 
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    
    SDL_RenderReadPixels(shim.state.renderer, nullptr, 
                         surface->format->format,
                         surface->pixels, surface->pitch);
    
    SDL_SetRenderTarget(shim.state.renderer, old_target);
    
    // Save based on extension
    bool success = false;
    const char* ext = strrchr(filename, '.');
    if (ext) {
        if (strcasecmp(ext, ".png") == 0) {
            success = IMG_SavePNG(surface, filename) == 0;
        } else if (strcasecmp(ext, ".bmp") == 0) {
            success = SDL_SaveBMP(surface, filename) == 0;
        } else {
            // Try to save as PNG by default
            success = IMG_SavePNG(surface, filename) == 0;
        }
    }
    
    SDL_FreeSurface(surface);
    return success;
}
```

**Implementation Details:**
- Uses SDL_RenderReadPixels to get texture data
- Uses SDL2_image (IMG_SavePNG) or SDL_SaveBMP for saving
- Extension determines output format

---

### al_init_image_addon

**Function Signature:**
```c
bool al_init_image_addon(void);
```

**SDL2 Implementation:**
```c
bool al_init_image_addon(void) {
    // SDL2_image auto-initializes with IMG_Init
    return true;
}
```

---

## TTF Addon

### al_load_ttf_font

**Function Signature:**
```c
ALLEGRO_FONT* al_load_ttf_font(const char* filename, int size, int flags);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_load_ttf_font(const char* filename, int size, int flags) {
    TTF_Font* font = TTF_OpenFont(filename, size);
    if (!font) return nullptr;
    
    if (flags & ALLEGRO_TTF_NO_KERNING) {
        TTF_SetFontKerning(font, 0);
    }
    
    return font;
}
```

---

### al_load_ttf_font_stretch

**Function Signature:**
```c
ALLEGRO_FONT* al_load_ttf_font_stretch(const char* filename, int w, int h, int flags);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_load_ttf_font_stretch(const char* filename, int w, int h, int flags) {
    // SDL2_ttf doesn't support separate width/height
    // Use average or just height
    return TTF_OpenFont(filename, h);
}
```

---

### al_init_ttf_addon

**Function Signature:**
```c
bool al_init_ttf_addon(void);
```

**SDL2 Implementation:**
```c
bool al_init_ttf_addon(void) {
    return TTF_Init() == 0;
}
```

---

### al_is_ttf_addon_initialized / al_shutdown_ttf_addon

**Function Signatures:**
```c
bool al_is_ttf_addon_initialized(void);
void al_shutdown_ttf_addon(void);
```

**SDL2 Implementation:**
```c
bool al_is_ttf_addon_initialized(void) {
    return shim.ttf_initialized;
}

void al_shutdown_ttf_addon(void) {
    TTF_Quit();
    shim.ttf_initialized = false;
}
```

---

## Primitives Addon

### al_init_primitives_addon

**Function Signature:**
```c
bool al_init_primitives_addon(void);
```

**SDL2 Implementation:**
```c
bool al_init_primitives_addon(void) {
    // All primitives are implemented in graphics.cpp
    return true;
}
```

---

## Additional Bitmap Functions

### al_get_bitmap_format

**Function Signature:**
```c
int al_get_bitmap_format(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
int al_get_bitmap_format(ALLEGRO_BITMAP* bitmap) {
    Uint32 sdl_format;
    int access, w, h;
    SDL_QueryTexture(bitmap, &sdl_format, &access, &w, &h);
    
    // Convert SDL format to Allegro format
    switch (sdl_format) {
        case SDL_PIXELFORMAT_RGBA8888: return ALLEGRO_PIXEL_FORMAT_RGBA_8888;
        case SDL_PIXELFORMAT_RGB888: return ALLEGRO_PIXEL_FORMAT_RGB_888;
        // Add other mappings
        default: return ALLEGRO_PIXEL_FORMAT_ANY;
    }
}
```

---

### al_get_bitmap_flags

**Function Signature:**
```c
int al_get_bitmap_flags(ALLEGRO_BITMAP* bitmap);
```

**SDL2 Implementation:**
```c
int al_get_bitmap_flags(ALLEGRO_BITMAP* bitmap) {
    Uint32 sdl_format;
    int access, w, h;
    SDL_QueryTexture(bitmap, &sdl_format, &access, &w, &h);
    
    int flags = 0;
    if (access == SDL_TEXTUREACCESS_TARGET) {
        flags |= ALLEGRO_VIDEO_BITMAP;
    }
    return flags;
}
```

---

### al_convert_memory_bitmaps

**Function Signature:**
```c
void al_convert_memory_bitmaps(void);
```

**SDL2 Implementation:**
```c
void al_convert_memory_bitmaps(void) {
    // In SDL2, all textures are video bitmaps
    // No-op implementation
}
```

---

### al_reparent_bitmap

**Function Signature:**
```c
void al_reparent_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_BITMAP* parent, int x, int y, int w, int h);
```

**SDL2 Implementation:**
```c
void al_reparent_bitmap(ALLEGRO_BITMAP* bitmap, ALLEGRO_BITMAP* parent, int x, int y, int w, int h) {
    // Update bitmap wrapper to point to new parent
    // Adjust offsets
}
```

---

## Implementation Priority

1. **Phase 1 - Core Bitmap Operations (Critical)**
   - al_create_bitmap, al_destroy_bitmap
   - al_get_bitmap_width, al_get_bitmap_height
   - al_set_target_bitmap, al_get_target_bitmap

2. **Phase 2 - Basic Drawing**
   - al_clear_to_color
   - al_draw_pixel
   - al_draw_filled_rectangle
   - al_draw_line
   - al_draw_circle

3. **Phase 3 - Bitmap Blitting**
   - al_draw_bitmap
   - al_draw_scaled_bitmap
   - al_draw_bitmap_region

4. **Phase 4 - Color Functions**
   - al_map_rgb, al_map_rgba
   - al_premul_rgba
   - Color unmapping functions

5. **Phase 5 - Font Rendering**
   - al_load_font, al_destroy_font
   - al_draw_text
   - al_get_text_width

6. **Phase 6 - Image I/O**
   - al_load_bitmap
   - al_save_bitmap

7. **Phase 7 - Advanced Features**
   - Rotation and transformation
   - Tinted drawing
   - Sub-bitmaps
   - Additional primitives

---

## Dependencies

The graphics module requires the following SDL2 libraries:
- **SDL2** - Core rendering
- **SDL2_image** - Image loading/saving (PNG, JPEG support)
- **SDL2_ttf** - TrueType font support

Optional:
- **SDL2_gfx** - Advanced primitives (anti-aliased lines, ellipses, etc.)

---

## Notes

1. **Performance Considerations**: Creating SDL_Surface/Texture for each text draw is slow. Consider caching frequently drawn text.

2. **Alpha Blending**: SDL2 requires SDL_SetRenderDrawBlendMode for alpha blending. Set blend mode before drawing with alpha.

3. **Coordinate Systems**: Both Allegro and SDL2 use top-left origin. No conversion needed.

4. **Rotation**: SDL2 uses degrees, Allegro uses radians. Conversion required.

5. **Thread Safety**: SDL_Renderer is not thread-safe. All drawing must occur on the main thread.

6. **Memory Bitmaps**: SDL2 doesn't distinguish between memory and video bitmaps. All textures can be render targets.

---

## New Details from SDL Source Analysis

### SDL2 Graphics APIs

The repository contains **SDL2** with graphics capabilities:

1. **SDL_Render (2D)**: Located in `src/render/`
   - Multiple backends: Direct3D, Direct3D11, Metal, OpenGL, OpenGLES2, software renderer
   - Main file: `SDL_render.c`
   - Features: Points, lines, rectangles, textures, primitives

2. **SDL_Surface**: Located in `src/video/SDL_surface.c`
   - Pixel surface management
   - Blitting operations
   - RLE acceleration

3. **Video Backends**: `src/video/` contains platform-specific implementations
   - X11, Wayland (Linux)
   - Cocoa (macOS)
   - Windows (Direct3D, WinRT)
   - Android, iOS

### SDL2 Pixel Formats

SDL2 uses `SDL_pixels.h` for pixel format definitions:
- `SDL_PIXELFORMAT_RGBA8888` - Common format
- `SDL_PIXELFORMAT_RGB888` - 24-bit RGB
- Use `SDL_ConvertPixels()` for format conversion

### SDL2 Rendering Updates

1. **Functions**:
   - `SDL_RenderCopy()` - Copy texture to renderer
   - `SDL_RenderCopyEx()` - Copy with rotation/flip
   - `SDL_RenderDrawPoint()` / `SDL_RenderDrawLine()` / `SDL_RenderDrawRect()` / `SDL_RenderFillRect()`

2. **Texture Updates**:
   - `SDL_CreateTexture()` - Create GPU texture
   - `SDL_LockTexture()` - CPU access to texture pixels
   - `SDL_UpdateTexture()` - Upload pixels to GPU

### Implementation Notes for SDL2

1. Use `SDL_CreateTexture()` with `SDL_TEXTUREACCESS_TARGET` for render-to-texture
2. Use `SDL_SetRenderTarget()` for off-screen rendering
3. For fonts, use SDL2_ttf which is separate
4. For images, use SDL2_image which is separate

### Dependencies Update

| Library | Notes |
|---------|-------|
| SDL2 | Use native SDL2 rendering |
| SDL2_image | For extended image format support |
| SDL2_ttf | For font rendering |
