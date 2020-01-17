#include "../project/resource.h"
#include <Windows.h>
#include <cstdint>
#include <ctime>
#include <Xinput.h>
#include "Timer.h"

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
global_variable int XOffset;
global_variable int YOffset;
global_variable Timer InputTimer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)

typedef X_INPUT_GET_STATE(FPXInputGetState);
typedef X_INPUT_SET_STATE(FPXInputSetState);

X_INPUT_GET_STATE(XInputGetStateStub)
{
    return 0;
}

X_INPUT_SET_STATE(XInputSetStateStub)
{
    return 0;
}

global_variable FPXInputGetState* XInputGetStateDLL = XInputGetStateStub;
global_variable FPXInputSetState* XInputSetStateDLL = XInputSetStateStub;

#define XInputGetState XInputGetStateDLL
#define XInputSetState XInputSetStateDLL

struct Win32WindowInfo
{
    int X;
    int Y;
    int Width;
    int Height;
};

Win32WindowInfo WindowInfo;

Win32WindowInfo GetWindowInfo(HWND Window)
{
	RECT WindowRect;
	GetClientRect(Window, &WindowRect);

    Win32WindowInfo WindowInfo {};

    if (WindowRect.left != 0 && WindowRect.top != 0)
    {
		WindowInfo.X = WindowRect.left;
		WindowInfo.Y = WindowRect.top;
    }

	if (WindowRect.right != 0 && WindowRect.bottom != 0)
	{
        WindowInfo.Width = WindowRect.right - WindowRect.left;
        WindowInfo.Height = WindowRect.bottom - WindowRect.top;
	}

    return WindowInfo;
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

void RenderNoises(int Count)
{
	for (int i = 0; i < Count; i++)
	{
		int X = rand() % GlobalBackBuffer.Width;
		int Y = rand() % GlobalBackBuffer.Height;

		int Color = ((rand() % 256) << 16) | ((rand() % 256) << 8) | ((rand() % 256));

		PilotPixel(X, Y, Color);
	}
}

void RenderPureColorUInt8(uint8 Red, uint8 Green, uint8 Blue)
{
    uint8* Row = (uint8*)GlobalBackBuffer.Memory;

    for (int Y = 0; Y < GlobalBackBuffer.Height; Y++)
    {
        uint8* Pixel = Row;

        for (int X = 0; X < GlobalBackBuffer.Width; X++)
        {
            // Pixel in memory: BB GG RR XX
            // Little Endian Architecture 
            // 0xRRGGBBXX
			Pixel[0] = Blue;    // BB
			Pixel[1] = Green;   // GG
			Pixel[2] = Red;     // RR
			Pixel[3] = 0;       // XX
            Pixel += 4;

			//*Pixel = Red;
			//Pixel++;

			//*Pixel = Green;
			//Pixel++;

			//*Pixel = Blue;
			//Pixel++;

			//*Pixel = 0;
   //         Pixel++;
        }

        Row += GlobalBackBuffer.Pitch;
    }
}

void RenderPureColorUInt32(uint8 Red, uint8 Green, uint8 Blue)
{
	uint8* Row = (uint8*)GlobalBackBuffer.Memory;

	for (int Y = 0; Y < GlobalBackBuffer.Height; Y++)
	{
		uint32* Pixel = (uint32*)Row;

		for (int X = 0; X < GlobalBackBuffer.Width; X++)
		{
			// Pixel in memory: BB GG RR XX
			// Little Endian Architecture 
			// 0xXXRRGGBB
            // (Value << 24)就是XX的值
            uint32 Color = (Red << 16) + (Green << 8) + Blue;
			*Pixel = Color;

			Pixel++;
		}

		Row += GlobalBackBuffer.Pitch;
	}
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
internal void Win32ResizeDIBSection()
{
    // TODO(Princerin): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    if (GlobalBackBuffer.Memory)
    {
        // TODO(Princerin): VirtualProtect()
        VirtualFree(GlobalBackBuffer.Memory, 0, MEM_RELEASE);
    }

    GlobalBackBuffer.Width = WindowInfo.Width;
    GlobalBackBuffer.Height = WindowInfo.Height;

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

    GlobalBackBuffer.Pitch = WindowInfo.Width * GlobalBackBuffer.BytesPerPixel;

    // NOTE(Princerin): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (WindowInfo.Width * WindowInfo.Height) * GlobalBackBuffer.BytesPerPixel;

    // VirtualAlloc always allocate multiples of 4k(4096byte).
    GlobalBackBuffer.Memory = VirtualAlloc(nullptr, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

static void Win32UpdateWindow(HDC DeviceContext)
{
    // TODO(Princerin): Aspect ratio correction.
	StretchDIBits(DeviceContext,
		//X, Y, Width, Height,
		//X, Y, Width, Height,
        WindowInfo.X, WindowInfo.Y, WindowInfo.Width, WindowInfo.Height,
        0, 0, GlobalBackBuffer.Width, GlobalBackBuffer.Height,
        GlobalBackBuffer.Memory,
        &GlobalBackBuffer.Info,
        DIB_RGB_COLORS, SRCCOPY);
}

internal void Win32LoadXInput()
{
   HMODULE XInputLibrary = LoadLibrary(XINPUT_DLL_W);

   if (XInputLibrary)
   {
       XInputGetState = (FPXInputGetState*)GetProcAddress(XInputLibrary, "XInputGetState");
       XInputSetState = (FPXInputSetState*)GetProcAddress(XInputLibrary, "XInputSetState");
   }
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_CREATE:
    {
        WindowInfo = GetWindowInfo(Window);
        Win32ResizeDIBSection();
    }
        break;

    case WM_SIZE:
    {
        WindowInfo = GetWindowInfo(Window);
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

        WindowInfo = GetWindowInfo(Window);

		Win32UpdateWindow(DeviceContext);

        EndPaint(Window, &Paint);
    }

        break;

    case WM_KEYUP:
    case WM_KEYDOWN:
    {
        bool WasDown = ((LParam & (1 << 30)) != 0);
        bool IsDown = ((LParam & (1 << 31)) != 0);

        switch (WParam)
        {
        case VK_ESCAPE:
            DestroyWindow(Window);
            break;

        case 'W':
        case VK_UP:
        {

            YOffset++;
        }

        break;

        case 'S':
        case VK_DOWN:
            YOffset--;
            break;

        case 'A':
        case VK_LEFT:
            XOffset++;
            break;

        case 'D':
        case VK_RIGHT:
            XOffset--;
            break;

        default:
            break;
        }
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

void ProccessInput()
{
	for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++)
	{
		XINPUT_STATE ControllerState;
		DWORD Success = XInputGetState(ControllerIndex, &ControllerState);
		if (Success == ERROR_SUCCESS)
		{
			// NOTE(Princerin): This controller is plugged in.
			// TODO(Princerin): See if ControllerState.dwPacketNumber increments.
			XINPUT_GAMEPAD GamePad = ControllerState.Gamepad;

            bool AButton = GamePad.wButtons & XINPUT_GAMEPAD_A;

			if (AButton)
			{
                InputTimer.Stop();

                double ElapsedTime = InputTimer.GetElapsedTime();

                if (ElapsedTime > 100.0)
                {
                    OutputDebugString(L"A button pressed.\n");
                }

                InputTimer.Start();
			}

            bool Up = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
            bool Down = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
            bool Left = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
            bool Right = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
            int ThumbLeftX = GamePad.sThumbLX;
            int ThumbLeftY = GamePad.sThumbLY;

            if (Left || ThumbLeftX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                XOffset++;
            }

            if (Right || ThumbLeftX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                XOffset--;
            }

            if (Up || ThumbLeftY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                YOffset++;
            }

            if (Down || ThumbLeftY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                YOffset--;
            }

            XINPUT_VIBRATION Vibration;
            Vibration.wLeftMotorSpeed = 10000;
            Vibration.wRightMotorSpeed = 10000;
            XInputSetState(0, &Vibration);
		}
		else
		{
			// NOTE(Princerin): The controller is not available.
		}
	}
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

            srand(time(nullptr));

            Timer GameTimer;
            GameTimer.Start();

            double RealTime = GameTimer.Now();
            double SimulationTime = 0.0;

            HDC DeviceContext = GetDC(GetActiveWindow());

            Win32LoadXInput();

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
                        double Now = GameTimer.Now();

                        double GameTime = Now - RealTime;

                        while (SimulationTime < GameTime)
                        {
                            SimulationTime += 16;

						    RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);
						    //RenderNoises(10000);
                            //RenderPureColorUInt8(255, 0, 0);
                            //RenderPureColorUInt32(255, 0, 0);

                            ProccessInput();
                        }

						Win32UpdateWindow(DeviceContext);

						//CleanBitmap(100, 149, 237);

                        GameTimer.Update();

                        double ElapsedTime = GameTimer.GetElapsedTime();
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

