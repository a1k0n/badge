# Common rendering library
add_library(badge_common STATIC
    src/renderer.c
)

target_include_directories(badge_common PUBLIC
    include
)

# Common compile definitions
target_compile_definitions(badge_common PUBLIC
    BADGE_DISPLAY_WIDTH=240
    BADGE_DISPLAY_HEIGHT=240
) 