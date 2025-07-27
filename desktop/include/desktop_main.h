#ifndef DESKTOP_MAIN_H
#define DESKTOP_MAIN_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "renderer.h"

#define DESKTOP_SCALE_FACTOR 4
#define DESKTOP_WINDOW_WIDTH (BADGE_DISPLAY_WIDTH * DESKTOP_SCALE_FACTOR)
#define DESKTOP_WINDOW_HEIGHT (BADGE_DISPLAY_HEIGHT * DESKTOP_SCALE_FACTOR)

// Desktop application context
typedef struct {
    SDL_Window *window;
    SDL_Renderer *sdl_renderer;
    SDL_Texture *texture;
    
    badge_renderer_t renderer;
    
    uint32_t frame_count;
    bool running;
    
    // Performance tracking
    uint32_t last_frame_time;
    uint32_t target_frame_time; // 33ms for ~30 FPS
} desktop_context_t;

// Application lifecycle
bool desktop_init(desktop_context_t *ctx);
void desktop_run(desktop_context_t *ctx);
void desktop_shutdown(desktop_context_t *ctx);

// Event handling
void desktop_handle_events(desktop_context_t *ctx);

// Rendering functions
void desktop_render_frame(desktop_context_t *ctx);
void desktop_update_display(desktop_context_t *ctx);

// Utility functions
uint32_t desktop_rgb565_to_rgb888(badge_color_t color);

#endif // DESKTOP_MAIN_H 