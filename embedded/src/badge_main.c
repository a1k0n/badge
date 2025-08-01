#include "badge_main.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"


/*
void benchmark_fpu() {
  uint64_t start = time_us_64();
  float total = 0;
  for (int i = 0; i < 100000; i++) {
    float x = i;
    float y = i * 2;
    float z = i * 3;
    float result = x * x + y * y + z * z;
    total += result;
  }
  uint64_t end = time_us_64();
  printf("100k vector3 norm: %llu us, total: %f; %d ns/op\n", end - start, total, (end - start) / 100);

  start = time_us_64();
  int totalint = 0;
  for (int i = 0; i < 100000; i++) {
    int x = i;
    int y = i * 2;
    int z = i * 3;
    int result = x * x + y * y + z * z;
    totalint += result;
  }
  end = time_us_64();
  printf("100k int3 norm: %llu us, total: %d; %d ns/op\n", end - start, totalint, (end - start) / 100);
}
*/

static inline void enable_flush_to_zero(void) {
  uint32_t fpscr = __builtin_arm_get_fpscr();
  fpscr |= (1u << 24);  // bit-24 = FZ  → flush subnormals to zero
  __builtin_arm_set_fpscr(fpscr);
  asm volatile ("dsb 0xF":::"memory");
  asm volatile ("isb 0xF":::"memory");
}

bool badge_init(badge_context_t *ctx) {
  // Initialize stdio for debugging
  stdio_init_all();
  /*
  while (!stdio_usb_connected()) {
    sleep_ms(100);
  }
  */

  printf("Initializing badge...\n");

  enable_flush_to_zero();

  //benchmark_fpu();

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

  // Initialize core synchronization
  ctx->cores_running = false;

  // Initialize application state
  ctx->frame_count = 0;
  ctx->running = true;
  ctx->use_buffer_a = true;  // Start with buffer A

  printf("Badge initialized successfully\n");
  printf("Streaming dual-core DMA rendering enabled\n");
  return true;
}

// Core 1 worker function for rendering
void core1_worker(void) {
  // Get the context from core 0
  badge_context_t *ctx = (badge_context_t *)multicore_fifo_pop_blocking();
  
  printf("Core 1 worker started\n");
  
  // Core 1 will render scanlines on demand
  uint16_t cur_y = ~0;
  while (ctx->cores_running) {
    //printf("Core 1 waiting for scanline request...\n");
    // Wait for scanline request from core 0
    uint32_t scanline_request = multicore_fifo_pop_blocking();
    uint16_t y = (uint16_t)(scanline_request);
    uint16_t x_offset = BADGE_MASK_X_OFFSET(y);
    uint16_t width = BADGE_MASK_X_WIDTH(y);
    
    //printf("Core 1 rendering scanline %d\n", y);
    // Render the scanline
    if (y != cur_y) {
      cur_y = y;
      badge_render_scanline(&ctx->renderer, ctx->scanline_buffer_b, x_offset, y, width);
    }
    gc9a01_dma_wait(&ctx->display);
    gc9a01_write_scanline_dma(&ctx->display, ctx->scanline_buffer_b, x_offset, y, width);
    
    //printf("Core 1 completed scanline %d\n", y);
    // Signal completion
    multicore_fifo_push_blocking(scanline_request);

    // pre-render expected next scanline
    if (y <= 237) {
      y += 2;
      x_offset = BADGE_MASK_X_OFFSET(y);
      width = BADGE_MASK_X_WIDTH(y);
      badge_render_scanline(&ctx->renderer, ctx->scanline_buffer_b, x_offset, y, width);
      cur_y = y;
    }
  }
}

bool badge_start_render_threads(badge_context_t *ctx) {
  // Launch core 1 with rendering worker
  multicore_launch_core1(core1_worker);
  ctx->cores_running = true;
  multicore_fifo_push_blocking((uint32_t)ctx);
  
  printf("Streaming dual-core rendering started\n");
  return true;
}

void badge_stop_render_threads(badge_context_t *ctx) {
  ctx->cores_running = false;
  
  // Wait for core 1 to finish
  multicore_reset_core1();
  
  printf("Streaming dual-core rendering stopped\n");
}

void badge_run(badge_context_t *ctx) {
  printf("Starting badge main loop\n");

  // Start dual-core rendering
  badge_start_render_threads(ctx);

  uint64_t start_time = time_us_64();
  while (ctx->running) {
    // Advance frame for animations
    badge_advance_frame(&ctx->renderer);

    // Use threaded rendering
    badge_render_frame_threaded(ctx);
    ctx->frame_count++;

    // Simple frame rate control - approximately 30 FPS
    // sleep_ms(33);

    // Print frame count periodically
    if (ctx->frame_count % 100 == 0) {  // Every 100 frames
      uint64_t end_time = time_us_64();
      uint64_t duration = end_time - start_time;
      printf("Frame: %lu, Renderer frame: %lu, %0.2f fps\n", ctx->frame_count,
             ctx->renderer.frame_count, 100000000.0f / duration);
      start_time = end_time;
    }
  }
  
  // Stop threads before shutdown
  badge_stop_render_threads(ctx);
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

void badge_render_frame_threaded(badge_context_t *ctx) {
  //printf("Starting frame render...\n");
  
  for (uint16_t y = 0; y < BADGE_DISPLAY_HEIGHT; y+=2) {
    uint16_t x_offset_a = BADGE_MASK_X_OFFSET(y+1);
    uint16_t width_a = BADGE_MASK_X_WIDTH(y+1);
    uint16_t x_offset_b = BADGE_MASK_X_OFFSET(y);
    uint16_t width_b = BADGE_MASK_X_WIDTH(y);
    
    //printf("Processing scanline %d,%d\n", y, y+1);
    multicore_fifo_push_blocking(y); // other core will render scanline y into buffer b
    // meanwhile we render scanline y+1 into buffer a
    badge_render_scanline(&ctx->renderer, ctx->scanline_buffer_a, x_offset_a, y+1, width_a);
    multicore_fifo_pop_blocking();
    gc9a01_dma_wait(&ctx->display);
    gc9a01_write_scanline_dma(&ctx->display, ctx->scanline_buffer_b, 
                                x_offset_a, y+1, width_a);
  }
  //printf("Frame render completed\n");
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