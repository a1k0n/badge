#include "badge_main.h"

#include <stdio.h>

#include "pico/stdlib.h"

bool badge_init(badge_context_t *ctx) {
  // Initialize stdio for debugging
  stdio_init_all();

  printf("Initializing badge...\n");

  // Configure GC9A01 display
  gc9a01_config_t display_config = {
      .spi_port = spi0,
      .baudrate = 62500000,  // 62.5 MHz
      .pin_cs = 27,
      .pin_sck = 2,
      .pin_mosi = 3,
      .pin_dc = 5,
      .pin_blk = 0,
      .use_dma = 1,
      .dma_channel = -1  // Will be assigned by driver
  };

  // Initialize display
  if (!gc9a01_init(&ctx->display, &display_config)) {
    printf("Failed to initialize GC9A01 display\n");
    return false;
  }

  // Initialize renderer (no framebuffer needed for embedded)
  badge_renderer_init(&ctx->renderer);

  // Initialize application state
  //ctx->frame_count = 0;
  ctx->running = true;
  ctx->use_buffer_a = true;  // Start with buffer A

  printf("Badge initialized successfully\n");
  printf("Double buffered DMA rendering enabled\n");
  return true;
}

void badge_run(badge_context_t *ctx) {
  printf("Starting badge main loop\n");

  while (ctx->running) {
    // Advance frame for animations
    badge_advance_frame(&ctx->renderer);

    badge_render_frame(ctx);
    //ctx->frame_count++;

    // Simple frame rate control - approximately 30 FPS
    // sleep_ms(33);

    // Print frame count periodically
    //if (ctx->frame_count % 10 == 0) {  // Every 10 frames
    //  printf("Frame: %lu, Renderer frame: %lu\n", ctx->frame_count,
    //         ctx->renderer.frame_count);
    //}
  }
}

void badge_shutdown(badge_context_t *ctx) {
  printf("Shutting down badge\n");
  ctx->running = false;

  // Wait for any pending DMA transfer to complete
  gc9a01_dma_wait(&ctx->display);

  // Turn off display
  gc9a01_display_on(&ctx->display, false);
  gc9a01_sleep(&ctx->display, true);
}

void badge_render_frame(badge_context_t *ctx) {
  // Racing-the-beam rendering with double buffered DMA
  for (uint16_t y = 0; y < BADGE_DISPLAY_HEIGHT; y++) {
    // Select current buffer (ping-pong)
    badge_color_t *current_buffer =
        ctx->use_buffer_a ? ctx->scanline_buffer_a : ctx->scanline_buffer_b;

    //printf("Rendering scanline %d\n", y);
    // Generate scanline pixels using racing-the-beam renderer
    uint16_t x_offset = BADGE_MASK_X_OFFSET(y);
    uint16_t width = BADGE_MASK_X_WIDTH(y);
    badge_render_scanline(&ctx->renderer, current_buffer, x_offset, y, width);

    //printf("Writing scanline %d\n", y);

    // Write scanline to display using DMA with proper synchronization
    // This will wait for previous DMA, set window, then start new DMA
    gc9a01_write_scanline_dma(&ctx->display, current_buffer, x_offset, y, width);

    // Switch to other buffer for next scanline while DMA writes current one
    ctx->use_buffer_a = !ctx->use_buffer_a;

    // CPU can now render next scanline while DMA transfers current one
  }

  // Wait for final DMA transfer to complete before starting next frame
  gc9a01_dma_wait(&ctx->display);
}