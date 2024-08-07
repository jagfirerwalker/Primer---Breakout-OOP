#pragma once // Ensures this header is only included once

// Standard library includes
#include <stdio.h>
#include <stdlib.h>  // For malloc
#include <string.h>  // For memset
#include <sys/stat.h>  // For getting file timestamps
#include <cstdarg>
#include <cstdio>
#include <math.h>


// #############################################################################
//                           Defines
// #############################################################################

// Cross-platform debug break macro
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif

// Utility macros
#define BIT(x) 1 << (x)
#define b8 char // b8 is of type char, e.g. b8 a = 0;
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
//                           Array
// #############################################################################

template<typename T, int N>
struct Array
{
  static constexpr int maxElements = N;
  int count = 0;
  T elements[N];

  T& operator[](int idx)
  {
    SM_ASSERT(idx >= 0, "idx negative!");
    SM_ASSERT(idx < count, "Idx out of bounds!");// if idx is greater than count, it is out of bounds
    return elements[idx];
  }

  int add(T element)
  {
    SM_ASSERT(count < maxElements, "Array is full!");
    elements[count] = element;
    return count++;
  }

  void remove_idx_and_swap(int idx)
  {
    SM_ASSERT(idx >= 0, "idx negative!");
    SM_ASSERT(idx < count, "Idx out of bounds!");

    elements[idx] = elements[--count];
  }

  void clear()
  {
    count = 0;
  }

  bool is_full()
  {
    return count == N;
  }
};

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
long long max(long long a, long long b)
{
  if(a>b)
  {
    return a;
  }
  return b;
}

// This function performs linear interpolation (LERP) between two values
float lerp(float a, float b, float t)
{
  // 'a' is the start value
  // 'b' is the end value
  // 't' is the interpolation factor (usually between 0 and 1)

  // The formula calculates a point between 'a' and 'b' based on 't'
  return a + (b - a) * t;
  
  // When t = 0, the result is 'a'
  // When t = 1, the result is 'b'
  // For values of t between 0 and 1, it returns a value between 'a' and 'b'
}

struct Vec2
{
  float x;
  float y;

  Vec2 operator/(float scalar)
  {
    return {x / scalar, y / scalar};
  }

  Vec2 operator-(Vec2 other)
  {
    return {x - other.x, y - other.y};
  }
};

struct IVec2
{
  int x;
  int y;

  IVec2 operator-(IVec2 other)
  {
    return {x - other.x, y - other.y};
  }
};

Vec2 vec_2(IVec2 v)
{
  return Vec2{(float)v.x, (float)v.y};
}

Vec2 lerp(Vec2 a, Vec2 b, float t)
{
  Vec2 result;
  result.x = lerp(a.x, b.x, t);
  result.y = lerp(a.y, b.y, t);
  return result;
}

IVec2 lerp(IVec2 a, IVec2 b, float t)
{
  IVec2 result;
  result.x = (int)floorf(lerp((float)a.x, (float)b.x, t));
  result.y = (int)floorf(lerp((float)a.y, (float)b.y, t));
  return result;
}

struct Vec4
{
  union
  {
    float values[4];
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };

    struct
    {
      float r;
      float g;
      float b;
      float a;
    };
  };

  float& operator[](int idx)
  {
    return values[idx];
  }
};

struct Mat4
{
  union
  {
    Vec4 values[4];
    struct 
    {
      float ax;
      float bx;
      float cx;
      float dx;

      float ay;
      float by;
      float cy;
      float dy;

      float az;
      float bz;
      float cz;
      float dz;

      float aw;
      float bw;
      float cw;
      float dw;
    };    
  };

  Vec4& operator[](int col)
  {
    return values[col];
  }
};

Mat4 orthographic_projection(float left, float right, float top, float bottom)
{
  Mat4 result = {};
  result.aw = -(right + left) / (right - left);
  result.bw = (top + bottom) / (top - bottom);
  result.cw = 0.0f; // near plane
  result[0][0] = 2.0f / (right - left);
  result[1][1] = 2.0f / (top - bottom);
  result[2][2] = 1.0f; // far plane
  result[3][3] = 1.0f;

  return result;
}