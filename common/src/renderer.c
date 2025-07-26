#include "renderer.h"
#include <string.h>

void badge_renderer_init(badge_renderer_t *renderer, badge_color_t *framebuffer) {
    renderer->framebuffer = framebuffer;
    renderer->display_width = BADGE_DISPLAY_WIDTH;
    renderer->display_height = BADGE_DISPLAY_HEIGHT;
    renderer->center_x = BADGE_DISPLAY_WIDTH / 2;
    renderer->center_y = BADGE_DISPLAY_HEIGHT / 2;
    renderer->radius = BADGE_DISPLAY_WIDTH / 2;
    renderer->use_circular_clipping = false;
}

void badge_render_scanline(badge_renderer_t *renderer, badge_scanline_t *scanline,
                          uint16_t x_offset, uint16_t y, uint16_t width) {
    // Bounds checking
    if (y >= renderer->display_height) return;
    if (x_offset >= renderer->display_width) return;
    
    // Clamp width to display bounds
    if (x_offset + width > renderer->display_width) {
        width = renderer->display_width - x_offset;
    }
    
    // Update scanline metadata
    scanline->x_offset = x_offset;
    scanline->y_coord = y;
    scanline->width = width;
    
    // If we have a framebuffer (desktop mode), copy scanline data
    if (renderer->framebuffer) {
        badge_color_t *dest = &renderer->framebuffer[y * renderer->display_width + x_offset];
        memcpy(dest, scanline->pixels, width * sizeof(badge_color_t));
    }
}

void badge_fill_scanline(badge_scanline_t *scanline, badge_color_t color) {
    for (int i = 0; i < BADGE_DISPLAY_WIDTH; i++) {
        scanline->pixels[i] = color;
    }
}

void badge_test_pattern_scanline(badge_scanline_t *scanline, uint16_t y) {
    for (int x = 0; x < BADGE_DISPLAY_WIDTH; x++) {
        // Create a test pattern with gradients and stripes
        uint8_t r = (x * 31) / BADGE_DISPLAY_WIDTH;
        uint8_t g = (y * 63) / BADGE_DISPLAY_HEIGHT;
        uint8_t b = ((x + y) * 31) / (BADGE_DISPLAY_WIDTH + BADGE_DISPLAY_HEIGHT);
        
        scanline->pixels[x] = BADGE_RGB565(r, g, b);
    }
}

bool badge_is_pixel_in_circle(uint16_t x, uint16_t y, uint16_t center_x, uint16_t center_y, uint16_t radius) {
    int32_t dx = (int32_t)x - (int32_t)center_x;
    int32_t dy = (int32_t)y - (int32_t)center_y;
    return (dx * dx + dy * dy) <= (int32_t)(radius * radius);
} 