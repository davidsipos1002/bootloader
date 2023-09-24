#include <bootloader/filesystem.h>
#include <bootloader/console.h>
#include <bootloader/paging.h>
#include <bootloader/memory.h>

EFI_FILE_HANDLE getRootDirectory(EFI_HANDLE Image, EFI_SYSTEM_TABLE *ST) 
{
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *IOVolume;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE Volume;
    EFI_STATUS Status = ST->BootServices->HandleProtocol(Image, &lipGuid, (VOID **) &loaded_image); 
    if(Status != EFI_SUCCESS) 
    {
        printString(ST, EFI_RED, L"Could not obtain EFI_LOADED_IMAGE_PROTOCOL");
        return NULL;
    }
    Status = ST->BootServices->HandleProtocol(loaded_image->DeviceHandle, &fsGuid, (VOID **) &IOVolume);
    if(Status != EFI_SUCCESS) 
    {
        printString(ST, EFI_RED, L"Could not obtain EFI_FILE_SYSTEM_PROTOCOL");
        return NULL;
    }
    Status = IOVolume->OpenVolume(IOVolume, &Volume);
    if(Status != EFI_SUCCESS) 
    {
        return NULL;
    }
    return Volume;
}

uint64_t getFileSize(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE fileHandle)
{
    void *buffer = alloc(ST, 256);
    UINTN bufferSize = 256;
    EFI_GUID fileInfoGuid = EFI_FILE_INFO_ID;
    while(EFI_ERROR(fileHandle->GetInfo(fileHandle, &fileInfoGuid, &bufferSize, buffer)))
    {
        free(ST, buffer);
        buffer = alloc(ST, bufferSize);
    }
    uint64_t fileSize = ((EFI_FILE_INFO *) buffer)->FileSize;
    free(ST, buffer);
    return fileSize;
}

uint64_t loadFileToMemory(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE rootDirectory, CHAR16 *filePath, uint64_t *pageCount) 
{
    EFI_FILE_HANDLE fileHandle;
    if(EFI_ERROR(openFileForRead(rootDirectory, filePath, &fileHandle)))
        return 0;
    uint64_t fileSize = getFileSize(ST, fileHandle);
    *pageCount = getPageCount(fileSize);
    void *buffer = allocateZeroedPages(ST, EfiReservedMemoryType, *pageCount);
    UINTN bufferSize = *pageCount * PAGE_SIZE;
    EFI_STATUS Status = fileHandle->Read(fileHandle, &bufferSize, buffer);
    closeFileHandle(fileHandle);
    if(EFI_ERROR(Status))
    {
        ST->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) buffer, *pageCount);
        return 0;
    }
    return (uint64_t) buffer;
}

EFI_STATUS openFileForRead(EFI_FILE_HANDLE rootDirectory, CHAR16 *path, EFI_FILE_HANDLE *fileHandle) 
{
    return rootDirectory->Open(rootDirectory, fileHandle, 
        path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
} 

EFI_STATUS closeFileHandle(EFI_FILE_HANDLE fileHandle)
{
    return fileHandle->Close(fileHandle);
}