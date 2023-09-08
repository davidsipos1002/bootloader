# Bootloader

Simple 64-bit UEFI bootloader for the x86_64 architecture.
Kernel is searched according to the Kernel_Path CMake variable
relative to the root directory of the volume from which the bootloader is started.
Kernel must be a 64-bit ELF executable. Program segments are loaded at the virtual
addresses specified in the program header table. Upon successful boot the processor
jumps to the kernel's entry point and loads the BootInfo structure's virtual address,
configurable by the BootInfo_Address CMake variable, in register %rdi (first function 
parameter according to the System V ABI). 4-level recursive paging is also set up by the bootloader.

BootInfo structure:
1. efi_system_table physical address
2. FrameBuffer structure
   * width
   * height
   * pitch
   * base (framebuffer physical address, format used is PixelBlueGreenRedReserved8BitPerColor)
3. MemoryMap structure
   * descriptor_size (size of EFI_MEMORY_DESCRIPTOR)
   * size (size of the memory map)
   * map (physical address of the list of EFI_MEMORY_DESCRIPTORS)

The messages displayed by the bootloader can be configured using the following
CMake options:
* basic_logging
* extensive_logging
* print_elf
* print_memory_map

Take a look in the CMakeLists.txt file to see how to configure.

To change the compiler modify the Toolchain.cmake file.