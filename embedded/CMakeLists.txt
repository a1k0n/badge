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
    GC9A01_PIN_DC=3
    GC9A01_PIN_MOSI=10
    GC9A01_PIN_SCK=8
    GC9A01_PIN_BLK=6
    
    # DMA configuration
    GC9A01_USE_DMA=1
)

# Add flash target using picotool
add_custom_target(flash
    COMMAND picotool load ${TARGET_NAME}.elf.uf2 -f
    DEPENDS ${TARGET_NAME}
    COMMENT "Flashing ${TARGET_NAME}.elf.uf2 to RP2350 using picotool"
    VERBATIM
)

# Add flash-reboot target that flashes and reboots the device
add_custom_target(flash-reboot
    COMMAND picotool load ${TARGET_NAME}.elf.uf2 && picotool reboot
    DEPENDS ${TARGET_NAME}
    COMMENT "Flashing ${TARGET_NAME}.elf.uf2 and rebooting RP2350"
    VERBATIM
)

# Add info target to show UF2 file information  
add_custom_target(info
    COMMAND picotool info ${TARGET_NAME}.elf.uf2
    DEPENDS ${TARGET_NAME}
    COMMENT "Showing information about ${TARGET_NAME}.elf.uf2"
    VERBATIM
)

# Add device-info target to show any connected device information
add_custom_target(device-info
    COMMAND picotool info
    COMMENT "Showing information about connected RP2350 device"
    VERBATIM
)