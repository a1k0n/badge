# Desktop emulator target
set(TARGET_NAME badge_desktop)

# Find SDL2
find_package(SDL2 REQUIRED)

# Create the executable
add_executable(${TARGET_NAME}
    src/main.c
    src/desktop_main.c
)

# Link libraries
target_link_libraries(${TARGET_NAME}
    badge_common
    ${SDL2_LIBRARIES}
)

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
)

# Desktop-specific compile definitions
target_compile_definitions(${TARGET_NAME} PRIVATE
    DESKTOP_SCALE_FACTOR=4
    DESKTOP_WINDOW_WIDTH=960   # 240 * 4
    DESKTOP_WINDOW_HEIGHT=960  # 240 * 4
)

# Compiler flags for SDL2
target_compile_options(${TARGET_NAME} PRIVATE ${SDL2_CFLAGS_OTHER}) 