#include "breaknotes_lib.h"
#include "input.h"
#include "game.h"
// #include "sound.h"
// # include "ui.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES
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

#include "gl_renderer.h"

int main()
{
    platform_create_window(1200, 720, "Breaknote");

    while (running)
    {
        //Upate
        platform_update_window();


    return 0;
}
