#include "renderer.h"
#include <string.h>

const uint8_t mask_x_offset[240] = {
    109, 101, 95, 91, 87,  84, 81, 78, 75, 73, 70, 68, 66, 64, 62, 61, 59, 57,
    55,  54,  52, 51, 50,  48, 47, 46, 44, 43, 42, 41, 40, 38, 37, 36, 35, 34,
    33,  32,  31, 31, 30,  29, 28, 27, 26, 25, 25, 24, 23, 22, 22, 21, 20, 20,
    19,  18,  18, 17, 16,  16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,
    9,   9,   8,  8,  8,   7,  7,  7,  6,  6,  6,  5,  5,  5,  4,  4,  4,  3,
    3,   3,   3,  2,  2,   2,  2,  2,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,
    0,   0,   0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,   0,   0,  0,  0,   0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  2,  2,
    2,   2,   2,  3,  3,   3,  3,  4,  4,  4,  5,  5,  5,  6,  6,  6,  7,  7,
    7,   8,   8,  8,  9,   9,  10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
    16,  16,  17, 18, 18,  19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 28,
    29,  30,  31, 31, 32,  33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 44, 46, 47,
    48,  50,  51, 52, 54,  55, 57, 59, 61, 62, 64, 66, 68, 70, 73, 75, 78, 81,
    84,  87,  91, 95, 101, 109};

void badge_renderer_init(badge_renderer_t *renderer) {
    renderer->frame_count = 0;
}

void badge_advance_frame(badge_renderer_t *renderer) {
    renderer->frame_count++;
}

void badge_render_scanline(badge_renderer_t *renderer, badge_color_t *pixels,
                          uint16_t x_offset, uint16_t y, uint16_t width) {
    // Bounds checking
    if (y >= BADGE_DISPLAY_HEIGHT) return;
    if (x_offset >= BADGE_DISPLAY_WIDTH) return;
    
    // Clamp width to display bounds
    if (x_offset + width > BADGE_DISPLAY_WIDTH) {
        width = BADGE_DISPLAY_WIDTH - x_offset;
    }
    
    // Generate animated test pattern scanline
    // Add vertical scroll animation using frame counter
    uint16_t animated_y = (y + renderer->frame_count) % (BADGE_DISPLAY_HEIGHT * 2);
    
    for (uint16_t i = 0; i < width; i++) {
        uint16_t x = x_offset + i;
        
        // Create animated test pattern with gradients and scrolling
        uint8_t r = (x * 31) / BADGE_DISPLAY_WIDTH;
        uint8_t g = (animated_y * 63) / (BADGE_DISPLAY_HEIGHT * 2);
        uint8_t b = ((x + animated_y) * 31) / (BADGE_DISPLAY_WIDTH + BADGE_DISPLAY_HEIGHT * 2);
        
        pixels[i] = BADGE_RGB565(r, g, b);
    }
}
