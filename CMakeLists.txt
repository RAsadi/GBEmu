cmake_minimum_required(VERSION 3.16)
project(GBEmu VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

# Adding SDL as our graphic engine
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add all our roms
FILE(GLOB roms "roms/*.gb")
FILE(COPY ${ROMS} DESTINATION "${CMAKE_BINARY_DIR}")

# Add logging
execute_process(COMMAND git submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}}/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include)

add_executable(GBEmu src/main.cpp
        src/cpu/cpu.cpp src/cpu/cpu.h src/cpu/opcodes.cpp src/cpu/opcode_utils.cpp
        src/memory/mmu.cpp src/memory/mmu.h
        src/gpu.cpp src/gpu.h
        src/timer.cpp src/timer.h
        src/memory/cartridge.cpp src/memory/cartridge.h
        src/interrupt_handler.cpp src/interrupt_handler.h
        src/joypad.cpp src/joypad.h
        src/utils/bit_utils.h src/utils/enums.h src/utils/constants.h
        src/utils/bit_utils.cpp)

target_link_libraries(GBEmu ${SDL2_LIBRARIES})