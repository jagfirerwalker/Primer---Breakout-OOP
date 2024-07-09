#pragma once

#include <stdio.h>

// This is  to get malloc
#include <stdlib.h>

// This is to get memset
#include <string.h>

// Used to get th edit timestamp of files
#include <sys/stat.h>

#include <math.h>

// #############################################################################
//                           Constants
// #############################################################################
// WAV Files
constexpr int NUM_CHANNELS = 2;
constexpr int SAMPLE_RATE = 44100;

// ###########################################################
//                      Defines
// ###########################################################

#ifdef _WIN32
#  define DEBUG_BREAK() __debugbreak()
#  define EXPORT_FN __declspec(dllexport)
#elif __linux__
#  define DEBUG_BREAK() __builtin_debugtrap()
#  define EXPORT_FN
#elif __APPLE__
#  define DEBUG_BREAK() __builtin_trap()
#  define EXPORT_FN
#endif

#define line_id(index) (size_t)((__LINE__ << 16) | (index)) // Create a unique id for a line of code,which is shifted by 16 bits to the left and then or'd with the index
#define ArraySize(x) (sizeof((x)) / sizeof((x)[0])) // Get the size of an array by dividing the size of the array by the size of the first element of the array

#define b8 char
#define BIT(x) (1 << x) // Shift 1 by x bits to the left
#define KB(x) ((unsigned long long)1024 *x) // KiloBytes 
#define MB(x) ((unsigned long long)1024 * KB(x)) // MegaBytes
#define GB(x) ((unsigned long long)1024 * MB(x)) // GigaBytes


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
void _log(char* prefix, char* message, TextColor textColor, Args... args)
{
    static char* TextColorTable[TEXT_COLOR_COUNT] =
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

    
    char formatBuffer[8192] = {};
    sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, message); // replace the 1st %s with the color code, the 2nd %s with the prefix, and the 3rd %s with the message, then reset the color code

    char textBuffer[8192] = {};
    sprintf(textBuffer,"%s", formatBuffer, args...);

    puts(textBuffer);
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
//                           Array
// #############################################################################

template<typename T, int N>
struct Array
{
    static constexpr int maxElements = N;
    int count = 0;
    T elements[N];

    T& operator[](int ids)
    {
        SM_ASSERT(idx >= 0, "index negative");
        SM_ASSERT(idx < count, "index out of bounds");
        return elements[idx];
    }

    int add(T element)
    {
        SM_ASSERT(count < maxElements, "Array is full");
        elements[count] = element;
        return count++;
    }

    void remove_idx_and_swap(int idx)
    {
        SM_ASSERT(idx >= 0, "index negative");
        SM_ASSERT(idx < count, "index out of bounds");
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

    auto file = fopen(filePath, "rb"); // Open the file in read binary mode
    if(!file) // If the file does not exist
    {
        return false;
    }
    fclose(file); // Close the file

    return true;
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

// #############################################################################
//                           Math stuff
// #############################################################################

// Clamp a value between a min and max as integers
int sign(int x)
{
    return (x > 0)? 1 : -1;
}

// Overload the sign function for floats
// Clamp a value between a min and max float
float sign(float x)
{
    return (x >= 0.0f)? 1.0f : -1.0f;
}

// Geth the minimum of two integers
int min(int a, int b)
{
    return (a < b)? a : b;
}

// overload min function for float
float min(float a, float b)
{
    if (a < b)
    {
        return a;
    }

    return b;
}

int max(int a, int b)
{
    return (a > b)? a : b;
}

// Overload the max function with long long
// Get the maximum of two long
long long max(long long a, long long b)
{
    if (a > b)
    {
        return a;
    }

    return b;
}

// Overload the max function with float
float max(float a, float b)
{
    if (a > b)
    {
        return a;
    }

    return b;
}

// Step function to increase or decrease a value by a certain amount
float approach(float current, float target, float increase)
{
    if(current < target)
    {
        return min(current + increase, target); // ensures that the return is either the target or the current + increase
    }

    return max(current - increase, target); // ensures that the retur is either the target or the current - increase
}

// Linear interpolation
float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

// Struct definition for a 2D vector
struct Vec2
{
    float x;
    float y;

    Vec2 operator/(float scaler)
    {
        return {x / scaler, y / scaler};
    }

    Vec2 operator*(float scaler)
    {
        return {x * scaler, y * scaler};
    }

    Vec2 operator-(Vec2 other)
    {
        return {x - other.x, y - other.y};
    }

    operator bool()
    {
        return x!= 0.0f && y != 0.0f;
    }
};

// Struct definition for a IVec2
struct IVec2
{
    int x;
    int y;

    IVec2 operator-(IVec2 other)
    {
        return {x - other.x, y - other.y};
    }

    IVec2& operator-=(int value)
    {
        x -= value;
        y -= value;
        return *this;
    }

    IVec2& operator+=(int value)
    {
        x += value;
        y += value;
        return *this;
    }

    IVec2 operator/(int scaler)
    {
        return {x / scaler, y / scaler};
    }
};

// Converts a IVec2 to a Vec2
Vec2 vec_2(IVec2 v)
{
    return Vec2{(float)v.x, (float)v.y};
}

// Linear interpolation for a Vec2
Vec2 lerp(Vec2 a, Vec2 b, float t)
{
    Vec2 restult;
    restult.x = lerp(a.x, b.x, t);
    restult.y = lerp(a.y, b.y, t);
    return restult;
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

    bool operator==(Vec4 other)
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
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
    result.cw = 0.0f;
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom); 
    result[2][2] = 1.0f / (1.0f - 0.0f); // Far and Near
    result[3][3] = 1.0f;

    return result;
}

struct Rect
{
    Vec2 pos;
    Vec2 size;
};

struct IRect
{
    IVec2 pos;
    IVec2 size;
};

bool point_in_rect(Vec2 point, Rect rect)
{
    return (point.x >= rect.pos.x&&
            point.x <= rect.pos.x + rect.size.x &&
            point.y >= rect.pos.y &&
            point.y <= rect.pos.y + rect.size.y);
}

bool point_in_rect(Vec2 point, IRect rect)
{
  return (point.x >= rect.pos.x &&
          point.x <= rect.pos.x + rect.size.x &&
          point.y >= rect.pos.y &&
          point.y <= rect.pos.y + rect.size.y);
}

bool point_in_rect(IVec2 point, IRect rect)
{
  return point_in_rect(vec_2(point), rect);
}

bool rect_collision(IRect a, IRect b)
{
    return a.pos.x < b.pos.x  + b.size.x && // Collision on Left of a and right of b
            a.pos.x + a.size.x > b.pos.x  && // Collision on Right of a and left of b
            a.pos.y < b.pos.y  + b.size.y && // Collision on Bottom of a and Top of b
            a.pos.y + a.size.y > b.pos.y;    // Collision on Top of a and Bottom of b
}

//#######################################################################
//                          Normal Colors
//#######################################################################
constexpr Vec4 COLOR_WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr Vec4 COLOR_RED = {1.0f, 0.0f, 0.0f, 1.0f};
constexpr Vec4 COLOR_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
constexpr Vec4 COLOR_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};
constexpr Vec4 COLOR_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
constexpr Vec4 COLOR_BLACK = {0.0f, 0.0f, 0.0f, 1.0};