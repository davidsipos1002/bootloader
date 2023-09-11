#include <bootloader/filesystem.h>
#include <bootloader/console.h>

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

EFI_STATUS openFileForRead(EFI_FILE_HANDLE rootDirectory, CHAR16 *path, EFI_FILE_HANDLE *fileHandle) 
{
    return rootDirectory->Open(rootDirectory, fileHandle, 
        path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
} 

EFI_STATUS closeFileHandle(EFI_FILE_HANDLE fileHandle)
{
    return fileHandle->Close(fileHandle);
}