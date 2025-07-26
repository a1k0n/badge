#include "badge_main.h"
#include "pico/stdlib.h"
#include <stdio.h>

bool badge_init(badge_context_t *ctx) {
    // Initialize stdio for debugging
    stdio_init_all();
    
    printf("Initializing badge...\n");
    
    // Configure GC9A01 display
    gc9a01_config_t display_config = {
        .spi_port = spi0,
        .baudrate = 62500000, // 62.5 MHz
        .pin_cs = 1,
        .pin_dc = 2,
        .pin_rst = 3,
        .pin_mosi = 0,
        .pin_sck = 6,
        .use_dma = true,
        .dma_channel = -1 // Will be assigned by driver
    };
    
    // Initialize display
    if (!gc9a01_init(&ctx->display, &display_config)) {
        printf("Failed to initialize GC9A01 display\n");
        return false;
    }
    
    // Initialize renderer (no framebuffer needed for embedded)
    badge_renderer_init(&ctx->renderer, NULL);
    
    // Initialize application state
    ctx->frame_count = 0;
    ctx->running = true;
    
    printf("Badge initialized successfully\n");
    return true;
}

void badge_run(badge_context_t *ctx) {
    printf("Starting badge main loop\n");
    
    while (ctx->running) {
        badge_render_frame(ctx);
        ctx->frame_count++;
        
        // Simple frame rate control - approximately 30 FPS
        sleep_ms(33);
        
        // Print frame count periodically
        if (ctx->frame_count % 300 == 0) { // Every 10 seconds at 30 FPS
            printf("Frame count: %lu\n", ctx->frame_count);
        }
    }
}

void badge_shutdown(badge_context_t *ctx) {
    printf("Shutting down badge\n");
    ctx->running = false;
    
    // Turn off display
    gc9a01_display_on(&ctx->display, false);
    gc9a01_sleep(&ctx->display, true);
}

void badge_render_frame(badge_context_t *ctx) {
    // For now, render a test pattern
    badge_render_test_pattern(ctx);
}

void badge_render_test_pattern(badge_context_t *ctx) {
    // Render the display scanline by scanline
    for (uint16_t y = 0; y < BADGE_DISPLAY_HEIGHT; y++) {
        // Generate test pattern for this scanline
        badge_test_pattern_scanline(&ctx->scanline_buffer, y);
        
        // Render full width scanline
        badge_render_scanline(&ctx->renderer, &ctx->scanline_buffer, 0, y, BADGE_DISPLAY_WIDTH);
        
        // Write scanline to display using DMA for efficiency
        gc9a01_write_scanline_dma(&ctx->display, &ctx->scanline_buffer);
        
        // For very fast updates, we might want to wait for DMA completion
        // but for a test pattern at 30 FPS, the next scanline setup time
        // should be enough for DMA to complete
    }
    
    // Wait for final DMA transfer to complete
    gc9a01_dma_wait(&ctx->display);
} 