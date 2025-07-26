#ifndef BADGE_RENDERER_H
#define BADGE_RENDERER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BADGE_DISPLAY_WIDTH 240
#define BADGE_DISPLAY_HEIGHT 240

// RGB565 color format (16-bit)
typedef uint16_t badge_color_t;

// RGB565 color helpers
#define BADGE_RGB565(r, g, b) (((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))
#define BADGE_COLOR_BLACK   0x0000
#define BADGE_COLOR_WHITE   0xFFFF
#define BADGE_COLOR_RED     0xF800
#define BADGE_COLOR_GREEN   0x07E0
#define BADGE_COLOR_BLUE    0x001F

// Racing-the-beam renderer context with animation state
typedef struct {
    // Current frame buffer (if needed for desktop)
    badge_color_t *framebuffer;
    
    // Display dimensions
    uint16_t display_width;
    uint16_t display_height;
    
    // Animation state
    uint32_t frame_count;
    uint32_t time_ms;
    
    // Circular clipping (for future use)
    uint16_t center_x;
    uint16_t center_y;
    uint16_t radius;
    bool use_circular_clipping;
} badge_renderer_t;

// Initialize renderer
void badge_renderer_init(badge_renderer_t *renderer, badge_color_t *framebuffer);

// Advance frame for animations (call once per frame)
void badge_advance_frame(badge_renderer_t *renderer);

// Racing-the-beam scanline renderer - generates pixels on demand
// pixels: output buffer for scanline pixels
// x_offset: starting X coordinate for the scanline  
// y: Y coordinate of the scanline
// width: number of pixels to render
void badge_render_scanline(badge_renderer_t *renderer, badge_color_t *pixels,
                          uint16_t x_offset, uint16_t y, uint16_t width);

// Utility functions
bool badge_is_pixel_in_circle(uint16_t x, uint16_t y, uint16_t center_x, uint16_t center_y, uint16_t radius);

#endif // BADGE_RENDERER_H 