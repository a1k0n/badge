#include "renderer.h"
#include <math.h>
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

// torus radii and distance from camera
// these are pretty baked-in to other constants now, so it probably won't work
// if you change them too much.
const int dz = 5, r1 = 1, r2 = 2;

void badge_renderer_init(badge_renderer_t *renderer) {
    renderer->frame_count = 0;

    renderer->angleA = 0;
    renderer->angleB = 0;
}

// always call this before rendering a frame
void badge_advance_frame(badge_renderer_t *renderer) {
    renderer->frame_count++;

    // we're using a Cortex M33 which has an FPU, but only once per frame here
    // might replace with sine table so we get clean wrapping around 2pi
    float sA = sinf(renderer->angleA);
    float cA = cosf(renderer->angleA);
    float sB = sinf(renderer->angleB);
    float cB = cosf(renderer->angleB);

    renderer->sA = (int16_t) (16383.0 * sA);
    renderer->cA = (int16_t) (16383.0 * cA);

    renderer->sB = (int16_t) (16383.0 * sB);
    renderer->cB = (int16_t) (16383.0 * cB);

    renderer->sAsB = (int16_t) (16383.0 * (sA * sB));
    renderer->cAsB = (int16_t) (16383.0 * (cA * sB));
    renderer->sAcB = (int16_t) (16383.0 * (cA * cB));
    renderer->cAcB = (int16_t) (16383.0 * (cA * cB));

    //renderer->angleA += 0.07f;
    //renderer->angleB += 0.03f;

    renderer->p0x = (int32_t) (dz * renderer->sB >> 6);
    renderer->p0y = (int32_t) (dz * renderer->sAcB >> 6);
    renderer->p0z = (int32_t) (-dz * renderer->cAcB >> 6);

    // int16_t yincC = (cA >> 6) + (cA >> 5);      // 12*cA >> 8;
    // int16_t yincS = (sA >> 6) + (sA >> 5);      // 12*sA >> 8;
    // int16_t xincX = (cB >> 7) + (cB >> 6);      // 6*cB >> 8;
    // int16_t xincY = (sAsB >> 7) + (sAsB >> 6);  // 6*sAsB >> 8;
    // int16_t xincZ = (cAsB >> 7) + (cAsB >> 6);  // 6*cAsB >> 8;
    // int16_t ycA = -((cA >> 1) + (cA >> 4));     // -12 * yinc1 = -9*cA >> 4;
    // int16_t ysA = -((sA >> 1) + (sA >> 4));     // -12 * yinc2 = -9*sA >> 4;

    const int scale_factor = 2;

    renderer->yincC = scale_factor*renderer->cA >> 8;
    renderer->yincS = scale_factor*renderer->sA >> 8;

    renderer->xincX = scale_factor*renderer->cB >> 8;
    renderer->xincY = scale_factor*renderer->sAsB >> 8;
    renderer->xincZ = scale_factor*renderer->cAsB >> 8;

    renderer->ycA = -(BADGE_DISPLAY_WIDTH/2)*renderer->yincC;
    renderer->ysA = -(BADGE_DISPLAY_WIDTH/2)*renderer->yincS;
}

static int length_cordic(int16_t x, int16_t y, int16_t *x2_, int16_t y2) {
  int x2 = *x2_;
  if (x < 0) {  // start in right half-plane
    x = -x;
    x2 = -x2;
  }
  for (int i = 0; i < 8; i++) {
    int t = x;
    int t2 = x2;
    if (y < 0) {
      x -= y >> i;
      y += t >> i;
      x2 -= y2 >> i;
      y2 += t2 >> i;
    } else {
      x += y >> i;
      y -= t >> i;
      x2 += y2 >> i;
      y2 -= t2 >> i;
    }
  }
  // divide by 0.625 as a cheap approximation to the 0.607 scaling factor factor
  // introduced by this algorithm (see https://en.wikipedia.org/wiki/CORDIC)
  *x2_ = (x2 >> 1) + (x2 >> 3);
  return (x >> 1) + (x >> 3);
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

    int xsAsB = -width * renderer->xincY >> 1;
    int xcAsB = -width * renderer->xincZ >> 1;

    int16_t vxi14 = (-width * renderer->xincX >> 1) - renderer->sB;
    int16_t vyi14 = renderer->ycA - xsAsB - renderer->sAcB;
    int16_t vzi14 = renderer->ysA + xcAsB + renderer->cAcB;

    const int r1i = 256*r1;
    const int r2i = 256*r2;

    for (uint16_t i = 0; i < width; i++) {
        int t = 512;
        uint16_t x = x_offset + i;

        int16_t px = renderer->p0x + (vxi14 >> 5); // assuming t = 512, t*vxi>>8 == vxi<<1
        int16_t py = renderer->p0y + (vyi14 >> 5);
        int16_t pz = renderer->p0z + (vzi14 >> 5);

        int16_t lx0 = renderer->sB >> 2;
        int16_t ly0 = renderer->sAcB - renderer->cA >> 2;
        int16_t lz0 = -renderer->cAcB - renderer->sA >> 2;

        uint8_t r = 0, g = 0, b = 0;
        for (;;) {
            int t0, t1, t2, d;
            int16_t lx = lx0, ly = ly0, lz = lz0;
            t0 = length_cordic(px, py, &lx, ly);
            t1 = t0 - r2i;
            t2 = length_cordic(pz, t1, &lz, lx);
            d = t2 - r1i;
            t += d;
            if (t > 8*256) {
                break;
            } else if (d < 2) {
              //int N = lz >> 9;
              //putchar(".,-~:;!*=#$@"[N > 0 ? N < 12 ? N : 11 : 0]);
              if (lz > 0) {
                g = lz >> 7;
                r = lz >> 8;
              }
              break;
            }

            px += d*vxi14 >> 14;
            py += d*vyi14 >> 14;
            pz += d*vzi14 >> 14;

        }

        vxi14 += renderer->xincX;
        vyi14 -= renderer->xincY;
        vzi14 += renderer->xincZ;

        pixels[i] = BADGE_RGB565(r, g, b);
    }

    renderer->ycA += renderer->yincC;
    renderer->ysA += renderer->yincS;
}
