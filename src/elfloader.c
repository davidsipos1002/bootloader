#include <bootloader/elfloader.h>
#include <bootloader/elf.h>
#include <bootloader/console.h>

void printElfHeader(EFI_SYSTEM_TABLE *ST, Elf64_Ehdr *elfHeader);
void printElfProgramHeaderTable(EFI_SYSTEM_TABLE *ST, Elf64_Phdr *programHeaderEntry);

EFI_STATUS loadElf(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE kernelImage) {
    Elf64_Ehdr elfHeader;
    UINTN BufferSize = sizeof(elfHeader);
    EFI_STATUS Status = kernelImage->Read(kernelImage, &BufferSize, &elfHeader);
    if(Status != EFI_SUCCESS) 
    {
        printString(ST, EFI_RED, L"Error reading ELF header\r\n");
        return EFI_LOAD_ERROR;
    }
    printElfHeader(ST, &elfHeader);
    Status = kernelImage->SetPosition(kernelImage, elfHeader.e_phoff);
    if(Status != EFI_SUCCESS || sizeof(Elf64_Phdr) != elfHeader.e_phentsize)
    {
        printString(ST, EFI_RED, L"Error reading ELF program header table \r\n");
        kernelImage->Close(kernelImage);
        return EFI_LOAD_ERROR;
    } 
    Elf64_Phdr pHeader;
    BufferSize = elfHeader.e_phentsize;
    for(Elf64_Half i = 0;i < elfHeader.e_phnum;i++) {
        Status = kernelImage->Read(kernelImage, &BufferSize, &pHeader);
        if(Status != EFI_SUCCESS) {
           printString(ST, EFI_RED, L"Error reading program header table entries\r\n");
           kernelImage->Close(kernelImage);
           return EFI_LOAD_ERROR; 
        }
        printString(ST, EFI_WHITE, L"Program Header Table Entry ");
        printIntegerInDecimal(ST, EFI_WHITE, i);
        newLine(ST);
        printElfProgramHeaderTable(ST, &pHeader);
    }
    return EFI_SUCCESS;
}

void printElfHeader(EFI_SYSTEM_TABLE *ST, Elf64_Ehdr *elfHeader) 
{
       printString(ST, EFI_WHITE, L"ELF Header\r\n");

    printString(ST, EFI_WHITE, L"e_ident: ");
    for(int i = 0;i < EI_NIDENT;i++) {
        printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_ident[i]);
        printString(ST, EFI_WHITE, L" ");
    }
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_type: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_type);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_machine: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_machine);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_version: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_version);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_entry: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_entry);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_phoff: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_phoff);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_shoff: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_shoff);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_flags: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_flags);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_ehsize: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_ehsize);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_phentsize: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_phentsize);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_phnum: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_phnum);
    newLine(ST);
    
    printString(ST, EFI_WHITE, L"e_shentsize: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_shentsize);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_shnum: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_shnum);
    newLine(ST);

    printString(ST, EFI_WHITE, L"e_shstrndx: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, elfHeader->e_shstrndx);
    newLine(ST);
}

void printElfProgramHeaderTable(EFI_SYSTEM_TABLE *ST, Elf64_Phdr *programHeaderEntry) {
    printString(ST, EFI_WHITE, L"p_type: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_type);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_flags: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_flags);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_offset: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_offset);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_vaddr: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_vaddr);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_paddr: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_paddr);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_filesz: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_filesz);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_memsz: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_memsz);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_align: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_align);
    newLine(ST);
}