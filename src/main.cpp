#include "breaknotes_lib.h"
#include "input.h"
#include "game.cpp"
#include "platform.h"


// Get the function pointer for the OpenGL function
#if defined(_WIN32) || defined(_WIN64)
    #include "win32_platform.cpp"
    #ifndef APIENTRY
        #define APIENTRY __stdcall
    #endif
#else
    #define APIENTRY
#endif

#ifndef GL_GLEXT_PROTOTYPES
    #define GL_GLEXT_PROTOTYPES
#endif

#include "../third_party/glcorearb.h"
#include "gl_renderer.cpp"



int main()
{
    BumpAllocator transientStorage = make_bump_allocator(MB(50));

    platform_create_window(1200, 720, "BreakNotes");
    input.screenSizeX = 1200;
    input.screenSizeY = 720;

    gl_init(&transientStorage);

    while(running)
    {
        // Update
        platform_update_window();
        // update_game();
        gl_render();
        platform_swap_buffers();
    }

    return 0;
}

