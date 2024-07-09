#include "breaknotes_lib.h"
#include "input.h"
#include "game.h"
// #include "sound.h"
// # include "ui.h"


#include "glcorearb.h"

// #############################################################################
//                           Platform Includes
// #############################################################################
#include "platform.h"

#ifdef _WIN32 // check if we are on windows
#include "win32_platform.cpp"
const char* gameLibName = "game.dll";
const char* gameLoadLibName = "game_load.dll";
#elif defined(__APPLE__)
#include "mac_platform.cpp"
const char* gameLibName = "game.so";
const char* gameLoadLibName = "game_load.so";
#else // Linux
#include "linux_platform.cpp"
const char* gameLibName = "game.so";
const char* gameLoadLibName = "game_load.so";
#endif

#include "gl_renderer.cpp"

// #############################################################################
//                           Game DLL Stuff
// #############################################################################
// This is the function pointer to update_game in game.cpp
// typedef decltype(update_game) update_game_type;
// static update_game_type* update_game_ptr;

// #############################################################################
//                           Cross Platform functions
// #############################################################################
// Used to get Delta Time
// #include <chrono>
// double get_delta_time();
// void reload_game_dll(BumpAllocator* transientStorage);


int main()
{
    // Initialize timestamp
    // get_delta_time();

    BumpAllocator transientStorage = make_bump_allocator(MB(50));
    BumpAllocator persistentStorage = make_bump_allocator(MB(256));

    input = (Input*)bump_alloc(&persistentStorage, sizeof(Input));

    if(!input)
    {
        SM_ERROR("Failed to allocate input struct");
        return -1;
    }

    renderData = (RenderData*)bump_alloc(&persistentStorage, sizeof(RenderData));


    platform_create_window(1200, 720, "Breaknote");
    gl_init(&transientStorage);

    while (running)
    {
        //Upate
        platform_update_window();
        gl_render(&transientStorage);

        platform_swap_buffers();

    return 0;
    }   
}
