#include <bootloader/config.h>
#include <bootloader/memory.h>
#include <bootloader/console.h>
#include <bootloader/filesystem.h>

#define SPACE printString(ST, EFI_WHITE, L" ")
#define PRINT(String) printString(ST, EFI_WHITE, String)

typedef enum 
{
    NONE,
    WHITESPACE,
    CHARACTER,
    STRING,
    BOOT_CONFIG,
    CONFIGURATION,
    LEFT_PARANTHESIS,
    RIGHT_PARANTHESIS,
    LEFT_BRACE,
    RIGHT_BRACE,
    EQUALS,
    KERNEL_PATH,
    BOOTINFO_ADDR,
    INITIAL_DATA_FILE,
    FILE_PATH,
    LOAD_ADDR
 } Token;

 typedef struct
 {
    uint32_t tokenSize;
    uint32_t tokenCount;
    Token *tokens;
    uint32_t stringSize;
    uint32_t stringCount;
    uint32_t *stringLengths;
    char **strings;
} ConfigFile;

uint32_t min(uint32_t a, uint32_t b)
{
    if(a < b)
        return a;
    return b;
}

void addToken(EFI_SYSTEM_TABLE *ST, ConfigFile *configFile, Token token)
{
    if(configFile->tokenCount >= configFile->tokenSize)
    {
        uint32_t bytes = configFile->tokenSize * sizeof(Token);
        configFile->tokens = realloc(ST, bytes, bytes << 1, configFile->tokens);
        configFile->tokenSize <<= 1;
    }
    configFile->tokens[configFile->tokenCount++] = token;
}

void addString(EFI_SYSTEM_TABLE *ST, ConfigFile *configFile, uint32_t length, char *string)
{
    if(configFile->stringCount >= configFile->stringSize)
    {
        uint32_t bytes = configFile->stringSize * sizeof(uint32_t);
        configFile->stringLengths = realloc(ST, bytes, bytes << 1, configFile->stringLengths);
        bytes = configFile->stringSize * sizeof(char *);
        configFile->strings = realloc(ST, bytes, bytes << 1, configFile->strings);
        configFile->stringSize <<= 1;
    }
    configFile->stringLengths[configFile->stringCount] = length;
    configFile->strings[configFile->stringCount++] = string;
}

void freeConfigFile(EFI_SYSTEM_TABLE *ST, ConfigFile *configFile)
{
    free(ST, configFile->tokens);
    for(uint32_t i = 0;i < configFile->stringCount;i++)
        free(ST, configFile->strings[i]);
    free(ST, configFile->stringLengths);
    free(ST, configFile->strings);
    free(ST, configFile);
}

ConfigFile* tokenize(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE configFile)
{
    ConfigFile *config = alloc(ST, sizeof(ConfigFile));
    config->tokenSize = 128;
    config->tokenCount = 0;
    config->stringSize = 128;
    config->stringCount = 0;
    config->tokens = alloc(ST, 128 * sizeof(Token));
    config->stringLengths = alloc(ST, 128 * sizeof(uint32_t));
    config->strings = alloc(ST, 128 * sizeof(char *));

    UINTN bufferSize = 1024;
    char *buffer = alloc(ST, bufferSize);
    uint32_t currentStringLength = 0;
    char currentString[MAX_STRING_LENGTH + 1];
    bool inString = false;

    while(!EFI_ERROR(configFile->Read(configFile, &bufferSize, buffer)) && bufferSize)
    {
        // #if defined(DEBUG_BUILD)
        //     printString(ST, EFI_WHITE, L"read ");
        //     printIntegerInDecimal(ST, EFI_WHITE, bufferSize);
        //     newLine(ST);
        //  #endif
        for(uint64_t i = 0;i < bufferSize;i++)
        {
            Token currentToken;
            switch (buffer[i])
            {
                case '(':
                    currentToken = LEFT_PARANTHESIS;
                    break;
                case ')':
                    currentToken = RIGHT_PARANTHESIS;
                    break;
                case '{':
                    currentToken = LEFT_BRACE;
                    break;
                case '}':
                    currentToken = RIGHT_BRACE;
                    break;
                case '=':
                    currentToken = EQUALS;
                    break;
                case 'a' ... 'z':
                case 'A' ... 'Z':
                case '0' ... '9':
                case '_':
                case '-':
                case '\\':
                case '.':
                    currentToken = CHARACTER;
                    break;
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                case '\v':
                case '\f':
                    currentToken = WHITESPACE;
                    break;
                default:
                    currentToken = NONE;
                    break;
            }
            if(currentToken == NONE)
            {
               freeConfigFile(ST, config);
               free(ST, buffer);
               return NULL; 
            }

            if(inString || currentToken == CHARACTER)
            {
                if(currentToken == CHARACTER)
                {
                    inString = true;
                    if(currentStringLength < MAX_STRING_LENGTH)
                        currentString[currentStringLength++] = buffer[i];
                }
                else
                {
                    if(stringEquals(currentString, "boot_config", min(currentStringLength, 11)))
                        addToken(ST, config, BOOT_CONFIG);
                    else if(stringEquals(currentString, "configuration", min(currentStringLength, 13)))
                        addToken(ST, config, CONFIGURATION);
                    else if(stringEquals(currentString, "kernelPath", min(currentStringLength, 10)))
                        addToken(ST, config, KERNEL_PATH);
                    else if(stringEquals(currentString, "bootInfoVirtualAddress", min(currentStringLength, 22)))
                        addToken(ST, config, BOOTINFO_ADDR);
                    else if(stringEquals(currentString, "initialDataFile", min(currentStringLength, 15)))
                        addToken(ST, config, INITIAL_DATA_FILE);
                    else if(stringEquals(currentString, "filePath", min(currentStringLength, 8)))
                        addToken(ST, config, FILE_PATH);
                    else if(stringEquals(currentString, "loadVirtualAddress", min(currentStringLength, 18)))
                        addToken(ST, config, LOAD_ADDR);
                    else
                    {
                        currentString[currentStringLength++] = '\0';
                        char *temp = alloc(ST, currentStringLength);
                        memcpy(temp, currentString, currentStringLength);
                        addToken(ST, config, STRING);
                        addString(ST, config, currentStringLength, temp);
                    }
                    currentStringLength = 0;
                    inString = false;
                    if(currentToken != WHITESPACE)
                        addToken(ST, config, currentToken);
                }
            } 
            else if(currentToken != WHITESPACE)
                addToken(ST, config, currentToken);
        }
        bufferSize = 1024;
    }
    free(ST, buffer);
    return config;
}

#if defined(DEBUG_BUILD) && defined(PRINT_TOKENS)
void printTokens(EFI_SYSTEM_TABLE *ST, ConfigFile *configFile)
{
    uint32_t stringIndex = 0;
    for(uint32_t i = 0;i < configFile->tokenCount;i++)
    {
        if(configFile->tokens[i] == STRING)
        {
            PRINT(L"String: ");
            CHAR16 *buffer = alloc(ST, configFile->stringLengths[stringIndex] * sizeof(CHAR16));
            toWidechar(configFile->strings[stringIndex], buffer, configFile->stringLengths[stringIndex]);
            printString(ST, EFI_WHITE, buffer);
            PRINT(L"Length: ");
            printIntegerInHexadecimal(ST, EFI_WHITE, configFile->stringLengths[stringIndex]);
            free(ST, buffer);
            stringIndex++;
        }
        else
        {
            PRINT(L"Token: ");
            switch(configFile->tokens[i])
            {
                case BOOT_CONFIG:
                    PRINT(L"boot_info");
                    break;
                case CONFIGURATION:
                    PRINT(L"configuration");
                    break;
                case KERNEL_PATH:
                    PRINT(L"kernelPath");
                    break;
                case BOOTINFO_ADDR:
                    PRINT(L"bootInfoVirtualAddress");
                    break;
                case INITIAL_DATA_FILE:
                    PRINT(L"initialDataFile");
                    break;
                case FILE_PATH:
                    PRINT(L"filePath");
                    break;
                case LOAD_ADDR:
                    PRINT(L"loadVirtualAddress");
                    break;
                case LEFT_PARANTHESIS:
                    PRINT(L"(");
                    break;
                case RIGHT_PARANTHESIS:
                    PRINT(L")");
                    break;
                case LEFT_BRACE:
                    PRINT(L"{");
                    break;
                case RIGHT_BRACE:
                    PRINT(L"}");
                    break;
                case EQUALS:
                    PRINT(L"=");
                    break;
                default:
                    PRINT(L"NOT SUPPOSED TO BE HERE");
                    break;
            }
        } 
        SPACE;
        SPACE;
    }
}
#endif

char* getChosenBootConfig(ConfigFile *configFile, uint32_t *length)
{
    if(configFile->tokenCount < 4)
        return NULL;
    if(configFile->tokens[0] != BOOT_CONFIG)
        return NULL;
    if(configFile->tokens[1] != LEFT_PARANTHESIS)
        return NULL;
    if(configFile->tokens[2] != STRING)
        return NULL;
    if(configFile->tokens[3] != RIGHT_PARANTHESIS)
        return NULL;
    *length = configFile->stringLengths[0];
    return configFile->strings[0];
}

void printChars(EFI_SYSTEM_TABLE *ST, const char *string, uint32_t length);

BootConfig* generateBootConfig(EFI_SYSTEM_TABLE *ST, ConfigFile *configFile)
{
    // insideConfig:
    // 1 met config
    // 2 left p
    // 3 string
    // 4 right
    // 5 left brace
    // 0 right brace
    // insideInitial:
    // 1 met initial
    // 2 left brace
    // 0 right brace
    uint32_t bootConfigLength = 0;
    char *chosenConfig = getChosenBootConfig(configFile, &bootConfigLength);
    if(!chosenConfig || configFile->tokenCount < 5)
        return NULL;
    uint32_t insideConfig = 0;
    uint32_t configFields = 0;
    uint32_t insideInitial = 0;
    uint32_t initialFields = 0;
    uint32_t stringIndex = 1;
    bool ignore = true;
    bool found = false;
    BootConfig *config = alloc(ST, sizeof(BootConfig));
    config->dataFileSize = 10 * sizeof(InitialDataFile);
    config->dataFileCount = 0;
    config->dataFiles = alloc(ST, config->dataFileSize);
    Token leftHandSide = NONE;
    Token previous = NONE;
    #define DIE {printString(ST, EFI_RED, L"Config snytax error code: "); printIntegerInDecimal(ST, EFI_RED, __LINE__); newLine(ST); freeBootConfig(ST, config); return NULL;}
    for(uint32_t i = 4;i < configFile->tokenCount;i++)
    {
        switch(configFile->tokens[i])
        {
            case CONFIGURATION:
                if(!insideConfig)
                    insideConfig++;
                else
                    DIE
                break;
            case LEFT_PARANTHESIS:
                if(insideConfig == 1)
                    insideConfig++;
                else
                    DIE
                break;
            case STRING:
                char *currentString = configFile->strings[stringIndex];
                uint32_t currentLength = configFile->stringLengths[stringIndex];
                if(insideConfig == 2)
                {
                    insideConfig++;
                    if(stringEquals(currentString, chosenConfig, min(currentLength, bootConfigLength)))
                    {
                        ignore = false;
                        config->configNameLength = currentLength;
                        memcpy(config->configName, currentString, currentLength);
                        found = true;
                    }
                } 
                else if(!ignore && leftHandSide != NONE)
                {
                    if(insideConfig == 5 && !insideInitial)
                    {
                        if(leftHandSide == KERNEL_PATH)
                        {
                            config->kernelPathLength = currentLength;
                            memcpy(config->kernelPath, currentString, currentLength);
                        }
                        else
                            config->bootInfoVirtualAddress = hexadecimalToInt(currentString, currentLength - 1);
                        leftHandSide = NONE;
                        configFields++;
                    } 
                    else if(insideConfig == 5 && insideInitial == 2)
                    {
                        if(config->dataFileCount >= config->dataFileSize)
                        {
                            uint32_t bytes = config->dataFileSize * sizeof(InitialDataFile);
                            config->dataFiles = realloc(ST, bytes, bytes << 1, config->dataFiles);
                            config->dataFileSize <<= 1;
                        }
                        if(leftHandSide == FILE_PATH)
                        {
                            config->dataFiles[config->dataFileCount].filePathLength = currentLength;
                            memcpy(config->dataFiles[config->dataFileCount].filePath, currentString, currentLength);
                        }
                        else
                            config->dataFiles[config->dataFileCount].loadVirtualAddress = hexadecimalToInt(currentString, currentLength - 1);
                        initialFields++;
                        leftHandSide = NONE;
                        if(initialFields == 2)
                            config->dataFileCount++;
                    }
                    else
                        DIE
                }
                stringIndex++;
                break;
            case RIGHT_PARANTHESIS:
                if(insideConfig == 3)
                    insideConfig++;
                else
                    DIE
                break;
            case LEFT_BRACE:
                if(insideConfig == 4)
                    insideConfig++;
                else if(insideConfig == 5 && insideInitial == 1)
                    insideInitial++;
                else
                    DIE
                break;
            case RIGHT_BRACE:
                if(insideInitial == 2)
                {
                    if(!ignore && initialFields != 2)
                        DIE
                    insideInitial = 0;
                    initialFields = 0;
                }
                else if(!insideInitial && insideConfig == 5) {
                    if(!ignore && configFields != 2)
                        DIE
                    insideConfig = 0;
                    configFields = 0;
                    ignore = true;
                }
                else
                    DIE
                break;
            case INITIAL_DATA_FILE:
                if(insideConfig == 5 && !insideInitial)
                    insideInitial++;
                else
                    DIE
                break;
            case KERNEL_PATH:
            case BOOTINFO_ADDR:
                if(insideConfig == 5 && !insideInitial)
                    leftHandSide = configFile->tokens[i];
                else
                    DIE
                break;
            case FILE_PATH:
            case LOAD_ADDR:
                if(insideConfig == 5 && insideInitial == 2) 
                    leftHandSide = configFile->tokens[i];
                else
                    DIE
                break;
            case EQUALS:
                if(previous != leftHandSide)
                    DIE
                break;
        }
        previous = configFile->tokens[i];
        // #if defined(DEBUG_BUILD)
        //     printString(ST, EFI_WHITE, L"config ");
        //     printIntegerInDecimal(ST, EFI_WHITE, insideConfig);
        //     printString(ST, EFI_WHITE, L" initial ");
        //     printIntegerInDecimal(ST, EFI_WHITE, insideInitial);
        //     printString(ST, EFI_WHITE, L" // ");
        // #endif
    }
    if(!found)
        DIE
    return config;
}

void printChars(EFI_SYSTEM_TABLE *ST, const char *string, uint32_t length)
{
    CHAR16 *buffer = alloc(ST, length * sizeof(CHAR16));
    toWidechar(string, buffer, length);
    printString(ST, EFI_WHITE, buffer);
    free(ST, buffer);
}

void printBootConfig(EFI_SYSTEM_TABLE *ST, BootConfig *bootConfig)
{
    printString(ST, EFI_WHITE, L"Configuration name: ");
    printChars(ST, bootConfig->configName, bootConfig->configNameLength);
    newLine(ST);
    printString(ST, EFI_WHITE, L"Kernel path: ");
    printChars(ST, bootConfig->kernelPath, bootConfig->kernelPathLength);
    newLine(ST);
    printString(ST, EFI_WHITE, L"BootInfo virtual address: ");
    printIntegerInHexadecimal(ST, EFI_WHITE, bootConfig->bootInfoVirtualAddress);
    newLine(ST);
    for(uint32_t i = 0;i < bootConfig->dataFileCount;i++)
    {
        printString(ST, EFI_WHITE, L"Initial file ");
        printIntegerInDecimal(ST, EFI_WHITE, i);
        newLine(ST);
        printString(ST, EFI_WHITE, L"File path: ");
        printChars(ST, bootConfig->dataFiles[i].filePath, bootConfig->dataFiles[i].filePathLength);
        newLine(ST);
        printString(ST, EFI_WHITE, L"Load virtual address: ");
        printIntegerInHexadecimal(ST, EFI_WHITE, bootConfig->dataFiles[i].loadVirtualAddress);
        newLine(ST);
    }
}

BootConfig* parseConfigurationFile(EFI_SYSTEM_TABLE *ST, EFI_FILE_HANDLE rootDirectory)
{
    EFI_FILE_HANDLE configFile; 
    if(EFI_ERROR(openFileForRead(rootDirectory, L"\\config\\boot.cfg", &configFile)))
    {
        printString(ST, EFI_RED, L"Cannot open configuration file\r\n");
        return NULL;
    }
    ConfigFile *config = tokenize(ST, configFile);
    if(config == NULL)
    {
        closeFileHandle(configFile);
        return NULL;
    }
    #if defined(DEBUG_BUILD) && defined(PRINT_TOKENS)
        printTokens(ST, config);
        newLine(ST);
    #endif
    BootConfig *bootConfig = generateBootConfig(ST, config);
    freeConfigFile(ST, config);
    if(bootConfig == NULL)
    {
        closeFileHandle(configFile);
        return NULL;
    }
    closeFileHandle(configFile);
    return bootConfig;
}

void freeBootConfig(EFI_SYSTEM_TABLE *ST, BootConfig *bootConfig)
{
    free(ST, bootConfig->dataFiles);
    free(ST, bootConfig);
}