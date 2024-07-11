#pragma once // Ensures this header is only included once

// Standard library includes
#include <stdio.h>
#include <stdlib.h>  // For malloc
#include <string.h>  // For memset
#include <sys/stat.h>  // For getting file timestamps
#include <cstdarg>
#include <cstdio>


// #############################################################################
//                           Defines
// #############################################################################

// Cross-platform debug break macro
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#endif

// Utility macros
#define BIT(x) 1 << (x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))

// #############################################################################
//                           Logging
// #############################################################################

// Enum for text colors in console output
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

// Template function for logging with color
template <typename ...Args>
void _log(const char* prefix, const char* msg, TextColor textColor, int dummy, Args... args)
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
    
    char formatBuffer[8192];
    snprintf(formatBuffer, sizeof(formatBuffer), "%s%s %s\033[0m", TextColorTable[textColor], prefix, msg);
    
    char textBuffer[8912];
    snprintf(textBuffer, sizeof(textBuffer), formatBuffer, args...);
    
    puts(textBuffer);
}

// Macros for different log levels
#define SM_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_GREEN, 0, ##__VA_ARGS__)
#define SM_WARN(msg, ...) _log("WARN:  ", msg, TEXT_COLOR_YELLOW, 0, ##__VA_ARGS__)
#define SM_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, 0, ##__VA_ARGS__)
#define SM_ASSERT(condition, message, ...) \
    do { \
        if (!(condition)) { \
            _log("ASSERT: ", message, TEXT_COLOR_RED, 0, ##__VA_ARGS__); \
            __debugbreak(); \
        } \
    } while(0)


// #############################################################################
//                           Bump Allocator
// #############################################################################

// Bump Allocator for simple memory management
struct BumpAllocator
{
  size_t capacity;
  size_t used;
  char* memory;
};

// Functions for Bump Allocator
BumpAllocator make_bump_allocator(size_t size)
{
  BumpAllocator ba = {};
  
  ba.memory = (char*)malloc(size);
  if(ba.memory)
  {
    ba.capacity = size;
    memset(ba.memory, 0, size); // Sets the memory to 0
  }
  else
  {
    SM_ASSERT(false, "Failed to allocate Memory!");
  }

  return ba;
}

char* bump_alloc(BumpAllocator* bumpAllocator, size_t size)
{
  char* result = nullptr;

  size_t allignedSize = (size + 7) & ~ 7; // This makes sure the first 4 bits are 0 
  if(bumpAllocator->used + allignedSize <= bumpAllocator->capacity)
  {
    result = bumpAllocator->memory + bumpAllocator->used;
    bumpAllocator->used += allignedSize;
  }
  else
  {
    SM_ASSERT(false, "BumpAllocator is full");
  }

  return result;
}

// #############################################################################
//                           File I/O
// #############################################################################
long long get_timestamp(const char* file)
{
  struct stat file_stat = {};
  stat(file, &file_stat);
  return file_stat.st_mtime;
}

bool file_exists(const char* filePath)
{
  SM_ASSERT(filePath, "No filePath supplied!");

  FILE* file = nullptr;
  errno_t err = fopen_s(&file, filePath, "rb");
  if(err != 0 || !file)
  {
    SM_ERROR("Failed opening File: %s", filePath);
    return false;
  }
  
  fclose(file);

  return true;
}

long get_file_size(const char* filePath)
{
  SM_ASSERT(filePath, "No filePath supplied!");

  long fileSize = 0;

  FILE* file = nullptr;
  errno_t err = fopen_s(&file, filePath, "rb");
  if(err != 0 || !file)
  {
    return false;
  }

  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  fclose(file);

  return fileSize;
}

/*
* Reads a file into a supplied buffer. We manage our own
* memory and therefore want more control over where it 
* is allocated
*/
char* read_file(const char* filePath, int* fileSize, char* buffer)
{
  SM_ASSERT(filePath, "No filePath supplied!");
  SM_ASSERT(fileSize, "No fileSize supplied!");
  SM_ASSERT(buffer, "No buffer supplied!");

  *fileSize = 0;
  FILE* file = nullptr;
  errno_t err = fopen_s(&file, filePath, "rb");
  if(err != 0 || !file)
  {
    SM_ERROR("Failed opening File: %s", filePath);
    return nullptr;
  }

  fseek(file, 0, SEEK_END);
  *fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  memset(buffer, 0, *fileSize + 1);
  fread(buffer, sizeof(char), *fileSize, file);

  fclose(file);

  return buffer;
}

char* read_file(char* filePath, int* fileSize, BumpAllocator* bumpAllocator)
{
  char* file = nullptr;
  long fileSize2 = get_file_size(filePath);

  if(fileSize2)
  {
    char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

    file = read_file(filePath, fileSize, buffer);
  }

  return file; 
}

void write_file(char* filePath, char* buffer, int size)
{
  SM_ASSERT(filePath, "No filePath supplied!");
  SM_ASSERT(buffer, "No buffer supplied!");
  FILE* file = nullptr;
  errno_t err = fopen_s(&file, filePath, "wb");
  if(err != 0 || !file)
  {
    SM_ERROR("Failed opening File: %s", filePath);
    return;
  }

  fwrite(buffer, sizeof(char), size, file);
  fclose(file);
}

bool copy_file(char* fileName, char* outputName, char* buffer)
{
  int fileSize = 0;
  char* data = read_file(fileName, &fileSize, buffer);

  FILE* outputFile = nullptr;
  errno_t err = fopen_s(&outputFile, outputName, "wb");
  if(err != 0 || !outputFile)
  {
    SM_ERROR("Failed opening File: %s", outputName);
    return false;
  }

  int result = fwrite(data, sizeof(char), fileSize, outputFile);
  if(!result)
  {
    SM_ERROR("Failed opening File: %s", outputName);
    return false;
  }
  
  fclose(outputFile);

  return true;
}

bool copy_file(char* fileName, char* outputName, BumpAllocator* bumpAllocator)
{
  char* file = 0;
  long fileSize2 = get_file_size(fileName);

  if(fileSize2)
  {
    char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

    return copy_file(fileName, outputName, buffer);
  }

  return false;
}

// #############################################################################
//                           Math stuff
// #############################################################################
struct Vec2
{
  float x;
  float y;
};

struct IVec2
{
  int x;
  int y;
};