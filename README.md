# Bootloader

Simple 64-bit UEFI bootloader for the x86_64 architecture.
Kernel must be a 64-bit ELF executable. Program segments are loaded at the virtual
addresses specified in the program header table. Upon successful boot the processor
jumps to the kernel's entry point and loads the BootInfo structure's virtual 
address in register %rdi (first function parameter according to the System V ABI). 
4-level recursive paging is also set up by the bootloader.

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
* basic_logging: activates simple messages
* extensive_logging: activates messages from internal functions
* print_elf: prints elf headers
* print_memory_map: prints initial memory, it can change
* print_tokens: prints config file tokenization(only if built in Debug mode)
* print_config: prints chosen bootloader config

The bootloader is capable of loading other configurable files,
called initial datafiles. An example bootloader configuration can
be found in boot.cfg. The loader will search for the configuration 
in /config/boot.cfg. 

To change the compiler modify the Toolchain.cmake file. The built UEFI
application can be found in build/efi.