#ifndef ELF_H_INCLUDE
#define ELF_H_INCLUDE

#include <stdint.h>

/*
    ELF64 Types
*/
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_SWord;
typedef uint64_t Elf64_XWord;
typedef int64_t Elf64_SXWord;

/*
    ELF64 Header
*/
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define EI_PAD 9
#define EI_NIDENT 16

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define EV_NONE 0
#define EV_CURRENT 1

#define ELFOSABI_SYSV 0
#define ELFOSABI_HPUX 1
#define ELFOSABI_NETBSD 2
#define ELFOSABI_LINUX 3
#define ELFOSABI_GNUHURD 4
#define ELFOSABI_SOLARIS 6
#define ELFOSABI_AIX 7
#define ELFOSABI_IRIX 8
#define ELFOSABI_FREEBSD 9
#define ELFOSABI_TRU64 10
#define ELFOSABI_NOVELL 11
#define ELFOSABI_OPENBSD 12
#define ELFOSABI_OPENVMS 13
#define ELFOSABI_NONSTOP 14
#define ELFOSABI_AROS 15
#define ELFOSABI_FENIXOS 16
#define ELFOSABI_NUXI 17
#define ELFOSABI_STRATUSOPENVOS 18

#define ET_NONE 0 /* Unknown */
#define ET_REL 1 /* Relocatable file */
#define ET_EXEC 2 /* Executable file */
#define ET_DYN 3 /* Shared object */
#define ET_CORE 4 /* Core file */

#define EM_NONE 0
#define EM_X86 3
#define EM_AMDX86_64 0x3E

typedef struct 
{
    unsigned char e_ident[16]; /* ELF identification */
    Elf64_Half e_type; /* Object file type */
    Elf64_Half e_machine; /* Machine type */
    Elf64_Word e_version; /* Object file version */
    Elf64_Addr e_entry; /* Entry point address */
    Elf64_Off e_phoff; /* Program header offset */
    Elf64_Off e_shoff; /* Section header offset */
    Elf64_Word e_flags; /* Processor-specific flags */
    Elf64_Half e_ehsize; /* ELF header size */
    Elf64_Half e_phentsize; /* Size of program header entries */
    Elf64_Half e_phnum; /* Number of program header entries */
    Elf64_Half e_shentsize; /* Size of section header entries */
    Elf64_Half e_shnum; /* Number of section header entries */
    Elf64_Half e_shstrndx; /* Section name string table index */
}Elf64_Ehdr;

/*
    ELF Program Header Table
*/
#define PT_NULL 0 /* Unused entry */
#define PT_LOAD 1 /* Loadable segment */
#define PT_DYNAMIC 2 /* Dynamic linking information */
#define PT_INTERP 3 /* Interpreter information */
#define PT_NOTE 4 /* Auxiliary information */
#define PT_SHLIB 5 /* Reserved */
#define PT_PHDR 6 /* Segment containing program header itself */
#define PT_TLS 7 /* Thread-Local Storage template */
#define PT_LOOS 0x6000000 /* Reserved. OS Specific */
#define PT_HIOS 0X6FFFFFF /* Reserved. OS Specific */
#define PT_LOPROC 0x7000000 /* Reserved. Processor Specific */
#define PT_HIPROC 0x7FFFFFF /* Reserved. Processor Specific */

#define PF_X 1 /* Executable segment */
#define PF_W 2 /* Writable segment */
#define PF_R 4 /* Readable segment */

typedef struct 
{
    Elf64_Word p_type; /* Type of segment */
    Elf64_Word p_flags; /* Segment attributes */
    Elf64_Off p_offset; /* Offset in file */
    Elf64_Addr p_vaddr; /* Virtual address in memory */
    Elf64_Addr p_paddr; /* Physical address in memory */
    Elf64_XWord p_filesz; /* Size of segment in file */
    Elf64_XWord p_memsz; /* Size of segment in memory */
    Elf64_XWord p_align; /* Alignment of segment */
}Elf64_Phdr;

#endif