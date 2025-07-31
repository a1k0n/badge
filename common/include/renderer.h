#ifndef BADGE_RENDERER_H
#define BADGE_RENDERER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BADGE_DISPLAY_WIDTH 240
#define BADGE_DISPLAY_HEIGHT 240

// RGB565 color format (16-bit)
typedef uint16_t badge_color_t;

// RGB565 color helpers
#define BADGE_COLOR_BLACK 0x0000
#define BADGE_COLOR_WHITE 0xFFFF
#define BADGE_COLOR_RED 0xF800
#define BADGE_COLOR_GREEN 0x07E0
#define BADGE_COLOR_BLUE 0x001F

// Endian swapping for display compatibility
// Embedded targets need endian swap, desktop does not
#ifdef BUILD_EMBEDDED
#define BADGE_RGB565(r, g, b) \
  __builtin_bswap16(((b & 0x1F) << 11) | ((g & 0x3F) << 5) | (r & 0x1F))
#else
#define BADGE_RGB565(r, g, b) \
  (((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))
#endif

extern const uint8_t mask_x_offset[240];
#define BADGE_MASK_X_OFFSET(y) mask_x_offset[y]
#define BADGE_MASK_X_WIDTH(y) 240 - (mask_x_offset[y] << 1)

// Racing-the-beam renderer context with animation state
typedef struct {
  // Animation state
  uint32_t frame_count;

  // rotation matrix parameters
  float sA, cA;  // sin/cos of A
  float sB, cB;  // sin/cos of B

  int angleA, angleB;
} badge_renderer_t;

// Initialize renderer
void badge_renderer_init(badge_renderer_t *renderer);

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
bool badge_is_pixel_in_circle(uint16_t x, uint16_t y, uint16_t center_x,
                              uint16_t center_y, uint16_t radius);

#endif  // BADGE_RENDERER_H