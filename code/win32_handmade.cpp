#include <Windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable BITMAPINFO BitmapInfo;
global_variable HBITMAP BitmapHandle;
global_variable HDC DeviceContext;
global_variable void* BitmapMemroy;

// DIB stand for Device Independent Bitmap
static void Win32ResizeDIBSection(int Width, int Height)
{
    // TODO(Princerin): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    if (BitmapHandle)
    {
        DeleteObject(BitmapHandle);
    }
    
    if (!DeviceContext)
    {
        DeviceContext = CreateCompatibleDC(0);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    

    BitmapHandle = CreateDIBSection(DeviceContext, &BitmapInfo, DIB_RGB_COLORS, &BitmapMemroy, 0, 0);
}

static void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	StretchDIBits(DeviceContext,
		X, Y, Width, Height,
		X, Y, Width, Height,
        BitmapMemroy,
        &BitmapInfo,
        0, 0);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParame, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_SIZE:
    {
        RECT ClientRect;

        GetClientRect(Window, &ClientRect);

        int Width = ClientRect.right - ClientRect.left;
        int Height = ClientRect.bottom - ClientRect.top;
        Win32ResizeDIBSection(Width, Height);
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

        Win32UpdateWindow(DeviceContext, X, Y, Width, Height);

        EndPaint(Window, &Paint);
    }

        break;

    case WM_CLOSE:
        DestroyWindow(Window);
        break;

	case WM_DESTROY:
		PostQuitMessage(0);
        break;

    default:
        Result =  DefWindowProc(Window, Message, WParame, LParam);
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
            }
        }
    }
    else
    {
        // TODO(Princerin): Logging
    }

    return 0;
}

