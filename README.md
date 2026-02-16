# Allegro Shim

A compatibility shim layer that allows Allegro 5 code to run on top of SDL2.

## Overview

This project provides a drop-in replacement for the Allegro 5 library, implemented using SDL2 as the underlying backend. It enables existing Allegro 5 applications to be ported to SDL2 with minimal code changes.

## Features

- Display management (window creation, resizing, fullscreen)
- Color system (RGB, RGBA, float variants)
- Bitmap management and rendering
- Drawing primitives (rectangles, lines, circles, polygons)
- Event handling
- Input support (keyboard, mouse, joystick)
- Audio playback
- Threading primitives
- File I/O and configuration

## Building

Requires CMake, SDL2, SDL2_image, SDL2_mixer, and SDL2_ttf.

```bash
mkdir build && cd build
cmake ..
make
```

## Architecture

The shim maps Allegro 5 types to their SDL2 equivalents:

| Allegro Type | SDL2 Equivalent |
|--------------|-----------------|
| ALLEGRO_DISPLAY | SDL_Window* + SDL_Renderer* |
| ALLEGRO_BITMAP | SDL_Texture* |
| ALLEGRO_COLOR | SDL_Color (float r,g,b,a) |
| ALLEGRO_EVENT_QUEUE | std::queue<SDL_Event> |
| ALLEGRO_MUTEX | SDL_mutex* |
| ALLEGRO_SAMPLE | Mix_Chunk* |
| ALLEGRO_FONT | TTF_Font* |

## Modules

- **Core**: Display, events, initialization
- **Graphics**: Bitmaps, drawing, colors, fonts
- **Audio**: Samples, streams, mixer
- **Input**: Keyboard, mouse, joystick
- **Threading**: Mutex, cond, thread
- **File I/O**: Config, file operations
- **Addons**: Image, font, TTF, primitives

## Documentation

See `spec/README.md` for detailed specification documents and implementation plans.

## License

See project repository for license information.
