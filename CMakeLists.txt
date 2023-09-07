cmake_minimum_required(VERSION 3.27)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/Toolchain.cmake)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/)

project(Bootloader VERSION 0.1 LANGUAGES ASM C)

set(includes include gnu-efi/inc gnu-efi/inc/x86_64 gnu-efi/inc/protocol ${CMAKE_BINARY_DIR}/include)
file(GLOB_RECURSE sources src/*.c src/kerneljump.s)
list(APPEND sources gnu-efi/lib/data.c)

set(BootInfo_Address 0xFFFFFF7FFFFFF000)

option(basic_logging "Basic logging" ON)
option(extensive_logging "Detailed logging")
option(print_elf "Print elf headers")
option(print_memory_map "Print memory map from first call")

add_executable(BOOTX64.EFI ${sources})
configure_file(${CMAKE_SOURCE_DIR}/include/bootloader/bootloadercfg.h.in ${CMAKE_BINARY_DIR}/include/bootloader/bootloadercfg.h)
target_include_directories(BOOTX64.EFI PRIVATE ${includes})

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

install(TARGETS BOOTX64.EFI DESTINATION build/efi)