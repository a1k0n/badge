#include <stdio.h>

#include "desktop_main.h"

int main(int argc, char *argv[]) {
  desktop_context_t desktop_ctx;

  printf("Badge Desktop Emulator\n");
  printf("======================\n");
  printf("Controls:\n");
  printf("  ESC - Exit\n");
  printf("  Close window - Exit\n\n");

  // Initialize the desktop emulator
  if (!desktop_init(&desktop_ctx)) {
    printf("Desktop emulator initialization failed!\n");
    return -1;
  }

  // Run the main emulator loop
  desktop_run(&desktop_ctx);

  // Clean shutdown
  desktop_shutdown(&desktop_ctx);

  printf("Desktop emulator exited cleanly\n");
  return 0;
}