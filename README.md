# Badge Project

A dual-target badge project featuring a circular 240x240 OLED display with GC9A01 driver on the Seeed RP2350, plus a desktop emulator for development and testing.

## Features

- **Embedded Target**: Real badge using Seeed RP2350 with circular OLED display
- **Desktop Emulator**: SDL-based emulator with 4x chunky pixels (960x960 window)
- **Common Rendering**: Shared scanline-based renderer to avoid spurious SPI transfers
- **High Performance**: Raw SPI + DMA for efficient display updates

## Project Structure

```
badge/
├── CMakeLists.txt          # Root build configuration
├── common/                 # Shared rendering code
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── renderer.h
│   └── src/
│       └── renderer.c
├── embedded/               # RP2350 embedded target
│   ├── CMakeLists.txt
│   ├── pico_sdk_import.cmake
│   ├── include/
│   │   ├── gc9a01.h
│   │   └── badge_main.h
│   └── src/
│       ├── gc9a01.c
│       ├── badge_main.c
│       └── main.c
└── desktop/                # SDL desktop emulator
    ├── CMakeLists.txt
    ├── include/
    │   └── desktop_main.h
    └── src/
        ├── desktop_main.c
        └── main.c
```

## Building

### Desktop Emulator

Requirements:
- CMake 3.16+
- SDL2 development libraries
- C compiler (GCC, Clang, or MSVC)

#### macOS:
```bash
# Install SDL2 with Homebrew
brew install sdl2

# Build
mkdir build-desktop && cd build-desktop
cmake .. -DBUILD_DESKTOP=ON -DBUILD_EMBEDDED=OFF
make
./badge_desktop
```

#### Linux:
```bash
# Install SDL2 (Ubuntu/Debian)
sudo apt-get install libsdl2-dev

# Build
mkdir build-desktop && cd build-desktop
cmake .. -DBUILD_DESKTOP=ON -DBUILD_EMBEDDED=OFF
make
./badge_desktop
```

### Embedded Target

Requirements:
- CMake 3.16+
- ARM GCC toolchain
- Raspberry Pi Pico SDK

#### Setup:
```bash
# Set PICO_SDK_PATH environment variable
export PICO_SDK_PATH=/path/to/pico-sdk

# Or let CMake fetch it automatically
export PICO_SDK_FETCH_FROM_GIT=1

# Build
mkdir build-embedded && cd build-embedded
cmake .. -DBUILD_EMBEDDED=ON -DBUILD_DESKTOP=OFF
make
```

This will generate `badge_embedded.uf2` which can be copied to the RP2350 in bootloader mode.

## Hardware Configuration

### Pin Assignments (RP2350)
- **MOSI**: GPIO 0
- **CS**: GPIO 1  
- **DC**: GPIO 2
- **RST**: GPIO 3
- **SCK**: GPIO 6
- **SPI**: spi0 @ 62.5 MHz

### Display
- **Controller**: GC9A01 
- **Resolution**: 240x240 circular OLED
- **Color Format**: RGB565 (16-bit)
- **Interface**: SPI with DMA

## Rendering Architecture

The project uses a scanline-based rendering approach:

1. **Common Renderer**: Generates one scanline at a time with configurable x, y, width
2. **Embedded Path**: Writes scanlines directly to GC9A01 via SPI+DMA  
3. **Desktop Path**: Accumulates scanlines in framebuffer, then displays via SDL

This approach minimizes memory usage and allows for efficient circular clipping in the future.

## Development

### Desktop Emulator Controls
- **ESC**: Exit emulator
- **Close Window**: Exit emulator

### Test Pattern
Both targets currently display the same test pattern with gradients to verify the rendering pipeline.

## Future Enhancements

- [ ] Circular clipping for round display area
- [ ] Real badge content (text, graphics, animations)
- [ ] Button input handling
- [ ] Power management for embedded target
- [ ] Communication interface (USB, Bluetooth, etc.)

## License

[Add your license here] 