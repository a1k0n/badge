#include <stdio.h>

#include "badge_main.h"
#include "pico/stdlib.h"

int main() {
  badge_context_t badge_ctx;

  // Initialize the badge
  if (!badge_init(&badge_ctx)) {
    printf("Badge initialization failed!\n");
    return -1;
  }

  // Run the main badge loop
  badge_run(&badge_ctx);

  // Clean shutdown (though we may never reach this in embedded)
  badge_shutdown(&badge_ctx);

  return 0;
}