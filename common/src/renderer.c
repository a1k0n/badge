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

const uint16_t palette1_r[] = {
    0x0,  0x0,  0x1,  0x1,  0x2,  0x2,  0x3,  0x3,  0x4,  0x4,  0x5,  0x6,
    0x6,  0x7,  0x7,  0x8,  0x8,  0x9,  0x9,  0xa,  0xb,  0xb,  0xc,  0xc,
    0xd,  0xd,  0xe,  0xf,  0xf,  0x10, 0x10, 0x11, 0x11, 0x12, 0x13, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x17, 0x18, 0x19, 0x1b, 0x1c, 0x1d, 0x1f, 0x1f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};

const uint16_t palette1_g[] = {
    0x0,  0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x5,  0x6,  0x7,  0x8,  0x9,
    0xa,  0xa,  0xb,  0xc,  0xd,  0xe,  0xf,  0x10, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
    0x1f, 0x21, 0x22, 0x23, 0x25, 0x27, 0x28, 0x2a, 0x2c, 0x2f, 0x32, 0x35,
    0x38, 0x3c, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f};

const uint16_t palette1_b[] = {
    0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x1, 0x1, 0x1,
    0x1, 0x1, 0x1,  0x1,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2, 0x2, 0x3,
    0x3, 0x3, 0x3,  0x3,  0x3,  0x3,  0x4,  0x4,  0x4,  0x4, 0x4, 0x5,
    0x5, 0x5, 0x6,  0x6,  0x7,  0x7,  0x8,  0x8,  0x9,  0xa, 0xb, 0xc,
    0xe, 0xf, 0x11, 0x13, 0x15, 0x18, 0x1b, 0x1e, 0x1f, 0x1f};

const uint16_t palette2_r[] = {
    0x0,  0x0,  0x1,  0x1,  0x2,  0x2,  0x3,  0x3,  0x4,  0x4,  0x5,  0x6,
    0x6,  0x7,  0x7,  0x8,  0x8,  0x9,  0x9,  0xa,  0xb,  0xb,  0xc,  0xc,
    0xd,  0xd,  0xe,  0xf,  0xf,  0x10, 0x10, 0x11, 0x11, 0x12, 0x13, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x17, 0x18, 0x19, 0x1b, 0x1c, 0x1d, 0x1f, 0x1f,
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
const uint16_t palette2_g[] = {
    0x0, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,
    0x0, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,
    0x0, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x1, 0x1,
    0x1, 0x1,  0x2,  0x3,  0x3,  0x4,  0x5,  0x6,  0x7,  0x9, 0xb, 0xd,
    0xf, 0x12, 0x16, 0x19, 0x1e, 0x23, 0x28, 0x2f, 0x36, 0x3f};
const uint16_t palette2_b[] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,
    0x0, 0x0, 0x1, 0x1, 0x1, 0x2,  0x2,  0x3,  0x3,  0x4, 0x5, 0x6,
    0x7, 0x9, 0xb, 0xc, 0xf, 0x11, 0x14, 0x17, 0x1b, 0x1f};

const int NPALETTE = sizeof(palette1_r) / sizeof(palette1_r[0]);

void rotate2(float x, float y, float s, float c, float *xout, float *yout) {
  *xout = x * c - y * s;
  *yout = x * s + y * c;
}

float length2(float x, float y) { return sqrtf(x * x + y * y); }

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

float fast_atan2f(float y, float x) {
  // https://math.stackexchange.com/a/1105038
  // Fast approximation of atan2 with max error ~0.07 radians
  const float abs_y = fabsf(y);
  const float abs_x = fabsf(x);
  const float a = fminf(abs_x, abs_y) / fmaxf(abs_x, abs_y);
  const float s = a * a;
  float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;

  if (abs_y > abs_x) r = 1.57079637f - r;
  if (x < 0) r = 3.14159274f - r;
  if (y < 0) r = -r;

  return r;
}

// step from a to b smoothly as t goes from 0 to 1
float smoothstep(float a, float b, float t) {
  t = t * t * (3 - 2 * t);
  return t * (b - a) + a;
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

  float drawdist = 4.5;
  float rox = 0, roy = 0, roz = -drawdist;
  float Lx = 0.5, Ly = 0.5, Lz = -1.0;

  int bg10_r = 21, bg10_g = 42, bg10_b = 21;
  int bg11_r = 0, bg11_g = 21, bg11_b = 31;
  int bg20_r = 21, bg20_g = 42, bg20_b = 21;
  int bg21_r = 21, bg21_g = 0, bg21_b = 21;

  float r1 = 2.0, r2 = 1;
  int fc = renderer->frame_count & 511;
  int palette = 0;
  // start with r=(2,1), then after ~4 seconds @ 50Hz (192 frames), interpolate
  // so that we get to r=(0,2.5) on frame 256, then interpolate so that we
  // return to (2,1) during the last 64 frames of the 512 frame cycle
  if (fc < 192) {
    r1 = 2.0;
    r2 = 1;
  } else if (fc < 256) {
    float t = (fc - 192) / 64.0f;
    r1 = smoothstep(2.0f, 0.0f, t);
    r2 = smoothstep(1.0f, 2.5f, t);
    palette = smoothstep(0, 256, t);
  } else if (fc < 448) {
    r1 = 0.0f;
    r2 = 2.5f;
    palette = 256;
  } else {
    float t = (fc - 448) / 64.0f;
    r1 = smoothstep(0.0f, 2.0f, t);
    r2 = smoothstep(2.5f, 1.0f, t);
    palette = smoothstep(256, 0, t);
  }

  int bg0_r = (bg10_r * (256 - palette) + bg20_r * palette) >> 8;
  int bg0_g = (bg10_g * (256 - palette) + bg20_g * palette) >> 8;
  int bg0_b = (bg10_b * (256 - palette) + bg20_b * palette) >> 8;
  int bg1_r = (bg11_r * (256 - palette) + bg21_r * palette) >> 8;
  int bg1_g = (bg11_g * (256 - palette) + bg21_g * palette) >> 8;
  int bg1_b = (bg11_b * (256 - palette) + bg21_b * palette) >> 8;
  uint16_t bg0 = (bg0_r << 11) | (bg0_g << 5) | bg0_b;
  uint16_t bg1 = (bg1_r << 11) | (bg1_g << 5) | bg1_b;

  // rotate yz by A
  rotate2(roy, roz, renderer->sA, renderer->cA, &roy, &roz);
  rotate2(Ly, Lz, renderer->sA, renderer->cA, &Ly, &Lz);
  // rotate xz by B
  rotate2(rox, roz, renderer->sB, renderer->cB, &rox, &roz);
  rotate2(Lx, Lz, renderer->sB, renderer->cB, &Lx, &Lz);

  int ooy = 65536 / (y + 50);
  int xcheck = -width / 2 * ooy + (renderer->frame_count << 9);
  int ycheck = ((ooy + (renderer->frame_count << 1)) & 0x40) ? 1 : 0;

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
    uint16_t color = (xcheck2 ^ ycheck) ? bg0 : bg1;
    float t = drawdist - r2 - r1 * 1.5;
    float px = rdx * t + rox;
    float py = rdy * t + roy;
    float pz = rdz * t + roz;
    for (int j = 0; j < 20; j++) {
      /*
      ([(x0, rdx*t + rox), -> px
        (x1, rdy*t + roy), -> py
        (x2, sqrt(x0**2 + x1**2)), -> lxy
        (x3, rdz*t + roz), -> pz
        (x4, r1 - x2), -> -d1
        (x5, sqrt(x3**2 + x4**2))], -> ldz
       [x2*x5*(r2 - x5)/(rdz*x2*x3 - x4*(rdx*x0 + rdy*x1))])

      float px = rox + t * rdx;
      float py = roy + t * rdy;
      float pz = roz + t * rdz;
      float rxy = 0, rdz = 0;
      float lxy = length2(px, py, &rxy);
      float d = lxy - r1;
      float ldz = length2(d, pz, &rdz);
      float dt = ldz - r2;
      t += dt;
      */
      float lxy = length2(px, py);
      float d1 = lxy - r1;
      float ldz = length2(pz, d1);
      // [x2*x5*(-r2 + x5)/(rdz*x2*x3 - x4*(rdx*x0 + rdy*x1))])
      float d2 = ldz - r2;
      // float dt = -lxy*ldz*d2/(rdz*lxy*pz + d1*(rdx*px + rdy*py));

      float dt = d2;
      // if (dt > 0.5) dt = 0.5;
      // if (dt < -0.5) dt = -0.5;

      px += rdx * dt;
      py += rdy * dt;
      pz += rdz * dt;

      if (dt > -0.05 && dt < 0.05) {
        float Nx = px - r1 * px / lxy;
        float Ny = py - r1 * py / lxy;
        float Nz = pz;
        float Nmag = 1.0 / r2;  // 1.0/sqrt(Nx*Nx + Ny*Ny + Nz*Nz);
        float rxy = fast_atan2f(py, px);
        float rxz = fast_atan2f(pz, d1);
        int lxyi = (int)(rxy * 256.0 / M_PI);
        int lxzi = (int)(rxz * 256.0 / M_PI);
        float check_xy = (lxyi ^ lxzi) & 0x20 ? 1.0 + palette / 256.0 : 0.0;
        float l = Nmag * 0.6 * (0.2 + Nx * Lx + Ny * Ly + Nz * Lz + check_xy);
        if (l > 0) {
          if (l > 1) l = 1;
          int k = (int)(l * (NPALETTE - 1));
          int r =
              (palette1_r[k] * (256 - palette) + palette2_r[k] * palette) >> 8;
          int g =
              (palette1_g[k] * (256 - palette) + palette2_g[k] * palette) >> 8;
          int b =
              (palette1_b[k] * (256 - palette) + palette2_b[k] * palette) >> 8;
          color = (r << 11) | (g << 5) | b;
        } else {
          color = 0x0000;
        }
        break;
      }

      t += dt;
      if (t > drawdist + r2 + r1 + 0.5) {
        break;
      }
    }

    pixels[i] = color;

    xcheck += ooy;
  }
}
