#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop_main.h"

int main(int argc, char *argv[]) {
  desktop_context_t desktop_ctx;
  int frames_to_dump = 0;

  printf("Badge Desktop Emulator\n");
  printf("======================\n");
  printf("Controls:\n");
  printf("  ESC - Exit\n");
  printf("  Close window - Exit\n\n");

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--dump-frames") == 0 && i + 1 < argc) {
      frames_to_dump = atoi(argv[i + 1]);
      printf("Will dump first %d frames to frame_*.ppm files\n", frames_to_dump);
      i++; // Skip the next argument
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [--dump-frames N]\n", argv[0]);
      printf("  --dump-frames N  Dump first N frames to frame_*.ppm files\n");
      printf("  --help, -h       Show this help message\n");
      return 0;
    }
  }

  // Initialize the desktop emulator
  if (!desktop_init(&desktop_ctx)) {
    printf("Desktop emulator initialization failed!\n");
    return -1;
  }

  // Set frame dumping if requested
  if (frames_to_dump > 0) {
    desktop_ctx.frames_to_dump = frames_to_dump;
    desktop_ctx.dump_frame_count = 0;
  }

  // Run the main emulator loop
  desktop_run(&desktop_ctx);

  // Clean shutdown
  desktop_shutdown(&desktop_ctx);

  printf("Desktop emulator exited cleanly\n");
  return 0;
}