#include "renderer.h"

#include <math.h>
#include <string.h>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// sin(0..pi/2)
const float halfsintbl[256] = {
    0.,         0.00613588, 0.01227154, 0.01840673, 0.02454123, 0.0306748,
    0.03680722, 0.04293826, 0.04906767, 0.05519524, 0.06132074, 0.06744392,
    0.07356456, 0.07968244, 0.08579731, 0.09190896, 0.09801714, 0.10412163,
    0.11022221, 0.11631863, 0.12241068, 0.12849811, 0.13458071, 0.14065824,
    0.14673047, 0.15279719, 0.15885814, 0.16491312, 0.17096189, 0.17700422,
    0.18303989, 0.18906866, 0.19509032, 0.20110463, 0.20711138, 0.21311032,
    0.21910124, 0.22508391, 0.23105811, 0.23702361, 0.24298018, 0.24892761,
    0.25486566, 0.26079412, 0.26671276, 0.27262136, 0.27851969, 0.28440754,
    0.29028468, 0.29615089, 0.30200595, 0.30784964, 0.31368174, 0.31950203,
    0.32531029, 0.33110631, 0.33688985, 0.34266072, 0.34841868, 0.35416353,
    0.35989504, 0.365613,   0.37131719, 0.37700741, 0.38268343, 0.38834505,
    0.39399204, 0.3996242,  0.40524131, 0.41084317, 0.41642956, 0.42200027,
    0.42755509, 0.43309382, 0.43861624, 0.44412214, 0.44961133, 0.45508359,
    0.46053871, 0.4659765,  0.47139674, 0.47679923, 0.48218377, 0.48755016,
    0.49289819, 0.49822767, 0.50353838, 0.50883014, 0.51410274, 0.51935599,
    0.52458968, 0.52980362, 0.53499762, 0.54017147, 0.54532499, 0.55045797,
    0.55557023, 0.56066158, 0.56573181, 0.57078075, 0.57580819, 0.58081396,
    0.58579786, 0.5907597,  0.5956993,  0.60061648, 0.60551104, 0.61038281,
    0.61523159, 0.62005721, 0.62485949, 0.62963824, 0.63439328, 0.63912444,
    0.64383154, 0.6485144,  0.65317284, 0.65780669, 0.66241578, 0.66699992,
    0.67155895, 0.6760927,  0.680601,   0.68508367, 0.68954054, 0.69397146,
    0.69837625, 0.70275474, 0.70710678, 0.7114322,  0.71573083, 0.72000251,
    0.72424708, 0.72846439, 0.73265427, 0.73681657, 0.74095113, 0.74505779,
    0.74913639, 0.7531868,  0.75720885, 0.76120239, 0.76516727, 0.76910334,
    0.77301045, 0.77688847, 0.78073723, 0.7845566,  0.78834643, 0.79210658,
    0.7958369,  0.79953727, 0.80320753, 0.80684755, 0.8104572,  0.81403633,
    0.81758481, 0.82110251, 0.8245893,  0.82804505, 0.83146961, 0.83486287,
    0.83822471, 0.84155498, 0.84485357, 0.84812034, 0.85135519, 0.85455799,
    0.85772861, 0.86086694, 0.86397286, 0.86704625, 0.87008699, 0.87309498,
    0.87607009, 0.87901223, 0.88192126, 0.8847971,  0.88763962, 0.89044872,
    0.8932243,  0.89596625, 0.89867447, 0.90134885, 0.90398929, 0.9065957,
    0.90916798, 0.91170603, 0.91420976, 0.91667906, 0.91911385, 0.92151404,
    0.92387953, 0.92621024, 0.92850608, 0.93076696, 0.9329928,  0.93518351,
    0.93733901, 0.93945922, 0.94154407, 0.94359346, 0.94560733, 0.94758559,
    0.94952818, 0.95143502, 0.95330604, 0.95514117, 0.95694034, 0.95870347,
    0.96043052, 0.9621214,  0.96377607, 0.96539444, 0.96697647, 0.96852209,
    0.97003125, 0.97150389, 0.97293995, 0.97433938, 0.97570213, 0.97702814,
    0.97831737, 0.97956977, 0.98078528, 0.98196387, 0.98310549, 0.98421009,
    0.98527764, 0.9863081,  0.98730142, 0.98825757, 0.98917651, 0.99005821,
    0.99090264, 0.99170975, 0.99247953, 0.99321195, 0.99390697, 0.99456457,
    0.99518473, 0.99576741, 0.99631261, 0.9968203,  0.99729046, 0.99772307,
    0.99811811, 0.99847558, 0.99879546, 0.99907773, 0.99932238, 0.99952942,
    0.99969882, 0.99983058, 0.9999247,  0.99998118};

float fast_sin(int x) {
  // 0..255 -> 0..pi/2, return halfsintbl[x]
  // 256..511 -> pi/2..pi, return halfsintbl[255 - x]
  // 512..767 -> pi..3pi/2, return -halfsintbl[x - 512]
  // 768..1023 -> 3pi/2..2pi, return -halfsintbl[1023 - x]
  int i = x & 0xff;
  switch (x & 0x300) {
    case 0x000:
      return halfsintbl[i];
    case 0x100:
      return halfsintbl[255 - i];
    case 0x200:
      return -halfsintbl[i];
    case 0x300:
      return -halfsintbl[255 - i];
  }
}

float fast_cos(int x) {
  return fast_sin(x + 256);
}

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
  renderer->sA = fast_sin(renderer->angleA);
  renderer->cA = fast_cos(renderer->angleA);
  renderer->sB = fast_sin(renderer->angleB);
  renderer->cB = fast_cos(renderer->angleB);

  renderer->angleA += 1;
  renderer->angleB += 5;
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
  uint16_t bg0 = BADGE_RGB565(bg0_r, bg0_g, bg0_b);
  uint16_t bg1 = BADGE_RGB565(bg1_r, bg1_g, bg1_b);

  // rotate yz by A
  rotate2(roy, roz, renderer->sA, renderer->cA, &roy, &roz);
  rotate2(Ly, Lz, renderer->sA, renderer->cA, &Ly, &Lz);
  // rotate xz by B
  rotate2(rox, roz, renderer->sB, renderer->cB, &rox, &roz);
  rotate2(Lx, Lz, renderer->sB, renderer->cB, &Lx, &Lz);

  int ooy = 65536 / (y + 50);
  int xcheck = -width / 2 * ooy + (renderer->frame_count << 9);
  int ycheck = ((ooy + (renderer->frame_count << 1)) & 0x40) ? 1 : 0;

  float rdy = (y - 120) / 120.0;
  float rdz1;
  // rotate yz by A
  rotate2(rdy, 1.0, renderer->sA, renderer->cA, &rdy, &rdz1);

  // Pre-compute constants for better performance
  const float inv_120 = 1.0f / 120.0f;
  const float sB = renderer->sB;
  const float cB = renderer->cB;
  
  for (uint16_t i = 0; i < width; i++) {
    float rdx = (x_offset + i - 120) * inv_120;

    // rotate xz by B (inlined for better performance)
    float rdx_rot = rdx * cB - rdz1 * sB;
    float rdz = rdx * sB + rdz1 * cB;
    rdx = rdx_rot;

    int xcheck2 = xcheck & 0x4000 ? 1 : 0;
    // (0, 21, 63) : (42, 42, 42)
    uint16_t color = (xcheck2 ^ ycheck) ? bg0 : bg1;

    // ray marching
    float t = drawdist - r2 - r1 * 1.5;
    float px = rdx * t + rox;
    float py = rdy * t + roy;
    float pz = rdz * t + roz;
    for (int j = 0; j < 20; j++) {
      // Optimized ray marching with reduced function calls
      float lxy = px * px + py * py;
      lxy = sqrtf(lxy);
      float d1 = lxy - r1;
      float ldz = pz * pz + d1 * d1;
      ldz = sqrtf(ldz);
      float d2 = ldz - r2;

      float dt = d2;

      px += rdx * dt;
      py += rdy * dt;
      pz += rdz * dt;

      if (dt > -0.05f && dt < 0.05f) {
        // Optimized normal calculation
        float inv_lxy = 1.0f / lxy;
        float Nx = px * (1.0f - r1 * inv_lxy);
        float Ny = py * (1.0f - r1 * inv_lxy);
        float Nz = pz;
        float Nmag = 1.0f / r2;
        
        // Optimized texture coordinate calculation
        float rxy = fast_atan2f(py, px);
        float rxz = fast_atan2f(pz, d1);
        int lxyi = (int)(rxy * 256.0f / M_PI);
        int lxzi = (int)(rxz * 256.0f / M_PI);
        float check_xy = (lxyi ^ lxzi) & 0x20 ? 1.0f + palette * (1.0f / 256.0f) : 0.0f;
        
        // Optimized lighting calculation
        float l = Nmag * 0.6f * (0.2f + Nx * Lx + Ny * Ly + Nz * Lz + check_xy);
        if (l > 0) {
          if (l > 1.0f) l = 1.0f;
          int k = (int)(l * (NPALETTE - 1));
          int r = (palette1_r[k] * (256 - palette) + palette2_r[k] * palette) >> 8;
          int g = (palette1_g[k] * (256 - palette) + palette2_g[k] * palette) >> 8;
          int b = (palette1_b[k] * (256 - palette) + palette2_b[k] * palette) >> 8;
          color = BADGE_RGB565(r, g, b);
        } else {
          color = 0x0000;
        }
        break;
      }

      t += dt;
      if (t > drawdist + r2 + r1 + 0.5f) {
        break;
      }
    }

    pixels[i] = color;

    xcheck += ooy;
  }
}
