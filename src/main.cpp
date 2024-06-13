#include "breaknotes_lib.h"


// ###########################################################
//                      Platform Globals
// ###########################################################

static bool running = true;

// ###########################################################
//                     Platform Functions                        
// ###########################################################

bool platform_create_window(int width, int height, const char* title);
void platform_update_window();

// ###########################################################
//                     Windows Platform
// ###########################################################

#ifdef _WIN32 // check if we are on windows
#define WIN32_LEAN_AND_MEAN // remove unneeded windows stuff
#define NOMINMAX // remove min and max macros
#include <Windows.h>

// ###########################################################
//                     Windows Globals
// ###########################################################

static HWND window;

// ###########################################################
//                     Platform Implementation
// ###########################################################
LRESULT CALLBACK windows_window_callback(HWND window, UINT message,
                                         WPARAM w_param, LPARAM l_param)
{
  LRESULT result = 0;

  switch(message)
  {
    case WM_CLOSE: // if the window is closed
    case WM_DESTROY:
    {
      running = false; 
      break;
    }
       

    default:
        {
            result = DefWindowProcA(window, message, w_param, l_param);
        } 
    }   

    return result;
}


bool platform_create_window(int width, int height, const char* title)
{
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASSA window_class = {};
    window_class.hInstance = instance; // handle to the instance that contains the window procedure for the class
    window_class.hIcon = LoadIcon(instance, IDI_APPLICATION); // handle to the class icon
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW); // Decides the lok of the cursor
    window_class.lpszClassName = title; // Unique identifier for the class, which can be the title of the window
    window_class.lpfnWndProc = windows_window_callback; // Callback function that processes messages sent to the window

    if(!RegisterClassA(&window_class))
    {
        return false;
    }


    // WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX  | WS_MAXIMIZEBOX
    int dwStyle = WS_OVERLAPPEDWINDOW; // Window style
    
    // Fake Window initializing OpenGL
    window = CreateWindowExA(  0 // Extended window style
                                    ,title // This references references lpszClassName from window_class
                                    ,title // Window title
                                    ,dwStyle // Window style
                                    ,100 // X position
                                    ,100 // Y position
                                    ,width // Width
                                    ,height // Height
                                    ,NULL // Handle to the parent window
                                    ,NULL // Handle to the menu
                                    ,instance // Handle to the instance
                                    ,NULL // Pointer to the window creation data
                                    );
    if (!window) // If the window was not created
    {

        return false;
    }

    ShowWindow(window, SW_SHOW); // Show the window

    return true;
}

void platform_update_window()
{
    MSG message; // create a message object
    while (PeekMessageA(&message, window, 0, 0, PM_REMOVE)) // Peek for a message in the message queue and remove it 
    {
        TranslateMessage(&message); // Translate the message
        DispatchMessageA(&message); // Calls the callback specified in the WNDCLASSA struct
    }
}

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