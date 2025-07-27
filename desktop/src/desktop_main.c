#include "desktop_main.h"
#include <stdio.h>
#include <stdlib.h>

bool desktop_init(desktop_context_t *ctx) {
    printf("Initializing desktop badge emulator...\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return false;
    }
    
    // Create window
    ctx->window = SDL_CreateWindow(
        "Badge Emulator - Racing the Beam",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DESKTOP_WINDOW_WIDTH,
        DESKTOP_WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!ctx->window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    // Create renderer
    ctx->sdl_renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx->sdl_renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return false;
    }
    
    // Create texture for the display
    ctx->texture = SDL_CreateTexture(
        ctx->sdl_renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        BADGE_DISPLAY_WIDTH,
        BADGE_DISPLAY_HEIGHT
    );
    
    if (!ctx->texture) {
        printf("Texture creation failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->sdl_renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return false;
    }
    
    // Initialize badge renderer
    badge_renderer_init(&ctx->renderer);
    
    // Initialize application state
    ctx->frame_count = 0;
    ctx->running = true;
    ctx->last_frame_time = SDL_GetTicks();
    ctx->target_frame_time = 33; // ~30 FPS
    
    printf("Desktop emulator initialized successfully\n");
    printf("Window size: %dx%d (scale factor: %d)\n", 
           DESKTOP_WINDOW_WIDTH, DESKTOP_WINDOW_HEIGHT, DESKTOP_SCALE_FACTOR);
    printf("Racing-the-beam renderer with animated scrolling test pattern\n");
    
    return true;
}

void desktop_run(desktop_context_t *ctx) {
    printf("Starting desktop emulator main loop\n");
    
    while (ctx->running) {
        uint32_t frame_start = SDL_GetTicks();
        
        // Handle SDL events
        desktop_handle_events(ctx);
        
        if (!ctx->running) break;
        
        // Advance frame for animations
        badge_advance_frame(&ctx->renderer);
        
        // Update display
        desktop_update_display(ctx);
        
        ctx->frame_count++;
        
        // Frame rate control
        uint32_t frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < ctx->target_frame_time) {
            SDL_Delay(ctx->target_frame_time - frame_time);
        }
        
        // Print performance info periodically
        if (ctx->frame_count % 300 == 0) { // Every 10 seconds at 30 FPS
            uint32_t current_time = SDL_GetTicks();
            float fps = 300.0f / ((current_time - ctx->last_frame_time) / 1000.0f);
            printf("Frame: %u, Renderer frame: %u, FPS: %.1f\n", 
                   ctx->frame_count, ctx->renderer.frame_count, fps);
            ctx->last_frame_time = current_time;
        }
    }
}

void desktop_shutdown(desktop_context_t *ctx) {
    printf("Shutting down desktop emulator\n");
    ctx->running = false;
    
    if (ctx->texture) {
        SDL_DestroyTexture(ctx->texture);
    }
    
    if (ctx->sdl_renderer) {
        SDL_DestroyRenderer(ctx->sdl_renderer);
    }
    
    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
    }
    
    SDL_Quit();
}

void desktop_handle_events(desktop_context_t *ctx) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                ctx->running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        ctx->running = false;
                        break;
                }
                break;
        }
    }
}

void desktop_update_display(desktop_context_t *ctx) {
    void *pixels;
    int pitch;
    
    // Lock texture for writing
    if (SDL_LockTexture(ctx->texture, NULL, &pixels, &pitch) != 0) {
        printf("Failed to lock texture: %s\n", SDL_GetError());
        return;
    }
    
    // Convert RGB565 framebuffer to RGB888 texture
    uint32_t *texture_pixels = (uint32_t*)pixels;

    badge_color_t scanline_buffer[BADGE_DISPLAY_WIDTH];
    
    for (int y = 0; y < BADGE_DISPLAY_HEIGHT; y++) {
        memset(scanline_buffer, 0, BADGE_DISPLAY_WIDTH * sizeof(badge_color_t));
        uint16_t x_offset = BADGE_MASK_X_OFFSET(y);
        uint16_t width = BADGE_MASK_X_WIDTH(y);
        badge_render_scanline(&ctx->renderer, scanline_buffer, x_offset, y, width);
        
        memset(texture_pixels + y * (pitch / 4), 0, BADGE_DISPLAY_WIDTH * sizeof(uint32_t));
        for (int x = 0; x < width; x++) {
            badge_color_t rgb565 = scanline_buffer[x];
            uint32_t rgb888 = desktop_rgb565_to_rgb888(rgb565);
            texture_pixels[y * (pitch / 4) + x + x_offset] = rgb888;
        }
    }
    
    SDL_UnlockTexture(ctx->texture);
    
    // Clear renderer
    SDL_SetRenderDrawColor(ctx->sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->sdl_renderer);
    
    // Render the texture scaled up
    SDL_RenderCopy(ctx->sdl_renderer, ctx->texture, NULL, NULL);
    
    // Present
    SDL_RenderPresent(ctx->sdl_renderer);
}

uint32_t desktop_rgb565_to_rgb888(badge_color_t color) {
    // Extract RGB components from RGB565
    uint8_t r5 = (color >> 11) & 0x1F;
    uint8_t g6 = (color >> 5) & 0x3F;
    uint8_t b5 = color & 0x1F;
    
    // Convert to 8-bit components
    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g6 << 2) | (g6 >> 4);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);
    
    // Pack into RGB888 (SDL uses ARGB8888 format)
    return 0xFF000000 | (r8 << 16) | (g8 << 8) | b8;
} 