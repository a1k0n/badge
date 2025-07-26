#ifndef GC9A01_H
#define GC9A01_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "renderer.h"

// GC9A01 Commands
#define GC9A01_SWRESET     0x01
#define GC9A01_SLPIN       0x10
#define GC9A01_SLPOUT      0x11
#define GC9A01_INVOFF      0x20
#define GC9A01_INVON       0x21
#define GC9A01_DISPOFF     0x28
#define GC9A01_DISPON      0x29
#define GC9A01_CASET       0x2A
#define GC9A01_RASET       0x2B
#define GC9A01_RAMWR       0x2C
#define GC9A01_COLMOD      0x3A
#define GC9A01_MADCTL      0x36
#define GC9A01_DFUNCTR     0xB6

// Display configuration
#define GC9A01_WIDTH       240
#define GC9A01_HEIGHT      240

// Hardware configuration structure
typedef struct {
    spi_inst_t *spi_port;
    uint32_t baudrate;
    uint8_t pin_cs;
    uint8_t pin_dc;
    uint8_t pin_rst;
    uint8_t pin_mosi;
    uint8_t pin_sck;
    bool use_dma;
    int dma_channel;
} gc9a01_config_t;

// Driver state
typedef struct {
    gc9a01_config_t config;
    bool initialized;
    bool dma_busy;
} gc9a01_t;

// Initialize the GC9A01 display
bool gc9a01_init(gc9a01_t *display, const gc9a01_config_t *config);

// Basic display control
void gc9a01_reset(gc9a01_t *display);
void gc9a01_sleep(gc9a01_t *display, bool sleep);
void gc9a01_display_on(gc9a01_t *display, bool on);
void gc9a01_set_inversion(gc9a01_t *display, bool invert);

// Low-level communication
void gc9a01_write_command(gc9a01_t *display, uint8_t cmd);
void gc9a01_write_data(gc9a01_t *display, const uint8_t *data, size_t len);
void gc9a01_write_data_dma(gc9a01_t *display, const uint8_t *data, size_t len);

// Window/region setting
void gc9a01_set_window(gc9a01_t *display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// Racing-the-beam scanline rendering functions
void gc9a01_write_scanline(gc9a01_t *display, const badge_color_t *pixels, 
                          uint16_t x_offset, uint16_t y, uint16_t width);
void gc9a01_write_scanline_dma(gc9a01_t *display, const badge_color_t *pixels, 
                              uint16_t x_offset, uint16_t y, uint16_t width);

// DMA management
bool gc9a01_dma_is_busy(gc9a01_t *display);
void gc9a01_dma_wait(gc9a01_t *display);

// Helper functions
void gc9a01_fill_screen(gc9a01_t *display, uint16_t color);

#endif // GC9A01_H 