cmake_minimum_required(VERSION 3.27)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/Toolchain.cmake)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/)

project(Bootloader VERSION 0.3 LANGUAGES ASM C)

set(includes include gnu-efi/inc gnu-efi/inc/x86_64 gnu-efi/inc/protocol ${CMAKE_BINARY_DIR}/include)
file(GLOB_RECURSE sources src/*.c src/kerneljump.s)
list(APPEND sources gnu-efi/lib/data.c)

option(basic_logging "Basic logging" ON)
option(extensive_logging "Detailed logging" OFF)
option(print_elf "Print elf headers" OFF)
option(print_memory_map "Print memory map from first call" OFF)
option(print_tokens "Print config file tokens" OFF)
option(print_config "Print boot config" ON)

add_executable(BOOTX64.EFI ${sources})
configure_file(${CMAKE_SOURCE_DIR}/include/bootloader/version.h.in ${CMAKE_BINARY_DIR}/include/bootloader/version.h)
target_include_directories(BOOTX64.EFI PRIVATE ${includes})

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(BOOTX64.EFI PRIVATE DEBUG_BUILD)
endif()

if(basic_logging)
    target_compile_definitions(BOOTX64.EFI PRIVATE BASIC_LOGGING)
endif()

if(extensive_logging)
    target_compile_definitions(BOOTX64.EFI PRIVATE EXTENSIVE_LOGGING)
endif()

if(print_elf)
    target_compile_definitions(BOOTX64.EFI PRIVATE PRINT_ELF)
endif()

if(print_memory_map)
    target_compile_definitions(BOOTX64.EFI PRIVATE PRINT_MEMORY_MAP)
endif()

if(print_tokens)
    target_compile_definitions(BOOTX64.EFI PRIVATE PRINT_TOKENS)
endif()

if(print_config)
    target_compile_definitions(BOOTX64.EFI PRIVATE PRINT_CONFIG)
endif()

install(TARGETS BOOTX64.EFI DESTINATION build/efi)