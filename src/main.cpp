#include "breaknotes_lib.h"
#include "platform.h"

#ifdef _WIN32 // check if we are on windows
#include "win32_platform.cpp"

#endif


int main()
{
    platform_create_window(1200, 720, "Breaknote");

    while (running)
    {
        //Upate
        platform_update_window();


    return 0;
}