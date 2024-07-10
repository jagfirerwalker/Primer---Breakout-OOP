#pragma once

#include <stdio.h>

#include <errno.h>

// This is  to get malloc
#include <stdlib.h>

// This is to get memset
#include <string.h>

// Used to get th edit timestamp of files
#include <sys/stat.h>

#include <math.h>

// ###########################################################
//                      Defines
// ###########################################################

#ifdef _WIN32
#  define DEBUG_BREAK() __debugbreak()
#elif __linux__
# define DEBUG_BREAK() __builtin_debugtrap()
#elif __APPLE__
# define DEBUG_BREAK() __builtin_trap()
#endif

// ###########################################################
//                      Logging
// ###########################################################

enum TextColor
{
    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_MAGENTA,
    TEXT_COLOR_CYAN,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_BRIGHT_BLACK,
    TEXT_COLOR_BRIGHT_RED,
    TEXT_COLOR_BRIGHT_GREEN,
    TEXT_COLOR_BRIGHT_YELLOW,
    TEXT_COLOR_BRIGHT_BLUE,
    TEXT_COLOR_BRIGHT_MAGENTA,
    TEXT_COLOR_BRIGHT_CYAN,
    TEXT_COLOR_BRIGHT_WHITE,
    TEXT_COLOR_COUNT
};

// Create a template that creates log messages
template <typename ...Args> // createa a template for the variadic arguments
void _log(const char* prefix, const char* message, TextColor textColor, Args... args)
{
    static const char* TextColorTable[TEXT_COLOR_COUNT] =
    {
        "\x1b[30m", // TEXT_COLOR_BLACK
        "\x1b[31m", // TEXT_COLOR_RED
        "\x1b[32m", // TEXT_COLOR_GREEN
        "\x1b[33m", // TEXT_COLOR_YELLOW
        "\x1b[34m", // TEXT_COLOR_BLUE
        "\x1b[35m", // TEXT_COLOR_MAGENTA
        "\x1b[36m", // TEXT_COLOR_CYAN
        "\x1b[37m", // TEXT_COLOR_WHITE
        "\x1b[90m", // TEXT_COLOR_BRIGHT_BLACK
        "\x1b[91m", // TEXT_COLOR_BRIGHT_RED
        "\x1b[92m", // TEXT_COLOR_BRIGHT_GREEN
        "\x1b[93m", // TEXT_COLOR_BRIGHT_YELLOW
        "\x1b[94m", // TEXT_COLOR_BRIGHT_BLUE
        "\x1b[95m", // TEXT_COLOR_BRIGHT_MAGENTA
        "\x1b[96m", // TEXT_COLOR_BRIGHT_CYAN
        "\x1b[97m", // TEXT_COLOR_BRIGHT_WHITE
    };

    
    // Print the prefix, message, and color
    printf("%s%s %s", TextColorTable[textColor], prefix, message);

    // Print additional arguments
    int dummy[] = { (printf(" %s", args), 0)... };
    (void)dummy; // Suppress unused variable warning

    printf("\033[0m\n");
}

#define SM_TRACE(message, ...) _log("TRACE: ", message, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define SM_WARN(message, ...) _log("WARN: ", message, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define SM_ERROR(message, ...) _log("ERROR: ", message,TEXT_COLOR_RED, ##__VA_ARGS__);

#define SM_ASSERT(x, message, ...)          \
{                                           \
    if (!(x))                               \
    {                                       \
        SM_ERROR(message, ##__VA_ARGS__);   \
        DEBUG_BREAK();                      \
        SM_ERROR("Assertion not Hit!");     \
    }                                       \
}

// #############################################################################
//                           Bump Allocator
// #############################################################################

struct BumpAllocator
{
  size_t capacity;
  size_t used;
  char* memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
  BumpAllocator ba = {};
  
  ba.memory = (char*)malloc(size);

    if (ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size); // Sets the memory to 0
    }
    else
    {
        SM_ASSERT(false, "Failed to allocate memory for BumpAllocator");
    }

    return ba;
}

char* bump_alloc(BumpAllocator* bumpAllocator, size_t size)
{
    char* result = nullptr; // Create a pointer to the memory that will be returned

    size_t allignedSize = (size + 7) & ~7; // Makes sure that the first 4 bits are 0, so that the memory is alligned
    if(bumpAllocator->used + allignedSize <= bumpAllocator->capacity) // If the used memory plus the size of the memory to be allocated is less than the capacity
    {
        result = bumpAllocator->memory + bumpAllocator->used; // Set the result to the memory at the used memory
        bumpAllocator->used += allignedSize; // Increment the used memory by the size of the memory to be allocated
    }
    else
    {
        SM_ASSERT(false, "BumpAllocator out of memory");
    }

    return result;
}


// #############################################################################
//                           File I/O
// #############################################################################

long long get_timestamp(const char* file)
{
    struct stat file_stat = {}; // Create a struct to store the file stats
    stat(file, &file_stat); // Get the file stats
    return file_stat.st_mtime; // Get the last modified time of the file
}

bool file_exists(const char* filePath)
{
    SM_ASSERT(filePath, "File path is null");

    FILE* file = nullptr;
    bool exists = false;

    #ifdef _MSC_VER // Check if we're compiling with Microsoft Visual C++
        errno_t err = fopen_s(&file, filePath, "rb");
        if (err == 0) // If the file was opened successfully
        {
            exists = true;
            fclose(file);
        }
    #else
        file = fopen(filePath, "rb");
        if (file)
        {
            exists = true;
            fclose(file);
        }
    #endif

    return exists;
}

long get_file_size(const char* filePath)
{
    SM_ASSERT(filePath, "File path is null");

    long fileSize = 0;
    auto file = fopen(filePath, "rb"); // Open the file in read binary mode
    if(!file) // If the file does not exist
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
    fileSize = ftell(file); // Get the position of the file pointer
    fseek(file, 0 , SEEK_SET); // Move the file pointer to the beginning of the file
    fclose(file); // Close the file

    return fileSize;
}

/*
* Reads a file inot a supplied buffer. We manage our own
* memory and therefore want more control over where it
* is allocated
*/

char* read_file(const char* filePath, int* fileSize, char* buffer)
{
    SM_ASSERT(filePath, "No filePath supplied");
    SM_ASSERT(fileSize, "No fileSize supplied");
    SM_ASSERT(buffer, "No buffer supplied");

    *fileSize = 0;
    auto file = fopen(filePath, "rb"); // Open the file in read binary mode
    if(!file) // If the file does not exist
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return nullptr;
    }

    fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
    *fileSize = ftell(file); // Get the position of the file pointer
    fseek(file, 0 , SEEK_SET); // Move the file pointer to the beginning of the file

    memset(buffer, 0, *fileSize + 1); // Set the buffer to 0
    fread(buffer, sizeof(char), *fileSize, file); // Read the file into the buffer
    fclose(file); // Close the file

    return buffer;
}

char* read_file(const char* filePath, int* fileSize, BumpAllocator* BumpAllocator)
{
    char* file = nullptr; // Create a pointer to the file
    long fileSize2 = get_file_size(filePath); // Get the size of the file

    if(fileSize2)
    {
        char* buffer = bump_alloc(BumpAllocator, fileSize2 + 1); // Allocate memory for the buffer

        file = read_file(filePath, fileSize, buffer); // Read the file into the buffer
    }

    return file;
}

void write_file(const char* filePath, char* buffer, int size)
{
    SM_ASSERT(filePath, "No filePath supplied");
    SM_ASSERT(buffer, "No buffer supplied");

    auto file = fopen(filePath, "wb"); // Open the file in write binary mode
    if(!file) // If the file does not exist
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return;
    }

    fwrite(buffer, sizeof(char), size, file); // Write the buffer to the file
    fclose(file); // Close the file
}

bool copy_file(const char* fileName, const char* outputName, char* buffer)
{
    int fileSize = 0;
    char* data = read_file(fileName, &fileSize, buffer); // Read the file into the buffer

    auto outputFile = fopen(outputName, "wb"); // Open the output file in write binary mode
    if(!outputFile) // If the output file does not exist
    {
        SM_ERROR("Failed to open file: %s", outputName);
        return false;
    }

    int result = fwrite(data, sizeof(char), fileSize, outputFile); // Write the data to the output file
    if(!result) // If the write failed
    {
        SM_ERROR("Failed to write to file: %s", outputName);
        return false;
    }

    fclose(outputFile); // Close the output file
    return true;
}

bool copy_file(const char* fileName, const char* outputName, BumpAllocator* BumpAllocator)
{
    char* file = 0;
    long fileSize2 = get_file_size(fileName); // Get the size of the file

    if(fileSize2)
    {
        char* buffer = bump_alloc(BumpAllocator, fileSize2 + 1); // Allocate memory for the buffer

        return copy_file(fileName, outputName, buffer); // Copy the file
    }

    return false;
}