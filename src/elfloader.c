#include <bootloader/elfloader.h>
#include <bootloader/elf.h>
#include <bootloader/console.h>
#include <bootloader/filesystem.h>
#include <bootloader/paging.h>
#include <stdbool.h>

EFI_STATUS loadElf(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE kernelImage, uint64_t *pml4, uint64_t *kernelEntry);
bool validateElfHeader(EFI_SYSTEM_TABLE *ST, Elf64_Ehdr *elfHeader);
void printElfHeader(EFI_SYSTEM_TABLE *ST, Elf64_Ehdr *elfHeader);
void printElfProgramHeaderTable(EFI_SYSTEM_TABLE *ST, Elf64_Phdr *programHeaderEntry);
uint64_t getPageCount(Elf64_XWord p_memsz);

EFI_STATUS loadKernel(EFI_SYSTEM_TABLE *ST, EFI_HANDLE ImageHandle, uint64_t *pml4, uint64_t *kernelEntry) {
    EFI_FILE_HANDLE rootDirectory = getRootDirectory(ImageHandle, ST);
    if(rootDirectory == NULL) 
        return EFI_LOAD_ERROR;
    EFI_FILE_HANDLE kernelImage;
    EFI_STATUS Status = openKernelImage(rootDirectory, &kernelImage);
    if(Status != EFI_SUCCESS) 
        return EFI_LOAD_ERROR;
    if(EFI_ERROR(loadElf(ST, kernelImage, pml4, kernelEntry)))
        return EFI_LOAD_ERROR;
    kernelImage->Close(kernelImage);
    rootDirectory->Close(rootDirectory);
    return EFI_SUCCESS;
}

EFI_STATUS loadElf(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE kernelImage, uint64_t *pml4, uint64_t *kernelEntry) {
    Elf64_Ehdr elfHeader;
    UINTN BufferSize = sizeof(elfHeader);
    EFI_STATUS Status = kernelImage->Read(kernelImage, &BufferSize, &elfHeader);
    if(Status != EFI_SUCCESS) 
    {
        printString(ST, EFI_RED, L"Error reading ELF header\r\n");
        return EFI_LOAD_ERROR;
    }
    if(!validateElfHeader(ST, &elfHeader))
        return EFI_LOAD_ERROR;
    #ifdef PRINT_ELF
        printElfHeader(ST, &elfHeader);
    #endif
    Status = kernelImage->SetPosition(kernelImage, elfHeader.e_phoff);
    if(Status != EFI_SUCCESS || sizeof(Elf64_Phdr) != elfHeader.e_phentsize)
    {
        printString(ST, EFI_RED, L"Error reading ELF program header table \r\n");
        kernelImage->Close(kernelImage);
        return EFI_LOAD_ERROR;
    } 
    Elf64_Phdr pHeader;
    BufferSize = elfHeader.e_phentsize;
    for(Elf64_Half i = 0;i < elfHeader.e_phnum;i++) 
    {
        kernelImage->SetPosition(kernelImage, elfHeader.e_phoff + elfHeader.e_phentsize * i);
        Status = kernelImage->Read(kernelImage, &BufferSize, &pHeader);
        if(Status != EFI_SUCCESS) 
        {
           printString(ST, EFI_RED, L"Error reading program header table entries\r\n");
           kernelImage->Close(kernelImage);
           return EFI_LOAD_ERROR; 
        }

        #ifdef PRINT_ELF
            printString(ST, EFI_WHITE, L"Program Header Table Entry ");
            printIntegerInDecimal(ST, EFI_WHITE, i);
            newLine(ST);
            printElfProgramHeaderTable(ST, &pHeader);
        #endif
        
        if(pHeader.p_align != PAGE_SIZE) {
            printString(ST, EFI_RED, L"Invalid alignment field\r\n");
            return EFI_LOAD_ERROR;
        }
        if(pHeader.p_offset % pHeader.p_align != pHeader.p_vaddr % pHeader.p_align) {
            printString(ST, EFI_RED, L"Invalid section alignment\r\n");
            return EFI_LOAD_ERROR;
        }
        uint64_t pageCount = getPageCount(pHeader.p_memsz);
        uint64_t alignedVAddr = pHeader.p_vaddr & (~0xFFF);
        uint64_t *segment = allocateZeroedPages(ST, pageCount); 
        if(segment == NULL)
        {
            printString(ST, EFI_RED, L"Could not allocate pages\r\n");
            return EFI_LOAD_ERROR; 
        }
        if(!memoryMapPages(ST, pml4, (uint64_t) segment, alignedVAddr, pageCount)) {
            printString(ST, EFI_RED, L"Could not map allocated pages\r\n");
            return EFI_LOAD_ERROR;
        }
        if(EFI_ERROR(kernelImage->SetPosition(kernelImage, pHeader.p_offset))) {
            printString(ST, EFI_RED, L"Could not set file position\r\n");
            return EFI_LOAD_ERROR;
        }
        UINTN size = pHeader.p_filesz;
        if(EFI_ERROR(kernelImage->Read(kernelImage, &size, (void *) segment))) {
            printString(ST, EFI_RED, L"Could not read program segment\r\n");
            return EFI_LOAD_ERROR;
        }
    }
    *kernelEntry = elfHeader.e_entry;
    return EFI_SUCCESS;
}

bool validateElfHeader(EFI_SYSTEM_TABLE *ST, Elf64_Ehdr *elfHeader)
{
    if(elfHeader->e_ident[EI_MAG0] != 0x7F || elfHeader->e_ident[EI_MAG1] != 'E' || 
    elfHeader->e_ident[EI_MAG2] != 'L' || elfHeader->e_ident[EI_MAG3] != 'F')
    {
        printString(ST, EFI_RED, L"Kernel image is not in the ELF format\r\n");
        return false;
    }
    if(elfHeader->e_ident[EI_CLASS] != ELFCLASS64) 
    {
        printString(ST, EFI_RED, L"Kernel image not a 64-bit file\r\n");
        return false;
    }
    if(elfHeader->e_ident[EI_DATA] != ELFDATA2LSB) 
    {
        printString(ST, EFI_RED, L"Kernel image is not little endian\r\n");
        return false;
    }
    if(elfHeader->e_ident[EI_VERSION] != EV_CURRENT) 
    {
        printString(ST, EFI_RED, L"Kernel image has incorrect version\r\n");
        return false;
    }
    if(elfHeader->e_ident[EI_OSABI] != ELFOSABI_SYSV) 
    {
        printString(ST, EFI_RED, L"Kernel image has incorrect ABI\r\n");
        return false;
    }
    if(elfHeader->e_type != ET_EXEC)
    {
        printString(ST, EFI_RED, L"Kernel image is not executable\r\n");
        return false;
    }
    if(elfHeader->e_machine != EM_AMDX86_64)
    {
        printString(ST, EFI_RED, L"Kernel image is not compatible with the processor\r\n");
        return false;
    }
    return true;
}

void printElfHeader(EFI_SYSTEM_TABLE *ST, Elf64_Ehdr *elfHeader) 
{
       printString(ST, EFI_WHITE, L"ELF Header\r\n");

    printString(ST, EFI_WHITE, L"e_ident: ");
    for(int i = 0;i < EI_NIDENT;i++) 
    {
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
    uint64_t pageCount = getPageCount(programHeaderEntry->p_memsz);
    printString(ST, EFI_WHITE, L" pageCount: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, pageCount);
    newLine(ST);

    printString(ST, EFI_WHITE, L"p_align: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, programHeaderEntry->p_align);
    newLine(ST);
}