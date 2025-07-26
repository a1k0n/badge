#ifndef BADGE_MAIN_H
#define BADGE_MAIN_H

#include "gc9a01.h"
#include "renderer.h"

// Main badge application context
typedef struct {
    gc9a01_t display;
    badge_renderer_t renderer;
    badge_color_t scanline_buffer[BADGE_DISPLAY_WIDTH]; // Racing-the-beam scanline buffer
    uint32_t frame_count;
    bool running;
} badge_context_t;

// Application lifecycle
bool badge_init(badge_context_t *ctx);
void badge_run(badge_context_t *ctx);
void badge_shutdown(badge_context_t *ctx);

// Rendering functions
void badge_render_frame(badge_context_t *ctx);

#endif // BADGE_MAIN_H 