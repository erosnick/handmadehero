#include "../project/resource.h"
#include <Windows.h>
#include <cstdint>
#include <ctime>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

global_variable bool Running = true;

struct Win32WindowDimension
{
    int Width;
    int Height;
};

Win32WindowDimension WindowDimension;

Win32WindowDimension GetWindowDimension(HWND Window)
{
	RECT WindowRect;
	GetClientRect(Window, &WindowRect);

    Win32WindowDimension WindowDimension {};

	if (WindowRect.right != 0 && WindowRect.bottom != 0)
	{
        WindowDimension.Width = WindowRect.right - WindowRect.left;
        WindowDimension.Height = WindowRect.bottom - WindowRect.top;
	}

    return WindowDimension;
}

struct Win32OffScreenBuffer
{
    BITMAPINFO Info;
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
};

Win32OffScreenBuffer GlobalBackBuffer;

internal void RenderWeirdGradient(Win32OffScreenBuffer* GlobalBackBuffer, int XOffset, int YOffset)
{
    uint8* Row = (uint8*)GlobalBackBuffer->Memory;

    for (int Y = 0; Y < GlobalBackBuffer->Height; Y++)
    {
        uint32* Pixel = (uint32*)Row;

        for (int X = 0; X < GlobalBackBuffer->Width; X++)
        {
            // Pixel in memory: BB GG RR XX
            // Little Endian Architecture 
            // 0xRRGGBBXX
            //*Pixel = (255 << 16) + (255 << 8) + 255;
            //*Pixel = ((uint8)X << 16) + ((uint8)Y << 8);
            *Pixel = ((uint8)(X + XOffset) << 8) | ((uint8)(Y + YOffset));
            //*Pixel = (255 << 16);   // 255 0 0 0
            //*Pixel = 255;   // 0 0 255 0

            Pixel++;
        }

        Row += GlobalBackBuffer->Pitch;
    }
}

internal void PilotPixel(int X, int Y, int Color)
{
    ((uint32*)GlobalBackBuffer.Memory)[X + Y * GlobalBackBuffer.Width] = Color;
}

void CleanBitmap(uint8 Red = 255, uint8 Green = 255, uint8 Blue = 255)
{
    uint8* Row = (uint8*)GlobalBackBuffer.Memory;

    for (int Y = 0; Y < GlobalBackBuffer.Height; Y++)
    {
        uint32* Pixel = (uint32*)Row;

        for (int X = 0; X < GlobalBackBuffer.Width; X++)
        {
            *Pixel = (Red << 16) | (Green << 8) | Blue;

            Pixel++;
        }

        Row += GlobalBackBuffer.Pitch;
    }
}

// DIB stand for Device Independent Bitmap
static void Win32ResizeDIBSection(int Width, int Height)
{
    // TODO(Princerin): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    if (GlobalBackBuffer.Memory)
    {
        // TODO(Princerin): VirtualProtect()
        VirtualFree(GlobalBackBuffer.Memory, 0, MEM_RELEASE);
    }

    GlobalBackBuffer.Width = Width;
    GlobalBackBuffer.Height = Height;

    GlobalBackBuffer.Info.bmiHeader.biSize = sizeof(GlobalBackBuffer.Info.bmiHeader);
    GlobalBackBuffer.Info.bmiHeader.biWidth = GlobalBackBuffer.Width;
    // For uncompressed RGB bitmaps, if biHeight is positive, 
    // the bitmap is a bottom-up DIB with the origin at the 
    // lower left corner. If biHeight is negative, the bitmap 
    // is a top-down DIB with the origin at the upper left corner.
    GlobalBackBuffer.Info.bmiHeader.biHeight = -GlobalBackBuffer.Height;
    GlobalBackBuffer.Info.bmiHeader.biPlanes = 1;
    // We only need 24bits, 32bits for alignment purpose.
    // In this case we can access each pixel at 4bytes boundary.
    GlobalBackBuffer.Info.bmiHeader.biBitCount = 32;
    GlobalBackBuffer.Info.bmiHeader.biCompression = BI_RGB;

    GlobalBackBuffer.Pitch = Width * GlobalBackBuffer.BytesPerPixel;

    // NOTE(Princerin): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (Width * Height) * GlobalBackBuffer.BytesPerPixel;

    // VirtualAlloc always allocate multiples of 4k(4096byte).
    GlobalBackBuffer.Memory = VirtualAlloc(nullptr, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    RenderWeirdGradient(&GlobalBackBuffer, 0, 0);
}

static void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int WindowWidth, int WindowHeight)
{
    // TODO(Princerin): Aspect ratio correction.
	StretchDIBits(DeviceContext,
		//X, Y, Width, Height,
		//X, Y, Width, Height,
        0, 0, WindowWidth, WindowHeight,
        0, 0, GlobalBackBuffer.Width, GlobalBackBuffer.Height,
        GlobalBackBuffer.Memory,
        &GlobalBackBuffer.Info,
        DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_CREATE:
    {
        WindowDimension = GetWindowDimension(Window);
        Win32ResizeDIBSection(WindowDimension.Width, WindowDimension.Height);
    }
        break;

    case WM_SIZE:
    {
        WindowDimension = GetWindowDimension(Window);
        //Win32ResizeDIBSection(WindowDimension.Width, WindowDimension.Height);
    }
        break;
        
    case WM_ACTIVATEAPP:
        // WParam - TRUE for activated, FALSE for deactivated
        if (WParam)
        {
            OutputDebugString(L"Activated\n");
        }
        else
        {
            OutputDebugString(L"Deactivated\n");
        }
  
        Running = WParam;
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT Paint;

        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

		Win32UpdateWindow(DeviceContext, X, Y, WindowDimension.Width, WindowDimension.Height);

        EndPaint(Window, &Paint);
    }

        break;

    case WM_KEYDOWN:
        switch (WParam)
        {
        case VK_ESCAPE:
            DestroyWindow(Window);
            break;
        default:
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(Window);
        break;

	case WM_DESTROY:
		PostQuitMessage(0);
        break;

    default:
        Result =  DefWindowProc(Window, Message, WParam, LParam);
        break;
    }

    return Result;
}

void Render()
{

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    WNDCLASSEX WindowClassEx {};

    // TODO(Princerin): Check if HDREDRAW/VREDRAW/OWNDC still matter.
    WindowClassEx.cbSize = sizeof(WNDCLASSEX);
    WindowClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; WS_OVERLAPPEDWINDOW;
    WindowClassEx.lpfnWndProc = Win32MainWindowCallback;
    WindowClassEx.hInstance = hInstance;
    WindowClassEx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HANDMADEHERO));
    WindowClassEx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HANDMADEHERO));
    WindowClassEx.lpszClassName = L"HandmadeHeroWindowClass";

    if (RegisterClassEx(&WindowClassEx))
    {
        HWND WindowHandle = CreateWindowEx(0,                           // DWORD dwExStyle
                            WindowClassEx.lpszClassName,                  // LPCSTR lpClassName
                            L"Handmade Hero",                           // LPCSTR lpWindowName
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,           // DWORD dwStyle
                            CW_USEDEFAULT,                              // int X
                            CW_USEDEFAULT,                              // int Y
                            1280,                                       // int nWidth
                            720,                                        // int nHeight
                            0,                                          // HWND hWndParent
                            0,                                          // HMENU hMenu
                            hInstance,                                  // HWND hInstance
                            nullptr);                                   // LPVOID

        if (WindowHandle)
        {
            MSG Message {};

			int XOffset = 0;
			int YOffset = 0;

            srand(time(nullptr));

            while (Message.message != WM_QUIT)
            {
                // Pass NULL instead of the window - handle to PeekMessage / GetMessage.
                // WM_QUIT does not belong to a window, and is also sent after the window 
                // is destroyed, so the window handle will be invalid then.You usually 
                // want to handle all messages in the thread message queue, not just 
                // those associated with a specific window.
                BOOL MessageResult = PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE);

                if (MessageResult)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    if (Running)
                    {
						RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

						HDC DeviceContext = GetDC(WindowHandle);

						//for (int i = 0; i < 10000; i++)
						//{
						//	int X = rand() % WindowDimension.Width;
						//	int Y = rand() % WindowDimension.Height;

						//	int Color = ((rand() % 256) << 16) | ((rand() % 256) << 8) | ((rand() % 256));

						//	PilotPixel(X, Y, Color);
						//}

						Win32UpdateWindow(DeviceContext, 0, 0, WindowDimension.Width, WindowDimension.Height);

						CleanBitmap(100, 149, 237);

						XOffset++;
						YOffset++;
                    }
                }
            }
        }
    }
    else
    {
        // TODO(Princerin): Logging
    }

    return 0;
}

