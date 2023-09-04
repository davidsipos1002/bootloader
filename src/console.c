#include <bootloader/console.h>
#include <stdbool.h>

EFI_STATUS setConsoleMode(EFI_SYSTEM_TABLE *ST) 
{
    UINTN maxMode = ST->ConOut->Mode->MaxMode;
    UINTN finalMode;
    UINTN col = 0;
    UINTN row = 0;
    UINTN columns, rows;
    for(UINTN i = 0;i < maxMode;i++) 
    {
        ST->ConOut->QueryMode(ST->ConOut, i, &columns, &rows);
        if(rows > row && columns > col)
        {
            row = rows;
            col = columns;
            finalMode = i;
        }
    }
    return ST->ConOut->SetMode(ST->ConOut, finalMode);
}

EFI_STATUS clearScreen(EFI_SYSTEM_TABLE *ST) 
{
    return ST->ConOut->ClearScreen(ST->ConOut);
}

EFI_STATUS printString(EFI_SYSTEM_TABLE *ST, UINTN COLOR, CHAR16 *String) 
{
    ST->ConOut->SetAttribute(ST->ConOut, COLOR);
    return ST->ConOut->OutputString(ST->ConOut, String);
}

EFI_STATUS printIntegerInDecimal(EFI_SYSTEM_TABLE *ST, UINTN Color, UINTN Integer) 
{
    UINTN n = Integer;
    UINTN digit = 1;
    while(n > 9) 
    {
        digit *= 10;
        n /= 10; 
    }
    CHAR16 buff[2];
    buff[1] = 0;
    while(digit) {
        buff[0] = (CHAR16) ((Integer / digit) % 10 + L'0');
        printString(ST, Color, buff);
        digit /= 10;
    }
    return EFI_SUCCESS;
}

EFI_STATUS printIntegerInHexadecimal(EFI_SYSTEM_TABLE *ST, UINTN Color, UINTN Integer) {
    UINTN currentshift = 60;
    UINTN digitcount = 0;
    bool firstDigit = false;
    CHAR16 buff[2];
    buff[0] = L'0';
    buff[1] = 0;
    printString(ST, Color, buff);
    buff[0] = L'x';
    printString(ST, Color, buff);
    if(Integer == 0) 
    {
        buff[0] = L'0';
        printString(ST, Color, buff);
        return EFI_SUCCESS;
    }
    while(digitcount < 16) {
        UINTN digit = (((uint64_t) 0xF << currentshift) & Integer) >> currentshift;
        if(!firstDigit && digit)
            firstDigit = true;
        if(firstDigit)
        {
            if(digit < 10)
                buff[0] = (CHAR16) (digit + L'0');
            else
                buff[0] = (CHAR16) ((digit - 10)  + L'A');
            printString(ST, Color, buff);
        }
        currentshift -= 4;
        digitcount++;
    } 
    return EFI_SUCCESS;
}

EFI_STATUS WaitForKeyPress(EFI_SYSTEM_TABLE *ST) 
{
    EFI_INPUT_KEY Key;
    EFI_STATUS Status = ST->ConIn->Reset(ST->ConIn, FALSE);
    if (EFI_ERROR(Status))
        return Status;
 
    while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);
    return Status;
}