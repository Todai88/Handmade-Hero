// Day 005 38:18

#include <windows.h> 
#include <stdint.h>

#define local_persist static
#define global_variable static
#define internal static

typedef uint8_t uint8; 
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8; 
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer {
    BITMAPINFO Info;
    void *Memory; 
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension {
    int Width;
    int Height;
};

win32_window_dimension
GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
	uint8 *Row = (uint8 *)Buffer.Memory;

    for(int WindowRow = 0; WindowRow < Buffer.Height; ++WindowRow)
    {
        uint32 *Pixel = (uint32 *)Row;

        for(int WindowCol = 0; WindowCol < Buffer.Width; ++WindowCol)
        {
			uint8 Blue  = (WindowCol + XOffset);
            uint8 Green = (WindowRow + YOffset);
            
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer.Pitch;   
    }
}
internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{ 
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB; 
    
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize,
                                MEM_COMMIT, PAGE_READWRITE);
    // RenderWeirdGradient(0, 0);
    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferToWindow(HDC DeviceContext, RECT ClientRect,
                                        win32_offscreen_buffer Buffer,
                                        int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect.right - ClientRect.left;
    int WindowHeight = ClientRect.bottom - ClientRect.top;

    StretchDIBits(
        DeviceContext, 
        /*
        X, Y, Width, Height,
        X, Y, Width, Height,
        */
        0, 0, Buffer.Width, Buffer.Height,
        0, 0, WindowWidth, WindowHeight,
        Buffer.Memory,
        &Buffer.Info,
        DIB_RGB_COLORS,
        SRCCOPY);
}
LRESULT CALLBACK MainWindowCallback(HWND Window,
                   UINT Message,
                   WPARAM WParam,
                   LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    { 
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Height = ClientRect.bottom - ClientRect.top;
            int Width = ClientRect.right - ClientRect.left;   
            Win32ResizeDIBSection(&GlobalBackBuffer, Width, Height);
            OutputDebugStringA("WM_SIZE\n");
            break;  
        }

        case WM_DESTROY:
        {
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
            break;
        }

        case WM_CLOSE:
        {
            Running = false;
            PostQuitMessage(0);
            OutputDebugStringA("WM_CLOSE\n");
            break;
        }
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            OutputDebugStringA("WM_PAINT\n");
            HDC DeviceContext =  BeginPaint(Window, &Paint); 

            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left; 

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            Win32DisplayBufferToWindow(DeviceContext, ClientRect, GlobalBackBuffer, X, Y, Width, Height);
            EndPaint(Window, &Paint); 
            break;
        }
        default: 
        { 
            Result = DefWindowProc(Window, Message, WParam, LParam);
            break;
        }
    }   
    return(Result);
}
int CALLBACK WinMain(
                    HINSTANCE Instance,
                    HINSTANCE PrevInstance,
                    LPSTR CommandLine,
                    int ShowCode
)
{
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroClass";
    if (RegisterClass(&WindowClass))
    {
                // RegisterClass works.
        HWND Window = CreateWindowEx(
                            0,
                            WindowClass.lpszClassName,
                            "Handmade Hero",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            0,
                            0,
                            Instance,
                            0);
        if (Window)
        {
            int XOffset = 0;
            int YOffset = 0;
            Running = true;

            while(Running)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
                
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32DisplayBufferToWindow(DeviceContext, ClientRect, GlobalBackBuffer, 0, 0, WindowWidth, WindowHeight);

                ReleaseDC(Window, DeviceContext);
                ++XOffset;
                YOffset += 2;
            }
        }
        else 
        {
            // TODO: Register Logging
        }
    }
    else 
    {
        // TODO: Register Logging
    }
    return(0);
}
