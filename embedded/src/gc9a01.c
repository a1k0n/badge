#include "gc9a01.h"

#include <string.h>

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/platform.h"
#include "pico/stdlib.h"

static void gc9a01_gpio_init(gc9a01_t *display) {
  const gc9a01_config_t *cfg = &display->config;

  // Initialize SPI pins
  gpio_set_function(cfg->pin_mosi, GPIO_FUNC_SPI);
  gpio_set_function(cfg->pin_sck, GPIO_FUNC_SPI);

  // Initialize control pins
  gpio_init(cfg->pin_cs);
  gpio_set_dir(cfg->pin_cs, GPIO_OUT);
  gpio_put(cfg->pin_cs, 1);  // CS high (idle)

  gpio_init(cfg->pin_dc);
  gpio_set_dir(cfg->pin_dc, GPIO_OUT);
  gpio_put(cfg->pin_dc, 1);  // DC high (data mode)

  gpio_init(cfg->pin_blk);
  gpio_set_dir(cfg->pin_blk, GPIO_OUT);
  gpio_put(cfg->pin_blk, 1);  // backlight on
}

static void gc9a01_spi_init(gc9a01_t *display) {
  spi_init(display->config.spi_port, display->config.baudrate);
  spi_set_format(display->config.spi_port, 8, SPI_CPOL_0, SPI_CPHA_0,
                 SPI_MSB_FIRST);
}

static void gc9a01_dma_init(gc9a01_t *display) {
  if (!display->config.use_dma) return;

  display->config.dma_channel = dma_claim_unused_channel(true);

  // Configure DMA channel for SPI TX
  dma_channel_config c =
      dma_channel_get_default_config(display->config.dma_channel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_dreq(&c, spi_get_dreq(display->config.spi_port, true));

  dma_channel_configure(
      display->config.dma_channel, &c,
      &spi_get_hw(display->config.spi_port)->dr,  // Write to SPI TX FIFO
      NULL,  // Source will be set per transfer
      0,     // Transfer count will be set per transfer
      false  // Don't start yet
  );
}

bool gc9a01_init(gc9a01_t *display, const gc9a01_config_t *config) {
  memcpy(&display->config, config, sizeof(gc9a01_config_t));

  gc9a01_gpio_init(display);
  gc9a01_spi_init(display);

  if (config->use_dma) {
    gc9a01_dma_init(display);
  }

#define writecommand(cmd) gc9a01_write_command(display, cmd)
#define writedata(data) gc9a01_write_data1(display, data)
#define delay(ms) sleep_ms(ms)
#include "gc9a01_init.h"
#undef writecommand
#undef writedata
#undef delay

/*
  // Software reset
  gc9a01_write_command(display, GC9A01_SWRESET);
  sleep_ms(120);

  // Exit sleep mode
  gc9a01_write_command(display, GC9A01_SLPOUT);
  sleep_ms(120);

  // Set color mode to 16-bit (RGB565)
  gc9a01_write_command(display, GC9A01_COLMOD);
  uint8_t colmod_data = 0x05;  // 16 bits/pixel
  gc9a01_write_data(display, &colmod_data, 1);

  // Set display function control
  gc9a01_write_command(display, GC9A01_DFUNCTR);
  uint8_t dfunctr_data[] = {0x00, 0x20, 0x00};
  gc9a01_write_data(display, dfunctr_data, sizeof(dfunctr_data));

  // Turn on display
  gc9a01_display_on(display, true);
  */

  // Set memory access control (MADCTL)
  gc9a01_write_command(display, GC9A01_MADCTL);
  uint8_t madctl_data = 0x60;  // 90 degree clockwise rotation
  gc9a01_write_data(display, &madctl_data, 1);


  display->initialized = true;
  display->dma_busy = false;

  return true;
}

void gc9a01_sleep(gc9a01_t *display, bool sleep) {
  gc9a01_write_command(display, sleep ? GC9A01_SLPIN : GC9A01_SLPOUT);
  sleep_ms(120);
}

void gc9a01_display_on(gc9a01_t *display, bool on) {
  gc9a01_write_command(display, on ? GC9A01_DISPON : GC9A01_DISPOFF);
}

void gc9a01_set_inversion(gc9a01_t *display, bool invert) {
  gc9a01_write_command(display, invert ? GC9A01_INVON : GC9A01_INVOFF);
}

void gc9a01_write_command(gc9a01_t *display, uint8_t cmd) {
  gpio_put(display->config.pin_cs, 0);
  gpio_put(display->config.pin_dc, 0);  // Command mode
  spi_write_blocking(display->config.spi_port, &cmd, 1);
  gpio_put(display->config.pin_cs, 1);
}

void gc9a01_write_data(gc9a01_t *display, const uint8_t *data, size_t len) {
  if (len == 0) return;

  gpio_put(display->config.pin_cs, 0);
  gpio_put(display->config.pin_dc, 1);  // Data mode
  spi_write_blocking(display->config.spi_port, data, len);
  gpio_put(display->config.pin_cs, 1);
}

void gc9a01_write_data1(gc9a01_t *display, uint8_t data) {
  gpio_put(display->config.pin_cs, 0);
  gpio_put(display->config.pin_dc, 1);  // Data mode
  spi_write_blocking(display->config.spi_port, &data, 1);
  gpio_put(display->config.pin_cs, 1);
}

void gc9a01_write_data_dma(gc9a01_t *display, const uint8_t *data, size_t len) {
  if (!display->config.use_dma || len == 0) {
    gc9a01_write_data(display, data, len);
    return;
  }

  // CRITICAL: Wait for any previous DMA transfer to complete first
  // This ensures CS pin state is correct and SPI bus is available
  gc9a01_dma_wait(display);

  // CRITICAL: Ensure SPI bus is completely idle before starting new transfer
  // This prevents corruption of window commands that might be pending
  while (spi_is_busy(display->config.spi_port)) {
    tight_loop_contents();
  }

  gpio_put(display->config.pin_cs, 0);
  gpio_put(display->config.pin_dc, 1);  // Data mode

  // Configure and start DMA transfer
  dma_channel_set_read_addr(display->config.dma_channel, data, false);
  dma_channel_set_trans_count(display->config.dma_channel, len, true);

  display->dma_busy = true;
  // Note: CS will be released when DMA completes (in gc9a01_dma_wait or
  // gc9a01_dma_is_busy)
}

void gc9a01_set_window(gc9a01_t *display, uint16_t x0, uint16_t y0, uint16_t x1,
                       uint16_t y1) {
  // Set column address
  gc9a01_write_command(display, GC9A01_CASET);
  uint8_t caset_data[] = {(x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF,
                          x1 & 0xFF};
  gc9a01_write_data(display, caset_data, sizeof(caset_data));

  // Set row address
  gc9a01_write_command(display, GC9A01_RASET);
  uint8_t raset_data[] = {(y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF,
                          y1 & 0xFF};
  gc9a01_write_data(display, raset_data, sizeof(raset_data));

  // Memory write command
  gc9a01_write_command(display, GC9A01_RAMWR);
}

void gc9a01_write_scanline(gc9a01_t *display, const badge_color_t *pixels,
                           uint16_t x_offset, uint16_t y, uint16_t width) {
  if (!display->initialized || width == 0) return;

  // Set window for this scanline
  gc9a01_set_window(display, x_offset, y, x_offset + width - 1, y);

  // Write pixel data (RGB565 is already in the correct format)
  gc9a01_write_data(display, (const uint8_t *)pixels, width * 2);
}

void gc9a01_write_scanline_dma(gc9a01_t *display, const badge_color_t *pixels,
                               uint16_t x_offset, uint16_t y, uint16_t width) {
  if (!display->initialized || width == 0) return;

  // CRITICAL: Wait for any previous DMA transfer to complete FIRST
  // This ensures CS pin state is correct and SPI bus is available
  gc9a01_dma_wait(display);

  // CRITICAL: Ensure SPI bus is completely idle before setting window
  // This prevents corruption of window commands
  while (spi_is_busy(display->config.spi_port)) {
    tight_loop_contents();
  }

  // Now safe to set window with blocking SPI commands
  gc9a01_set_window(display, x_offset, y, x_offset + width - 1, y);

  // CRITICAL: Ensure window commands have completed before starting DMA
  while (spi_is_busy(display->config.spi_port)) {
    tight_loop_contents();
  }

  // Start DMA transfer of pixel data (non-blocking)
  // This will run in background while CPU renders next scanline
  gc9a01_write_data_dma(display, (const uint8_t *)pixels, width * 2);
}

bool gc9a01_dma_is_busy(gc9a01_t *display) {
  if (!display->config.use_dma) return false;

  bool busy = dma_channel_is_busy(display->config.dma_channel);
  if (!busy && display->dma_busy) {
    // CRITICAL: Wait for SPI to finish before releasing CS
    // This ensures all data has been transmitted
    while (spi_is_busy(display->config.spi_port)) {
      tight_loop_contents();
    }
    
    // DMA transfer completed, clean up CS pin state
    gpio_put(display->config.pin_cs, 1);
    display->dma_busy = false;
  }
  return display->dma_busy;
}

void gc9a01_dma_wait(gc9a01_t *display) {
  while (gc9a01_dma_is_busy(display)) {
    tight_loop_contents();
  }
}

void gc9a01_fill_screen(gc9a01_t *display, uint16_t color) {
  gc9a01_set_window(display, 0, 0, GC9A01_WIDTH - 1, GC9A01_HEIGHT - 1);

  // Create a buffer with the fill color
  static uint16_t fill_buffer[GC9A01_WIDTH];
  for (int i = 0; i < GC9A01_WIDTH; i++) {
    fill_buffer[i] = color;
  }

  // Write each row
  for (int y = 0; y < GC9A01_HEIGHT; y++) {
    gc9a01_write_data(display, (const uint8_t *)fill_buffer,
                      sizeof(fill_buffer));
  }
}