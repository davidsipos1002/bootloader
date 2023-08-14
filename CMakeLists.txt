cmake_minimum_required(VERSION 3.27)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Toolchain.cmake)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/)

project(Bootloader VERSION 0.1 LANGUAGES C)

set(includes include/UEFI/ include/UEFI/Uefi/ include/UEFI/X64/ include/)
file(GLOB_RECURSE sources src/*.c)

add_executable(BOOTX64.EFI ${sources})
target_include_directories(BOOTX64.EFI PUBLIC ${includes})

install(TARGETS BOOTX64.EFI DESTINATION build/efi)