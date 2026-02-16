# Addons Module Implementation Plan

This document details the SDL2 implementation approach for the Allegro 5 Addons Module, covering image, font, primitives, and TTF addons.

## Overview

The Addons Module provides initialization functions and support for various Allegro 5 addons. In SDL2, these map to:

| Allegro Addon | SDL2 Library | Purpose |
|--------------|--------------|---------|
| Image | SDL2_image | Image loading/saving (PNG, JPEG, BMP, etc.) |
| Font | SDL2_ttf | Font initialization and rendering |
| Primitives | SDL2 + custom code | Drawing primitives (lines, circles, polygons) |
| TTF | SDL2_ttf | TrueType font loading |

## Global State Requirements

```cpp
struct AddonsState {
    bool image_initialized = false;
    bool font_initialized = false;
    bool primitives_initialized = false;
    bool ttf_initialized = false;
    
    int image_init_flags = 0;
    int ttf_init_flags = 0;
};
```

---

## Image Addon

### Header Reference
`allegro5/addons/image/allegro5/allegro_image.h`

### al_init_image_addon

**Function Signature:**
```c
bool al_init_image_addon(void);
```

**SDL2 Implementation:**
```c
bool al_init_image_addon(void) {
    int flags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TGF | IMG_INIT_BMP | IMG_INIT_GIF;
    int initialized = IMG_Init(flags);
    
    shim.addons.image_initialized = (initialized & flags) != 0;
    shim.addons.image_init_flags = initialized;
    
    return shim.addons.image_initialized;
}
```

**Implementation Details:**
- Initialize SDL2_image with support for PNG, JPEG, TGF, BMP, and GIF formats
- Track which formats were successfully initialized
- Return true if at least one image format is available

---

### al_is_image_addon_initialized

**Function Signature:**
```c
bool al_is_image_addon_initialized(void);
```

**SDL2 Implementation:**
```c
bool al_is_image_addon_initialized(void) {
    return shim.addons.image_initialized;
}
```

**Implementation Details:**
- Simply return the stored initialization state

---

### al_shutdown_image_addon

**Function Signature:**
```c
void al_shutdown_image_addon(void);
```

**SDL2 Implementation:**
```c
void al_shutdown_image_addon(void) {
    IMG_Quit();
    shim.addons.image_initialized = false;
    shim.addons.image_init_flags = 0;
}
```

**Implementation Details:**
- Quit SDL2_image subsystem
- Reset initialization state

---

### al_get_allegro_image_version

**Function Signature:**
```c
uint32_t al_get_allegro_image_version(void);
```

**SDL2 Implementation:**
```c
uint32_t al_get_allegro_image_version(void) {
    return ALLEGRO_IMAGE_VERSION;
}
```

**Implementation Details:**
- Return the Allegro image addon version constant
- Used for version checking by Allegro programs

---

## Font Addon

### Header Reference
`allegro5/addons/font/allegro5/allegro_font.h`

**Note:** Many font functions are already covered in `graphics_impl.md`. This section provides addon-specific initialization functions and additional font-related functions.

### al_init_font_addon

**Function Signature:**
```c
bool al_init_font_addon(void);
```

**SDL2 Implementation:**
```c
bool al_init_font_addon(void) {
    int result = TTF_Init();
    shim.addons.font_initialized = (result == 0);
    return shim.addons.font_initialized;
}
```

**Implementation Details:**
- Initialize SDL2_ttf subsystem
- This is required before any font operations

---

### al_is_font_addon_initialized

**Function Signature:**
```c
bool al_is_font_addon_initialized(void);
```

**SDL2 Implementation:**
```c
bool al_is_font_addon_initialized(void) {
    return shim.addons.font_initialized;
}
```

**Implementation Details:**
- Return the font subsystem initialization state

---

### al_shutdown_font_addon

**Function Signature:**
```c
void al_shutdown_font_addon(void);
```

**SDL2 Implementation:**
```c
void al_shutdown_font_addon(void) {
    TTF_Quit();
    shim.addons.font_initialized = false;
}
```

**Implementation Details:**
- Quit SDL2_ttf subsystem
- All fonts should be closed before calling this

---

### al_get_allegro_font_version

**Function Signature:**
```c
uint32_t al_get_allegro_font_version(void);
```

**SDL2 Implementation:**
```c
uint32_t al_get_allegro_font_version(void) {
    return ALLEGRO_FONT_VERSION;
}
```

**Implementation Details:**
- Return the Allegro font addon version constant

---

### al_register_font_loader

**Function Signature:**
```c
bool al_register_font_loader(const char *ext, ALLEGRO_FONT *(*load)(const char *filename, int size, int flags));
```

**SDL2 Implementation:**
```c
typedef struct FontLoader {
    const char* extension;
    ALLEGRO_FONT* (*load)(const char*, int, int);
    struct FontLoader* next;
} FontLoader;

static FontLoader* font_loaders = nullptr;

bool al_register_font_loader(const char *ext, ALLEGRO_FONT *(*load)(const char *filename, int size, int flags)) {
    FontLoader* loader = (FontLoader*)malloc(sizeof(FontLoader));
    if (!loader) return false;
    
    loader->extension = strdup(ext);
    loader->load = load;
    loader->next = font_loaders;
    font_loaders = loader;
    
    return true;
}
```

**Implementation Details:**
- Register custom font loaders for additional file formats
- Store in a linked list for lookup during font loading

---

### al_load_bitmap_font

**Function Signature:**
```c
ALLEGRO_FONT* al_load_bitmap_font(const char *filename);
ALLEGRO_FONT* al_load_bitmap_font_flags(const char *filename, int flags);
```

**SDL2 Implementation:**
```c
// SDL2_ttf does not natively support bitmap fonts
// This would require a custom implementation

ALLEGRO_FONT* al_load_bitmap_font(const char *filename) {
    // Try registered loaders first
    for (FontLoader* loader = font_loaders; loader; loader = loader->next) {
        const char* ext = strrchr(filename, '.');
        if (ext && strcasecmp(ext + 1, loader->extension) == 0) {
            return loader->load(filename, 0, 0);
        }
    }
    
    // Fall back to trying as TTF
    return TTF_OpenFont(filename, 12);
}
```

**Implementation Details:**
- SDL2_ttf does not natively support bitmap fonts
- Check registered custom loaders first
- Fall back to TTF loading as a compatibility measure

---

### al_grab_font_from_bitmap

**Function Signature:**
```c
ALLEGRO_FONT* al_grab_font_from_bitmap(ALLEGRO_BITMAP *bmp, int n, const int ranges[]);
```

**SDL2 Implementation:**
```c
// Creating fonts from bitmap requires custom implementation
// Not directly supported by SDL2_ttf

ALLEGRO_FONT* al_grab_font_from_bitmap(ALLEGRO_BITMAP *bmp, int n, const int ranges[]) {
    // Would require parsing a sprite sheet bitmap font
    // Complex custom implementation needed
    return nullptr;
}
```

**Implementation Details:**
- Not supported by SDL2_ttf
- Would require custom bitmap font parsing implementation

---

### al_get_font_ranges

**Function Signature:**
```c
int al_get_font_ranges(ALLEGRO_FONT *font, int ranges_count, int *ranges);
```

**SDL2 Implementation:**
```c
int al_get_font_ranges(ALLEGRO_FONT *font, int ranges_count, int *ranges) {
    if (!font) return 0;
    
    // SDL2_ttf doesn't expose font ranges directly
    // Return a default ASCII range
    if (ranges && ranges_count >= 1) {
        ranges[0] = 32;  // Space
        ranges[1] = 126;  // Tilde
        return 1;
    }
    
    return 0;
}
```

**Implementation Details:**
- SDL2_ttf doesn't expose font range information
- Return basic ASCII range as fallback

---

### al_get_text_dimensions

**Function Signature:**
```c
void al_get_text_dimensions(const ALLEGRO_FONT *f, char const *text,
    int *bbx, int *bby, int *bbw, int *bbh);
```

**SDL2 Implementation:**
```c
void al_get_text_dimensions(const ALLEGRO_FONT *f, char const *text,
    int *bbx, int *bby, int *bbw, int *bbh) {
    if (!f || !text) {
        if (bbx) *bbx = 0;
        if (bby) *bby = 0;
        if (bbw) *bbw = 0;
        if (bbh) *bbh = 0;
        return;
    }
    
    int w, h;
    TTF_SizeUTF8(f, text, &w, &h);
    
    if (bbx) *bbx = 0;
    if (bby) *bby = 0;  // SDL2_ttf doesn't track baseline separately
    if (bbw) *bbw = w;
    if (bbh) *bbh = h;
}
```

**Implementation Details:**
- Use TTF_SizeUTF8 for width/height
- SDL2_ttf doesn't provide bounding box offset information

---

### al_get_ustr_dimensions

**Function Signature:**
```c
void al_get_ustr_dimensions(const ALLEGRO_FONT *f, ALLEGRO_USTR const *text,
    int *bbx, int *bby, int *bbw, int *bbh);
```

**SDL2 Implementation:**
```c
void al_get_ustr_dimensions(const ALLEGRO_FONT *f, ALLEGRO_USTR const *text,
    int *bbx, int *bby, int *bbw, int *bbh) {
    // Convert ALLEGRO_USTR to UTF-8 string
    // Then call al_get_text_dimensions
    if (text) {
        const char* str = al_cstr(text);  // If conversion function exists
        al_get_text_dimensions(f, str, bbx, bby, bbw, bbh);
    } else {
        al_get_text_dimensions(f, nullptr, bbx, bby, bbw, bbh);
    }
}
```

**Implementation Details:**
- Convert ALLEGRO_USTR to UTF-8 for SDL2_ttf

---

### al_get_glyph_dimensions

**Function Signature:**
```c
bool al_get_glyph_dimensions(const ALLEGRO_FONT *f, int codepoint, 
    int *bbx, int *bby, int *bbw, int *bbh);
```

**SDL2 Implementation:**
```c
bool al_get_glyph_dimensions(const ALLEGRO_FONT *f, int codepoint, 
    int *bbx, int *bby, int *bbw, int *bbh) {
    if (!f) return false;
    
    TTF_GlyphMetrics(f, codepoint, nullptr, nullptr, bby, bbh, bbx, bbw);
    
    return true;
}
```

**Implementation Details:**
- Use TTF_GlyphMetrics from SDL2_ttf
- Note: TTF_GlyphMetrics parameters are (minx, maxx, miny, maxy, advance)

---

### al_get_glyph_advance

**Function Signature:**
```c
int al_get_glyph_advance(const ALLEGRO_FONT *f, int codepoint1, int codepoint2);
```

**SDL2 Implementation:**
```c
int al_get_glyph_advance(const ALLEGRO_FONT *f, int codepoint1, int codepoint2) {
    if (!f) return 0;
    
    int minx1, maxx1, miny1, maxy1, adv1;
    int minx2, maxx2, miny2, maxy2, adv2;
    
    TTF_GlyphMetrics(f, codepoint1, &minx1, &maxx1, &miny1, &maxy1, &adv1);
    TTF_GlyphMetrics(f, codepoint2, &minx2, &maxx2, &miny2, &maxy2, &adv2);
    
    // Advance = first glyph's advance + kerning
    int kerning = TTF_GetFontKerning(f);
    return adv1 + kerning;
}
```

**Implementation Details:**
- Get advance from first glyph and add kerning

---

### al_get_glyph (Unstable API)

**Function Signature:**
```c
bool al_get_glyph(const ALLEGRO_FONT *f, int prev_codepoint, int codepoint, ALLEGRO_GLYPH *glyph);
```

**SDL2 Implementation:**
```c
bool al_get_glyph(const ALLEGRO_FONT *f, int prev_codepoint, int codepoint, ALLEGRO_GLYPH *glyph) {
    if (!f || !glyph) return false;
    
    int minx, maxx, miny, maxy, advance;
    TTF_GlyphMetrics(f, codepoint, &minx, &maxx, &miny, &maxy, &advance);
    
    glyph->x = minx;
    glyph->y = miny;
    glyph->w = maxx - minx;
    glyph->h = maxy - miny;
    glyph->offset_x = minx;
    glyph->offset_y = miny;
    glyph->advance = advance;
    
    // Glyph bitmap would need separate rendering
    glyph->bitmap = nullptr;
    
    return true;
}
```

**Implementation Details:**
- Fill in glyph metrics from SDL2_ttf
- Bitmap would need separate rendering pass

---

### al_set_fallback_font / al_get_fallback_font

**Function Signatures:**
```c
void al_set_fallback_font(ALLEGRO_FONT *font, ALLEGRO_FONT *fallback);
ALLEGRO_FONT* al_get_fallback_font(ALLEGRO_FONT *font);
```

**SDL2 Implementation:**
```c
// Need to track fallback fonts per font
typedef struct FontWithFallback {
    TTF_Font* font;
    TTF_Font* fallback;
} FontWithFallback;

static std::unordered_map<TTF_Font*, TTF_Font*> fallback_fonts;

void al_set_fallback_font(ALLEGRO_FONT *font, ALLEGRO_FONT *fallback) {
    fallback_fonts[(TTF_Font*)font] = (TTF_Font*)fallback;
}

ALLEGRO_FONT* al_get_fallback_font(ALLEGRO_FONT *font) {
    auto it = fallback_fonts.find((TTF_Font*)font);
    if (it != fallback_fonts.end()) {
        return (ALLEGRO_FONT*)it->second;
    }
    return nullptr;
}
```

**Implementation Details:**
- Maintain a map of fonts to their fallback fonts
- During text rendering, check fallback for missing glyphs

---

### al_draw_multiline_text / al_do_multiline_text

**Function Signatures:**
```c
void al_draw_multiline_text(const ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y,
    float max_width, float line_height, int flags, const char *text);
void al_do_multiline_text(const ALLEGRO_FONT *font, float max_width, const char *text,
    bool (*cb)(int line_num, const char *line, int size, void *extra), void *extra);
```

**SDL2 Implementation:**
```c
void al_draw_multiline_text(const ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y,
    float max_width, float line_height, int flags, const char *text) {
    if (!font || !text) return;
    
    const char* line_start = text;
    float current_y = y;
    
    while (*line_start) {
        // Find line end
        const char* line_end = strchr(line_start, '\n');
        size_t line_len;
        
        if (line_end) {
            line_len = line_end - line_start;
        } else {
            line_len = strlen(line_start);
        }
        
        if (line_len > 0) {
            char* line = (char*)malloc(line_len + 1);
            strncpy(line, line_start, line_len);
            line[line_len] = '\0';
            
            // Check if line exceeds max_width and wrap if needed
            int width = al_get_text_width(font, line);
            if (width > max_width && line_len > 1) {
                // Simple word wrap implementation
                // (Would need more sophisticated wrapping)
            }
            
            al_draw_text(font, color, x, current_y, flags, line);
            free(line);
        }
        
        if (!line_end) break;
        line_start = line_end + 1;
        current_y += line_height;
    }
}

void al_do_multiline_text(const ALLEGRO_FONT *font, float max_width, const char *text,
    bool (*cb)(int line_num, const char *line, int size, void *extra), void *extra) {
    if (!font || !text || !cb) return;
    
    const char* line_start = text;
    int line_num = 0;
    
    while (*line_start) {
        const char* line_end = strchr(line_start, '\n');
        size_t line_len;
        
        if (line_end) {
            line_len = line_end - line_start;
        } else {
            line_len = strlen(line_start);
        }
        
        if (line_len > 0) {
            char* line = (char*)malloc(line_len + 1);
            strncpy(line, line_start, line_len);
            line[line_len] = '\0';
            
            int width = al_get_text_width(font, line);
            
            if (!cb(line_num, line, width, extra)) {
                free(line);
                break;
            }
            
            free(line);
        }
        
        if (!line_end) break;
        line_start = line_end + 1;
        line_num++;
    }
}
```

**Implementation Details:**
- Parse text for newline characters
- Draw each line at the appropriate Y position
- For do_multiline, call callback for each line

---

## Primitives Addon

### Header Reference
`allegro5/addons/primitives/allegro5/allegro_primitives.h`

### al_init_primitives_addon

**Function Signature:**
```c
bool al_init_primitives_addon(void);
```

**SDL2 Implementation:**
```c
bool al_init_primitives_addon(void) {
    shim.addons.primitives_initialized = true;
    return true;
}
```

**Implementation Details:**
- All primitives are implemented in graphics.cpp
- This function simply marks the addon as initialized

---

### al_is_primitives_addon_initialized

**Function Signature:**
```c
bool al_is_primitives_addon_initialized(void);
```

**SDL2 Implementation:**
```c
bool al_is_primitives_addon_initialized(void) {
    return shim.addons.primitives_initialized;
}
```

---

### al_shutdown_primitives_addon

**Function Signature:**
```c
void al_shutdown_primitives_addon(void);
```

**SDL2 Implementation:**
```c
void al_shutdown_primitives_addon(void) {
    shim.addons.primitives_initialized = false;
}
```

---

### al_get_allegro_primitives_version

**Function Signature:**
```c
uint32_t al_get_allegro_primitives_version(void);
```

**SDL2 Implementation:**
```c
uint32_t al_get_allegro_primitives_version(void) {
    return ALLEGRO_PRIMITIVES_VERSION;
}
```

---

### al_draw_prim / al_draw_indexed_prim

**Function Signatures:**
```c
int al_draw_prim(const void* vtxs, const ALLEGRO_VERTEX_DECL* decl, ALLEGRO_BITMAP* texture, 
    int start, int end, int type);
int al_draw_indexed_prim(const void* vtxs, const ALLEGRO_VERTEX_DECL* decl, ALLEGRO_BITMAP* texture,
    const int* indices, int num_vtx, int type);
```

**SDL2 Implementation:**
```c
// Low-level primitive drawing requires custom vertex handling
// SDL2 doesn't support custom vertex declarations

int al_draw_prim(const void* vtxs, const ALLEGRO_VERTEX_DECL* decl, ALLEGRO_BITMAP* texture, 
    int start, int end, int type) {
    // Would need to interpret ALLEGRO_VERTEX_DECL
    // Complex implementation for custom vertex formats
    return 0;
}

int al_draw_indexed_prim(const void* vtxs, const ALLEGRO_VERTEX_DECL* decl, ALLEGRO_BITMAP* texture,
    const int* indices, int num_vtx, int type) {
    return 0;
}
```

**Implementation Details:**
- Complex low-level functions requiring custom vertex buffer handling
- SDL2 uses different approach for custom primitives

---

### al_create_vertex_decl / al_destroy_vertex_decl

**Function Signatures:**
```c
ALLEGRO_VERTEX_DECL* al_create_vertex_decl(const ALLEGRO_VERTEX_ELEMENT* elements, int stride);
void al_destroy_vertex_decl(ALLEGRO_VERTEX_DECL* decl);
```

**SDL2 Implementation:**
```c
// SDL2 doesn't have equivalent
// Can store for compatibility but won't use

ALLEGRO_VERTEX_DECL* al_create_vertex_decl(const ALLEGRO_VERTEX_ELEMENT* elements, int stride) {
    return nullptr;  // Not supported
}

void al_destroy_vertex_decl(ALLEGRO_VERTEX_DECL* decl) {
    // No-op
}
```

---

### Vertex and Index Buffers

**Functions:**
```c
ALLEGRO_VERTEX_BUFFER* al_create_vertex_buffer(ALLEGRO_VERTEX_DECL* decl, const void* initial_data, 
    int num_vertices, int flags);
void al_destroy_vertex_buffer(ALLEGRO_VERTEX_BUFFER* buffer);
void* al_lock_vertex_buffer(ALLEGRO_VERTEX_BUFFER* buffer, int offset, int length, int flags);
void al_unlock_vertex_buffer(ALLEGRO_VERTEX_BUFFER* buffer);
int al_get_vertex_buffer_size(ALLEGRO_VERTEX_BUFFER* buffer);

ALLEGRO_INDEX_BUFFER* al_create_index_buffer(int index_size, const void* initial_data, 
    int num_indices, int flags);
void al_destroy_index_buffer(ALLEGRO_INDEX_BUFFER* buffer);
void* al_lock_index_buffer(ALLEGRO_INDEX_BUFFER* buffer, int offset, int length, int flags);
void al_unlock_index_buffer(ALLEGRO_INDEX_BUFFER* buffer);
int al_get_index_buffer_size(ALLEGRO_INDEX_BUFFER* buffer);
```

**SDL2 Implementation:**
```c
// These would require custom implementations using SDL surfaces or textures
// Not directly supported in SDL2

// Return nullptr for creation functions
// No-op for destroy functions
```

---

### al_triangulate_polygon

**Function Signature:**
```c
bool al_triangulate_polygon(const float* vertices, size_t vertex_stride, const int* vertex_counts,
    void (*emit_triangle)(int, int, int, void*), void* userdata);
```

**SDL2 Implementation:**
```c
bool al_triangulate_polygon(const float* vertices, size_t vertex_stride, const int* vertex_counts,
    void (*emit_triangle)(int, int, int, void*), void* userdata) {
    // Would require polygon triangulation algorithm (ear clipping, etc.)
    return false;
}
```

**Implementation Details:**
- Would need to implement polygon triangulation algorithm

---

### Soft Triangle/Line Drawing

**Functions:**
```c
void al_draw_soft_triangle(ALLEGRO_VERTEX* v1, ALLEGRO_VERTEX* v2, ALLEGRO_VERTEX* v3, uintptr_t state,
    void (*init)(uintptr_t, ALLEGRO_VERTEX*, ALLEGRO_VERTEX*, ALLEGRO_VERTEX*),
    void (*first)(uintptr_t, int, int, int, int),
    void (*step)(uintptr_t, int),
    void (*draw)(uintptr_t, int, int, int));
void al_draw_soft_line(ALLEGRO_VERTEX* v1, ALLEGRO_VERTEX* v2, uintptr_t state,
    void (*first)(uintptr_t, int, int, ALLEGRO_VERTEX*, ALLEGRO_VERTEX*),
    void (*step)(uintptr_t, int),
    void (*draw)(uintptr_t, int, int));
```

**SDL2 Implementation:**
```c
// Software rasterization not supported in SDL2
// No-op implementations
```

---

### High-Level Primitives

The following primitives are already covered in `graphics_impl.md`:

- `al_draw_line` - Line drawing with thickness
- `al_draw_triangle` - Triangle outline
- `al_draw_rectangle` - Rectangle outline
- `al_draw_rounded_rectangle` - Rounded rectangle outline
- `al_draw_circle` - Circle outline
- `al_draw_ellipse` - Ellipse outline
- `al_draw_arc` - Arc drawing
- `al_draw_elliptical_arc` - Elliptical arc
- `al_draw_pieslice` - Pie slice (outline)
- `al_draw_spline` - Bezier spline
- `al_draw_ribbon` - Ribbon (thick line strip)
- `al_draw_polyline` - Polyline
- `al_draw_polygon` - Polygon outline
- `al_draw_filled_triangle` - Filled triangle
- `al_draw_filled_rectangle` - Filled rectangle
- `al_draw_filled_ellipse` - Filled ellipse
- `al_draw_filled_circle` - Filled circle
- `al_draw_filled_pieslice` - Filled pie slice
- `al_draw_filled_rounded_rectangle` - Filled rounded rectangle
- `al_draw_filled_polygon` - Filled polygon
- `al_draw_filled_polygon_with_holes` - Filled polygon with holes

---

### al_calculate_arc

**Function Signature:**
```c
void al_calculate_arc(float* dest, int stride, float cx, float cy, float rx, float ry,
    float start_theta, float delta_theta, float thickness, int num_points);
```

**SDL2 Implementation:**
```c
void al_calculate_arc(float* dest, int stride, float cx, float cy, float rx, float ry,
    float start_theta, float delta_theta, float thickness, int num_points) {
    float angle = start_theta;
    float step = delta_theta / num_points;
    
    for (int i = 0; i < num_points; i++) {
        float* point = (float*)((char*)dest + i * stride);
        
        float inner_r = rx - thickness / 2;
        float outer_r = rx + thickness / 2;
        
        // Inner arc
        point[0] = cx + inner_r * cosf(angle);
        point[1] = cy + inner_r * sinf(angle);
        
        // Outer arc would need to be calculated separately
        // or this would output interleaved inner/outer points
        
        angle += step;
    }
}
```

---

### al_calculate_spline

**Function Signature:**
```c
void al_calculate_spline(float* dest, int stride, const float points[8], 
    float thickness, int num_segments);
```

**SDL2 Implementation:**
```c
void al_calculate_spline(float* dest, int stride, const float points[8], 
    float thickness, int num_segments) {
    // Bezier spline calculation
    // points[0-7] = p1x, p1y, c1x, c1y, c2x, c2y, p2x, p2y
    
    for (int i = 0; i <= num_segments; i++) {
        float t = (float)i / num_segments;
        float* point = (float*)((char*)dest + i * stride);
        
        // Cubic Bezier: B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
        float t2 = t * t;
        float t3 = t2 * t;
        float mt = 1 - t;
        float mt2 = mt * mt;
        float mt3 = mt2 * mt;
        
        point[0] = mt3 * points[0] + 3 * mt2 * t * points[2] + 3 * mt * t2 * points[4] + t3 * points[6];
        point[1] = mt3 * points[1] + 3 * mt2 * t * points[3] + 3 * mt * t2 * points[5] + t3 * points[7];
    }
}
```

---

### al_calculate_ribbon

**Function Signature:**
```c
void al_calculate_ribbon(float* dest, int dest_stride, const float *points, 
    int points_stride, float thickness, int num_segments);
```

**SDL2 Implementation:**
```c
void al_calculate_ribbon(float* dest, int dest_stride, const float *points, 
    int points_stride, float thickness, int num_segments) {
    // Generate ribbon points from line strip
    for (int i = 0; i <= num_segments; i++) {
        float t = (float)i / num_segments;
        
        // Get current point
        const float* curr = (const float*)((const char*)points + (i * points_stride));
        float x = curr[0];
        float y = curr[1];
        
        // Calculate normal for thickness
        float nx = 0, ny = 1;
        if (i < num_segments) {
            const float* next = (const float*)((const char*)points + ((i + 1) * points_stride));
            float dx = next[0] - x;
            float dy = next[1] - y;
            float len = sqrtf(dx*dx + dy*dy);
            if (len > 0) {
                nx = -dy / len;
                ny = dx / len;
            }
        }
        
        float* dest_left = (float*)((char*)dest + (i * 2) * dest_stride);
        float* dest_right = (float*)((char*)dest + (i * 2 + 1) * dest_stride);
        
        dest_left[0] = x + nx * thickness / 2;
        dest_left[1] = y + ny * thickness / 2;
        dest_right[0] = x - nx * thickness / 2;
        dest_right[1] = y - ny * thickness / 2;
    }
}
```

---

## TTF Addon

### Header Reference
`allegro5/addons/ttf/allegro5/allegro_ttf.h`

**Note:** TTF font loading is partially covered in `graphics_impl.md`. This section provides addon-specific initialization functions and file-based loading.

### TTF Flags

```c
#define ALLEGRO_TTF_NO_KERNING  1   // Disable kerning
#define ALLEGRO_TTF_MONOCHROME  2   // Monochrome rendering
#define ALLEGRO_TTF_NO_AUTOHINT 4   // Disable autohinting
```

**SDL2 Mapping:**
```c
// These flags map to SDL2_ttf constants:
// ALLEGRO_TTF_NO_KERNING -> TTF_DISABLE_KERNING
// ALLEGRO_TTF_MONOCHROME -> TTF_MONOCHROME (if available)
// ALLEGRO_TTF_NO_AUTOHINT -> TTF_NO_AUTOHINT (if available)
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
    // May share initialization with font addon
    int result = TTF_Init();
    shim.addons.ttf_initialized = (result == 0);
    shim.addons.font_initialized = shim.addons.ttf_initialized;
    return shim.addons.ttf_initialized;
}
```

**Implementation Details:**
- Initialize SDL2_ttf (may share with font addon)
- Mark both TTF and font as initialized

---

### al_is_ttf_addon_initialized

**Function Signature:**
```c
bool al_is_ttf_addon_initialized(void);
```

**SDL2 Implementation:**
```c
bool al_is_ttf_addon_initialized(void) {
    return shim.addons.ttf_initialized;
}
```

---

### al_shutdown_ttf_addon

**Function Signature:**
```c
void al_shutdown_ttf_addon(void);
```

**SDL2 Implementation:**
```c
void al_shutdown_ttf_addon(void) {
    TTF_Quit();
    shim.addons.ttf_initialized = false;
    shim.addons.font_initialized = false;
}
```

---

### al_get_allegro_ttf_version

**Function Signature:**
```c
uint32_t al_get_allegro_ttf_version(void);
```

**SDL2 Implementation:**
```c
uint32_t al_get_allegro_ttf_version(void) {
    return ALLEGRO_TTF_VERSION;
}
```

---

### al_load_ttf_font

**Function Signature:**
```c
ALLEGRO_FONT* al_load_ttf_font(const char *filename, int size, int flags);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_load_ttf_font(const char *filename, int size, int flags) {
    TTF_Font* font = TTF_OpenFont(filename, size);
    if (!font) return nullptr;
    
    // Apply flags
    if (flags & ALLEGRO_TTF_NO_KERNING) {
        TTF_SetFontKerning(font, 0);
    }
    
    // SDL2_ttf may not support all flags
    // MONOCHROME and NO_AUTOHINT depend on SDL2_ttf version
    
    return (ALLEGRO_FONT*)font;
}
```

**Implementation Details:**
- Open TTF file with SDL2_ttf
- Apply Allegro flags to SDL2_ttf settings

---

### al_load_ttf_font_f

**Function Signature:**
```c
ALLEGRO_FONT* al_load_ttf_font_f(ALLEGRO_FILE *file, const char *filename, int size, int flags);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_load_ttf_font_f(ALLEGRO_FILE *file, const char *filename, int size, int flags) {
    // SDL2_ttf doesn't support loading from ALLEGRO_FILE
    // Would need to read file into memory and use TTF_OpenFontRW
    
    if (!file) return nullptr;
    
    // Get file size and read into memory buffer
    // Then use TTF_OpenFontRW with SDL_RWFromMem
    
    // This is complex and may not be fully compatible
    return nullptr;
}
```

**Implementation Details:**
- SDL2_ttf doesn't support custom file objects
- Would require reading file into memory buffer first

---

### al_load_ttf_font_stretch

**Function Signature:**
```c
ALLEGRO_FONT* al_load_ttf_font_stretch(const char *filename, int w, int h, int flags);
ALLEGRO_FONT* al_load_ttf_font_stretch_f(ALLEGRO_FILE *file, const char *filename, int w, int h, int flags);
```

**SDL2 Implementation:**
```c
ALLEGRO_FONT* al_load_ttf_font_stretch(const char *filename, int w, int h, int flags) {
    // SDL2_ttf doesn't support separate width/height
    // Use height as size, ignore width
    TTF_Font* font = TTF_OpenFont(filename, h);
    if (!font) return nullptr;
    
    if (flags & ALLEGRO_TTF_NO_KERNING) {
        TTF_SetFontKerning(font, 0);
    }
    
    return (ALLEGRO_FONT*)font;
}
```

**Implementation Details:**
- SDL2_ttf doesn't support font stretching
- Use height as font size, ignore width parameter

---

## Implementation Priority

1. **Phase 1 - Core Initialization**
   - `al_init_image_addon`
   - `al_init_font_addon`
   - `al_init_primitives_addon`
   - `al_init_ttf_addon`

2. **Phase 2 - Basic Font Functions**
   - Font loading and destruction
   - Text drawing
   - Text dimension queries

3. **Phase 3 - Image Functions**
   - Image loading/saving via al_load_bitmap/al_save_bitmap

4. **Phase 4 - Additional Primitives**
   - Spline calculations
   - Polygon triangulation

5. **Phase 5 - Advanced Features**
   - Custom vertex buffers
   - Soft rasterization
   - TTF file-based loading

---

## Dependencies

| SDL2 Library | Purpose |
|--------------|---------|
| SDL2 | Core functionality |
| SDL2_image | Image loading (PNG, JPEG, etc.) |
| SDL2_ttf | Font rendering |

---

## Notes

1. **Initialization Order**: Addons should be initialized in this order: image -> font -> primitives -> ttf

2. **Shared Initialization**: Font and TTF addons share SDL2_ttf initialization

3. **Version Functions**: Return Allegro version constants for API compatibility

4. **Fallback Behavior**: Some Allegro features not in SDL2 require fallback implementations or return nullptr/error

5. **Memory Management**: All loaded fonts and resources must be properly freed before shutdown

---

## New Details from SDL Source Analysis

### SDL2 Addon Equivalents

The repository contains **SDL2** with separate addon libraries:

1. **SDL2_image**: 
   - Located in `src/video/` as `SDL_stb.c` (basic) or external SDL2_image
   - Supports: PNG, JPEG, BMP, GIF, TGA, WEBP via stb_image
   - For full format support, use external SDL2_image library

2. **SDL2_ttf**: 
   - Separate library for font rendering
   - Use `TTF_OpenFont()`, `TTF_RenderUTF8_*()` functions

3. **SDL2_mixer**:
   - Located in `src/audio/` as `SDL_mixer.c`
   - For audio - handles music and sound effects

### SDL2 Native Features

SDL2 includes features that Allegro addons provide:

| Feature | SDL2 Equivalent |
|---------|-----------------|
| Image loading | SDL2_image (external) or SDL_stb.c (built-in, limited) |
| Font rendering | SDL2_ttf (external) |
| Primitives | SDL_RenderDraw* functions in SDL_render |
| Audio | SDL2_mixer (external) |

### Dependencies Update

| Library | Notes |
|---------|-------|
| SDL2 | Core library |
| SDL2_image | Extended image support |
| SDL2_ttf | Font rendering |
| SDL2_mixer | Audio mixing |
