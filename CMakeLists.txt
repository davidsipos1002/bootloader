cmake_minimum_required(VERSION 3.27)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Toolchain.cmake)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/)

project(Bootloader VERSION 0.1 LANGUAGES ASM C)

set(includes include/ gnu-efi/inc gnu-efi/inc/x86_64 gnu-efi/inc/protocol)
file(GLOB_RECURSE sources src/*.c src/kerneljump.s)
list(APPEND sources gnu-efi/lib/data.c)

add_executable(BOOTX64.EFI ${sources})
target_include_directories(BOOTX64.EFI PUBLIC ${includes})

install(TARGETS BOOTX64.EFI DESTINATION build/efi)