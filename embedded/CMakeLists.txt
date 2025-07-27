# Embedded badge target for RP2350
set(TARGET_NAME badge_embedded)

# Create the executable
add_executable(${TARGET_NAME}
    src/main.c
    src/badge_main.c
    src/gc9a01.c
)

# Set board to rp2350 (Seeed RP2350)
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Link libraries
target_link_libraries(${TARGET_NAME}
    badge_common
    pico_stdlib
    hardware_spi
    hardware_dma
    hardware_gpio
    hardware_pwm
)

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    include
)

# Set the output name to include .elf extension
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME badge_embedded.elf)

# Create map/bin/hex/uf2 file in addition to ELF
pico_add_extra_outputs(${TARGET_NAME})

# Enable/disable stdio
pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

# Define pins for RP2350 with GC9A01
target_compile_definitions(${TARGET_NAME} PRIVATE
    # SPI configuration
    GC9A01_SPI_PORT=spi0
    GC9A01_SPI_BAUDRATE=62500000
    
    # Pin definitions
    GC9A01_PIN_CS=1
    GC9A01_PIN_DC=2
    GC9A01_PIN_RST=3
    GC9A01_PIN_MOSI=0
    GC9A01_PIN_SCK=6
    
    # DMA configuration
    GC9A01_USE_DMA=1
)

# Add flash target - copies UF2 to mounted RP2350 in bootsel mode
add_custom_target(flash
    COMMAND sh -c "if [ -d /Volumes/RP2350 ]; then cp ${TARGET_NAME}.uf2 /Volumes/RP2350/ && echo 'Flashed ${TARGET_NAME}.uf2 to RP2350'; else echo 'ERROR: RP2350 not found in bootsel mode. Connect device and hold BOOTSEL button while plugging in.'; exit 1; fi"
    DEPENDS ${TARGET_NAME}
    COMMENT "Flashing ${TARGET_NAME}.uf2 to RP2350 (requires bootsel mode)"
    VERBATIM
)

# Add flash-auto target that waits for bootsel mode device
add_custom_target(flash-auto
    COMMAND sh -c "echo 'Waiting for RP2350 in bootsel mode...'; while [ ! -d /Volumes/RP2350 ]; do sleep 1; done; cp ${TARGET_NAME}.uf2 /Volumes/RP2350/ && echo 'Flashed ${TARGET_NAME}.uf2 to RP2350'"
    DEPENDS ${TARGET_NAME}
    COMMENT "Waiting for RP2350 bootsel mode and flashing ${TARGET_NAME}.uf2"
    VERBATIM
)

# Add info target to show UF2 file information  
add_custom_target(info
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../_deps/picotool-build/picotool info ${TARGET_NAME}.uf2
    DEPENDS ${TARGET_NAME}
    COMMENT "Showing information about ${TARGET_NAME}.uf2"
    VERBATIM
)

# Add device-info target to show any connected device information
add_custom_target(device-info
    COMMAND sh -c "if [ -d /Volumes/RP2350 ]; then echo 'RP2350 found in bootsel mode at /Volumes/RP2350'; else echo 'No RP2350 found in bootsel mode'; fi"
    COMMENT "Checking for connected RP2350 device"
    VERBATIM
)