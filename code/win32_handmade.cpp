#include <Windows.h>
#include <cstdint>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemroy;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;
global_variable int WindowWidth;
global_variable int WindowHeight;

void GetClientWindowDimension(HWND Window, int& WindowWidth, int& WindowHeight)
{
	RECT WindowRect;
	GetClientRect(Window, &WindowRect);

	WindowWidth = WindowRect.right - WindowRect.left;
	WindowHeight = WindowRect.bottom - WindowRect.top;
}

internal void RenderWeirdGradient(int XOffset, int YOffset)
{
    int Pitch = BitmapWidth * BytesPerPixel;

    uint8* Row = (uint8*)BitmapMemroy;

    for (int Y = 0; Y < BitmapHeight; Y++)
    {
        uint32* Pixel = (uint32*)Row;

        for (int X = 0; X < BitmapWidth; X++)
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

        Row += Pitch;
    }
}

// DIB stand for Device Independent Bitmap
static void Win32ResizeDIBSection(int Width, int Height)
{
    // TODO(Princerin): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    if (BitmapMemroy)
    {
        // TODO(Princerin): VirtualProtect()
        VirtualFree(BitmapMemroy, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    // For uncompressed RGB bitmaps, if biHeight is positive, 
    // the bitmap is a bottom-up DIB with the origin at the 
    // lower left corner. If biHeight is negative, the bitmap 
    // is a top-down DIB with the origin at the upper left corner.
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    // We only need 24bits, 32bits for alignment purpose.
    // In this case we can access each pixel at 4bytes boundary.
    BitmapInfo.bmiHeader.biBitCount = 32;           
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    // NOTE(Princerin): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (Width * Height) * BytesPerPixel;
    BitmapMemroy = VirtualAlloc(nullptr, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    RenderWeirdGradient(0, 0);
}

static void Win32ResizeDIBSection(HWND Window)
{
	Win32ResizeDIBSection(WindowWidth, WindowHeight);
}

static void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int WindowWidth, int WindowHeight)
{
	StretchDIBits(DeviceContext,
		//X, Y, Width, Height,
		//X, Y, Width, Height,
        0, 0, BitmapWidth, BitmapHeight,
        0, 0, WindowWidth, WindowHeight,
        BitmapMemroy,
        &BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_CREATE:

        GetClientWindowDimension(Window, WindowWidth, WindowHeight);

        Win32ResizeDIBSection(Window);
        break;
    case WM_SIZE:
    {
        GetClientWindowDimension(Window, WindowWidth, WindowHeight);
        Win32ResizeDIBSection(Window);
    }
        break;
        
    case WM_ACTIVATEAPP:
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT Paint;

        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

        Win32UpdateWindow(DeviceContext, X, Y, WindowWidth, WindowHeight);

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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    WNDCLASS WindowClass {};

    // TODO(Princerin): Check if HDREDRAW/VREDRAW/OWNDC still matter.
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = L"HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass))
    {
        HWND WindowHandle = CreateWindowEx(0,                           // DWORD dwExStyle
                            WindowClass.lpszClassName,                  // LPCSTR lpClassName
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
                    RenderWeirdGradient(XOffset, YOffset);

                    HDC DeviceContext = GetDC(WindowHandle);

					int WindowWidth = 0;
					int WindowHeight = 0;

                    GetClientWindowDimension(WindowHandle, WindowWidth, WindowHeight);

                    Win32UpdateWindow(DeviceContext, 0, 0, WindowWidth, WindowHeight);
                    ReleaseDC(WindowHandle, DeviceContext);
                    XOffset++;
                    YOffset++;
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

