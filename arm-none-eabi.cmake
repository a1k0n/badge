# Toolchain file for ARM Cortex-M cross compilation

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Specify the cross compiler
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# ARM Cortex-M33 flags for RP2350 with hardware FPU and optimizations
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -march=armv8-m.main+dsp+fp -O3 -ffast-math -funroll-loops -ftree-vectorize -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -march=armv8-m.main+dsp+fp -O3 -ffast-math -funroll-loops -ftree-vectorize -fomit-frame-pointer")
set(CMAKE_ASM_FLAGS_INIT "-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -march=armv8-m.main+dsp+fp")

# Where to find the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Don't try to link for this system
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Disable macOS-specific settings
set(CMAKE_OSX_DEPLOYMENT_TARGET "")
set(CMAKE_OSX_SYSROOT "") 
