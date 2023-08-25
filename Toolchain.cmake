set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_COMPILER_WORKS 1)
add_compile_options(-ffreestanding -mno-red-zone -fshort-wchar)
add_link_options(-nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main)