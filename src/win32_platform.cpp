
#include "platform.h"
#include "breaknotes_lib.h"
#define WIN32_LEAN_AND_MEAN // remove unneeded windows stuff
#define NOMINMAX // remove min and max macros

#include <windows.h>
#include <xaudio2.h>
#include "wglext.h"
#include "input.h"
#include <glcorearb.h>

// #############################################################################
//                           Windows Structures
// #############################################################################

// struct xAudioVoice : IXAudio2VoiceCallback
// {
// 	IXAudio2SourceVoice* voice;
//     SoundOptions options;
//     float fadeTimer;
//     char* soundPath;

//     int playing;

//     void OnStreamEnd() noexcept
//     {
//         voice->Stop(); // Stop the voice
//         playing = false;
//     }

//     void OnBufferStart(void * pBufferContext) noexcept
//     {
//         playing = true;
//     }

//     void OnVoiceProcessingPassEnd() noexcept {}
//     void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept {}
//     void OnBufferEnd(void * pBufferContext) noexcept {}
//     void OnVoiceError(void *pBufferContext, HRESULT Error) noexcept {} // Function to take in a buffer context and an error code and return nothing
// }

// ###########################################################
//                     Windows Globals
// ###########################################################

static HWND window;
static HDC dc;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT_ptr; // Swap Interval function pointer
// static xAudioVoice voiceArr[MAX_CONCURRENT_SOUNDS]; // Array of xAudioVoice objects


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
    
    case WM_SIZE:
    {
        RECT rect = {}; // Create a rectangle object
        GetClientRect(window, &rect); // Get the client area of the window
        input->screenSize.x = rect.right - rect.left; // Set the screen width
        input->screenSize.y = rect.bottom - rect.top; // Set the screen height

        break;
    }

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        bool isDown = (message == WM_KEYDOWN) || (message == WM_SYSKEYDOWN) || (message == WM_LBUTTONDOWN);
        
        KeyCodeID keycode = KeyCodeLookupTable[w_param];
        Key* key = &input->keys[keycode];
        key->justPressed = !key->justPressed && !key->isDown && isDown; // If the key was not just pressed and is not down and is down then assign true to justPressed
        key->justReleased = !key->justReleased && key->isDown && !isDown; // If the key was not just released and is down and is not down then assign true to justReleased
        key->isDown = isDown; // Assign isDown to the key
        key->halfTransitionCount++; // Increment the half transition count

        break;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
        bool isDown = (message == WM_LBUTTONDOWN) || (message == WM_RBUTTONDOWN) || (message == WM_MBUTTONDOWN);

        int mouseCode = (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) ? VK_LBUTTON : (message == WM_MBUTTONDOWN || message == WM_MBUTTONUP) ? VK_MBUTTON : VK_RBUTTON; // If the message is left button down or up then assign VK_LBUTTON to mouseCode, if the message is middle button down or up then assign VK_MBUTTON to mouseCode, if the message is right button down or up then assign VK_RBUTTON to mouseCode
        KeyCodeID keycode = KeyCodeLookupTable[mouseCode]; // Get the keycode from the lookup table
        Key* key = &input->keys[keycode]; // Get the key from the input keys array
        key->justPressed = !key->justPressed && !key->isDown && isDown; // If the key was not just pressed and is not down and is down then assign true to justPressed
        key->justReleased = !key->justReleased && key->isDown && !isDown; // If the key was not just released and is down and is not down then assign true to justReleased
        key->isDown = isDown; // Assign isDown to the key
        key->halfTransitionCount++; // Increment the half transition count

        break;
    }
    
    default:
        {
            result = DefWindowProcA(window, message, w_param, l_param);
        } 
    }   

    return result;
}


bool platform_create_window(int width, int height, char* title)
{
    HINSTANCE instance = GetModuleHandleA(0); // Module handle is a unique identifier for the executable file that the current process is running

    WNDCLASSA window_class = {};
    window_class.hInstance = instance; // handle to the instance that contains the window procedure for the class
    window_class.hIcon = LoadIcon(instance, IDI_APPLICATION); // handle to the class icon
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW); // Decides the lok of the cursor
    window_class.lpszClassName = title; // Unique identifier for the class, which can be the title of the window
    window_class.lpfnWndProc = windows_window_callback; // Callback function that processes messages sent to the window

    if(!RegisterClassA(&window_class)) // Register the window class - means that the created window class is registered with the system
    {
        return false;
    }


    // WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX  | WS_MAXIMIZEBOX
    int dwStyle = WS_OVERLAPPEDWINDOW; // Window style

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr; // Function pointer for choosing pixel format
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr; // Function pointer for creating context attributes
    

    // Fake Window initializing OpenGL
    // Fake window is needed to get the function pointers for OpenGL
    // after the device and window is destroyed
    {
        window = CreateWindowExA(   0 // Extended window style
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
            SM_ASSERT(false, "Failed to create window"); // Assert that the window was not created

            return false;
        }
        HDC fakeDC = GetDC(window); // Get the device context of the window
        if(!fakeDC)
        {
            SM_ASSERT(false, "Failed to get HDC");
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0}; // Create a pixel format descriptor
        pfd.nSize = sizeof(pfd); // Size of the pixel format descriptor
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // Flags for the pixel format descriptor
        pfd.iPixelType = PFD_TYPE_RGBA; // Pixel type
        pfd.cColorBits = 32; // Color bits
        pfd.cAlphaBits = 8; // Alpha bits
        pfd.cDepthBits = 24; // Depth bits

        int pixelFormat = ChoosePixelFormat(fakeDC, &pfd); // Choose the pixel format
        if(!pixelFormat)
        {
            SM_ASSERT(false, "Failed to choose pixel format");
            return false;
        }

        if(!SetPixelFormat(fakeDC, pixelFormat, &pfd)) // Set the pixel format
        {
            SM_ASSERT(false, "Failed to set pixel format");
            return false;
        }

        // Create a Handle to a fake OpenGL Rendering Context
        HGLRC fakeRC = wglCreateContext(fakeDC); // Create a fake OpenGL rendering context 
        if(!fakeRC)
        {
            SM_ASSERT(false, "Failed to create Render context");
            return false;
        }

        if(!wglMakeCurrent(fakeDC, fakeRC)) // Make the fake OpenGL rendering context current
        {
            SM_ASSERT(false, "Failed to make context current");
            return false;
        }

        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChoosePixelFormatARB"); // Load the wglChoosePixelFormatARB function
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB"); // Load the wglCreateContextAttribsARB function
        wglSwapIntervalEXT_ptr = (PFNWGLSWAPINTERVALEXTPROC)platform_load_gl_function("wglSwapIntervalEXT"); // Load the wglSwapIntervalEXT function

        if(!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
        {
            SM_ASSERT(false, "Failed to load OpenGL functions");
            return false;
        }

        // Clean up the take stuff
        wglMakeCurrent(fakeDC, 0); // Make the fake DC current
        wglDeleteContext(fakeRC); // Delete the fake RC
        ReleaseDC(window, fakeDC); // Release the DC

        // Can't reuse the same Device Context
        // because we already called "SetPixelFormat" on it
        DestroyWindow(window); // Destroy the window
    }   

    // Actual OpenGL initialization
    {
        // Add in the border size of the window
        {
            RECT borderRect = {};
            AdjustWindowRectEx(&borderRect, dwStyle, 0, 0); // Adjust the window rect

            width += borderRect.right - borderRect.left; // Add the border size to the width
            height += borderRect.bottom - borderRect.top; // Add the border size to the height
        }

        window = CreateWindowExA(   0 // Extended window style
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
        if(!window)
        {
            SM_ASSERT(false, "Failed to create Windows window");
            return false;
        }

        dc = GetDC(window); // Get the device context of the window
        if(!dc)
        {
            SM_ASSERT(false, "Failed to get DC");
            return false;
        }

        const int pixelAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            0 // Terminate with 0, otherwise OpenGL will throw an Error!
        };

        UINT numPixelFormat;
        int pixelFormat = 0;
        if(!wglChoosePixelFormatARB(dc, pixelAttribs,
                                    0, 
                                    1,
                                    &pixelFormat,
                                    &numPixelFormat))
        {
            SM_ASSERT(0, "Failed to wglChoosePixelFormatARB");
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        DescribePixelFormat(dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        if(!SetPixelFormat(dc, pixelFormat, &pfd))
        {
            SM_ASSERT(0, "Failed to set pixel format");
            return false;
        }

        const int contextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            0 // Terminate with 0, otherwise OpenGL will throw an Error!
        };

        HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
        if(!rc)
        {
            SM_ASSERT(0, "Failed to create Render Context for OpenGL");
            return false;
        }

        if(!wglMakeCurrent(dc, rc))
        {
            SM_ASSERT(0, "Failed to wglMakeCurrent");
            return false;
        }
    }
    ShowWindow(window, SW_SHOW);
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
