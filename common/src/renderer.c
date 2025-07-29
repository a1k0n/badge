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

const uint16_t palette[] = {
    0x0000,  // (0, 0, 0)
    0x0000,  // (1, 0, 0)
    0x0820,  // (2, 1, 0)
    0x0840,  // (3, 2, 0)
    0x1060,  // (4, 3, 1)
    0x1080,  // (5, 4, 1)
    0x18a0,  // (6, 5, 1)
    0x18a0,  // (7, 5, 1)
    0x20c1,  // (8, 6, 2)
    0x20e1,  // (9, 7, 2)
    0x2901,  // (11, 8, 2)
    0x3121,  // (12, 9, 2)
    0x3141,  // (13, 10, 3)
    0x3941,  // (14, 10, 3)
    0x3961,  // (15, 11, 3)
    0x4181,  // (16, 12, 3)
    0x41a2,  // (17, 13, 4)
    0x49c2,  // (18, 14, 4)
    0x49e2,  // (19, 15, 4)
    0x5202,  // (21, 16, 5)
    0x5a02,  // (22, 16, 5)
    0x5a22,  // (23, 17, 5)
    0x6242,  // (24, 18, 5)
    0x6263,  // (25, 19, 6)
    0x6a83,  // (26, 20, 6)
    0x6aa3,  // (27, 21, 6)
    0x72c3,  // (28, 22, 6)
    0x7ac3,  // (30, 22, 7)
    0x7ae3,  // (31, 23, 7)
    0x8303,  // (32, 24, 7)
    0x8324,  // (33, 25, 8)
    0x8b44,  // (34, 26, 8)
    0x8b64,  // (35, 27, 9)
    0x9384,  // (37, 28, 9)
    0x9ba4,  // (38, 29, 9)
    0x9bc5,  // (39, 30, 10)
    0xa3e5,  // (41, 31, 11)
    0xac25,  // (42, 33, 11)
    0xb446,  // (44, 34, 12)
    0xbc66,  // (46, 35, 13)
    0xbca7,  // (47, 37, 14)
    0xc4e7,  // (49, 39, 15)
    0xcd08,  // (51, 40, 16)
    0xdd48,  // (54, 42, 17)
    0xe589,  // (56, 44, 19)
    0xedea,  // (59, 47, 21)
    0xfe4b,  // (62, 50, 23)
    0xfeac,  // (63, 53, 25)
    0xff0e,  // (63, 56, 28)
    0xff8f,  // (63, 60, 31)
    0xfff1,  // (63, 63, 35)
    0xfff3,  // (63, 63, 39)
    0xfff5,  // (63, 63, 43)
    0xfff8,  // (63, 63, 49)
    0xfffb,  // (63, 63, 55)
    0xfffe,  // (63, 63, 61)
    0xffff,  // (63, 63, 63)
};
const int NPALETTE = sizeof(palette) / sizeof(palette[0]);

// torus radii and distance from camera
// these are pretty baked-in to other constants now, so it probably won't work
// if you change them too much.
const int dz = 5, r1 = 1, r2 = 2;

void rotate2(float x, float y, float s, float c, float *xout, float *yout) {
    *xout = x*c - y*s;
    *yout = x*s + y*c;
}

float length2(float x, float y) {
    return sqrtf(x*x + y*y);
}

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
  renderer->sA = sinf(renderer->angleA);
  renderer->cA = cosf(renderer->angleA);
  renderer->sB = sinf(renderer->angleB);
  renderer->cB = cosf(renderer->angleB);

  renderer->angleA += 0.037;
  renderer->angleB += 0.011;
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

  float rox = 0, roy = 0, roz = -4.5;
  float Lx = 0.5, Ly = 0.5, Lz = -1.0;

  // rotate yz by A
  rotate2(roy, roz, renderer->sA, renderer->cA, &roy, &roz);
  rotate2(Ly, Lz, renderer->sA, renderer->cA, &Ly, &Lz);
  // rotate xz by B
  rotate2(rox, roz, renderer->sB, renderer->cB, &rox, &roz);
  rotate2(Lx, Lz, renderer->sB, renderer->cB, &Lx, &Lz);

  int ooy = 65536 / (y + 50);
  int xcheck = -width/2 * ooy + (renderer->frame_count << 7);
  int ycheck = ((ooy + renderer->frame_count) & 0x40) ? 1 : 0;

  for (uint16_t i = 0; i < width; i++) {
    float rdx = (x_offset + i - 120) / 120.0;
    float rdy = (y - 120) / 120.0;
    float rdz = 1.0;

    // rotate yz by A
    rotate2(rdy, rdz, renderer->sA, renderer->cA, &rdy, &rdz);
    // rotate xz by B
    rotate2(rdx, rdz, renderer->sB, renderer->cB, &rdx, &rdz);

    int xcheck2 = xcheck & 0x4000 ? 1 : 0;
    // (0, 21, 63) : (42, 42, 42)
    uint16_t color = (xcheck2 ^ ycheck) ? (21 << 5) | 31 : (21 << 11) | (42 << 5) | 21;
    float t = 0.0;
    for (int j = 0; j < 20; j++) {
      /*
            vec3 pos = ro + t * rd;
            float lxy = length(pos.xy);
            float d = lxy - 2.0;
            vec2 uxy = 2.*pos.xy / lxy;
            float ldz = length(vec2(d, pos.z));
            float d2 = ldz - 1.0;
            t += d2;
            if (d2 < 0.05) {
                vec3 N = pos - vec3(uxy, 0.0);
                float l = 0.7*(dot(N, L));
                color = 0.2*color + 0.8*vec3(l, l, l);
                break;
            }
            if (t > 15.0) {
                // color.y += float(i-1)*.025;
                break;
            }
      */
      float px = rox + t * rdx;
      float py = roy + t * rdy;
      float pz = roz + t * rdz;
      float lxy = length2(px, py);
      float d = lxy - 2.0;
      float ux = 2. * px / lxy;
      float uy = 2. * py / lxy;
      float ldz = length2(d, pz);
      float d2 = ldz - 1.0;
      t += d2;
      if (d2 < 0.1) {
        float Nx = px - ux;
        float Ny = py - uy;
        float Nz = pz;
        float l = 0.6*(0.5 + Nx * Lx + Ny * Ly + Nz * Lz);
        if (l > 0) {
          if (l > 1) l = 1;
          color = palette[(int)(l * (NPALETTE - 1))];
        } else {
          color = 0x0000;
        }
        break;
      }
      if (t > 15.0) {
        break;
      }
    }

    pixels[i] = color;

    xcheck += ooy;
  }
}
