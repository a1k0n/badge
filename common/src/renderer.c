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
    renderer->frame_count = 0;
    renderer->time_ms = 0;
}

void badge_advance_frame(badge_renderer_t *renderer) {
    renderer->frame_count++;
    renderer->time_ms += 33; // Assume ~30 FPS
}

void badge_render_scanline(badge_renderer_t *renderer, badge_color_t *pixels,
                          uint16_t x_offset, uint16_t y, uint16_t width) {
    // Bounds checking
    if (y >= renderer->display_height) return;
    if (x_offset >= renderer->display_width) return;
    
    // Clamp width to display bounds
    if (x_offset + width > renderer->display_width) {
        width = renderer->display_width - x_offset;
    }
    
    // Generate animated test pattern scanline
    // Add vertical scroll animation using frame counter
    uint16_t animated_y = (y + renderer->frame_count) % (renderer->display_height * 2);
    
    for (uint16_t i = 0; i < width; i++) {
        uint16_t x = x_offset + i;
        
        // Create animated test pattern with gradients and scrolling
        uint8_t r = (x * 31) / renderer->display_width;
        uint8_t g = (animated_y * 63) / (renderer->display_height * 2);
        uint8_t b = ((x + animated_y) * 31) / (renderer->display_width + renderer->display_height * 2);
        
        pixels[i] = BADGE_RGB565(r, g, b);
    }
    
    // If we have a framebuffer (desktop mode), copy scanline data
    if (renderer->framebuffer) {
        badge_color_t *dest = &renderer->framebuffer[y * renderer->display_width + x_offset];
        memcpy(dest, pixels, width * sizeof(badge_color_t));
    }
}

bool badge_is_pixel_in_circle(uint16_t x, uint16_t y, uint16_t center_x, uint16_t center_y, uint16_t radius) {
    int32_t dx = (int32_t)x - (int32_t)center_x;
    int32_t dy = (int32_t)y - (int32_t)center_y;
    return (dx * dx + dy * dy) <= (int32_t)(radius * radius);
} 