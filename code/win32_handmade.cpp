/*
TODO(Princerin): THIS IS NOT A FINAL PLATFORM LAYER!!!

- Saved game locations
- Getting a handle to our own executable file
- Asset loading path
- Threading (launch a thread)
- Raw Input (support for multiple keyboards
- Sleep/timeBeginPeriod
- ClipCursor() (for multimonitor support)
- Fullscreen support
- WM_SETCURSOR (Control cursor visibility)
- QueryCancelAutoplay
- WM_ACTIVATEAPP (for when we are not the active application)
- Blit speed improvements (Bitblt)
- Hardware acceleration (OpenGL or Direct3D or BOTH??)
- GetKeyboardLayout (for French keyboards, international WASD support)

Just a partial list of stuff!!
*/

#include "../project/resource.h"
#include <Windows.h>
#include <cstdint>
#include <ctime>
#include <Xinput.h>
#include <xaudio2.h>
#include <dsound.h>
#include <cmath>
#include <cstdio>
#include "Timer.h"

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float real32;
typedef double real64;

const float PI = 3.14159265359f;

#include "win32_handmade.h"
#include "handmade.cpp"

global_variable bool Running = true;
global_variable bool Activate = true;
global_variable int XOffset;
global_variable int YOffset;
global_variable Timer InputTimer;
global_variable LPDIRECTSOUNDBUFFER GlobalPrimaryBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)

typedef X_INPUT_GET_STATE(FPXInputGetState);
typedef X_INPUT_SET_STATE(FPXInputSetState);

X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(FPDirectSoundCreate);

global_variable FPXInputGetState* XInputGetStateDLL = XInputGetStateStub;
global_variable FPXInputSetState* XInputSetStateDLL = XInputSetStateStub;

#define XInputGetState XInputGetStateDLL
#define XInputSetState XInputSetStateDLL

Win32WindowInfo WindowInfo {};

Win32WindowInfo GetWindowInfo(HWND Window)
{
	RECT WindowRect;
	GetClientRect(Window, &WindowRect);

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



Win32OffScreenBuffer GlobalBackBuffer;

//internal void RenderWeirdGradient()
//{
//    uint8* Row = (uint8*)GlobalBackBuffer.Memory;
//
//    for (int Y = 0; Y < GlobalBackBuffer.Height; Y++)
//    {
//        uint32* Pixel = (uint32*)Row;
//
//        for (int X = 0; X < GlobalBackBuffer.Width; X++)
//        {
//            // Pixel in memory: BB GG RR XX
//            // Little Endian Architecture 
//            // 0xRRGGBBXX
//            //*Pixel = (255 << 16) + (255 << 8) + 255;
//            //*Pixel = ((uint8)X << 16) + ((uint8)Y << 8);
//            *Pixel = ((uint8)(X + XOffset) << 8) | ((uint8)(Y + YOffset));
//            //*Pixel = (255 << 16);   // 255 0 0 0
//            //*Pixel = 255;   // 0 0 255 0
//
//            Pixel++;
//        }
//
//        Row += GlobalBackBuffer.Pitch;
//    }
//}

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
    GlobalBackBuffer.Memory = VirtualAlloc(nullptr, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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

internal void Win32InitDirectSound(HWND Window, uint32 SamplesPerSecond, uint32 BufferSize)
{
    // NOTE(Princerin): Load the library
    HMODULE DirectSoundLibrary = LoadLibrary(L"dsound.dll");

    if (DirectSoundLibrary)
    {
        // NOTE(Princerin): Get a DirectSound object! - Cooperative
        FPDirectSoundCreate* DirectSoundCreate = (FPDirectSoundCreate*)GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, nullptr)))
        {

			WAVEFORMATEX WaveFormatEx{};
			WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormatEx.nChannels = 2;
			WaveFormatEx.nSamplesPerSec = SamplesPerSecond;
			WaveFormatEx.wBitsPerSample = 16;
			WaveFormatEx.nBlockAlign = (WaveFormatEx.nChannels * WaveFormatEx.wBitsPerSample) / 8;
			WaveFormatEx.nAvgBytesPerSec = WaveFormatEx.nSamplesPerSec * WaveFormatEx.nBlockAlign;
			WaveFormatEx.cbSize = 0;

            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                // NOTE(Princerin): "Create" a primary buffer
                DSBUFFERDESC PrimaryBufferDescription {};
                PrimaryBufferDescription.dwSize = sizeof(DSBUFFERDESC);
                PrimaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                PrimaryBufferDescription.dwBufferBytes = 0;    // For primary buffer must set to 0

                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&PrimaryBufferDescription, &GlobalPrimaryBuffer, 0)))
                {
                    OutputDebugString(L"Primary buffer created successfully.\n");

                    HRESULT HResult = GlobalPrimaryBuffer->SetFormat(&WaveFormatEx);

                    if (SUCCEEDED(HResult))
                    {
                        OutputDebugString(L"Primary buffer format was set.\n");
                    }
                }

                // NOTE(Princerin): Start it playing!
            }

            // NOTE(Princerin): "Create" a secondary buffer
			DSBUFFERDESC SecondaryBufferDescription{};
            SecondaryBufferDescription.dwSize = sizeof(DSBUFFERDESC);
            SecondaryBufferDescription.dwFlags = 0;
			SecondaryBufferDescription.dwBufferBytes = BufferSize;
            SecondaryBufferDescription.lpwfxFormat = &WaveFormatEx;

            if (SUCCEEDED(DirectSound->CreateSoundBuffer(&SecondaryBufferDescription, &GlobalSecondaryBuffer, 0)))
            {
                OutputDebugString(L"Secondary buffer created successfully.\n");
            }
        }
    }
}

internal void Win32ClearSoundBuffer(Win32SoundOutput& SoundOutput)
{
	void* Region1 = nullptr;
	DWORD Region1Size = 0;
	void* Region2 = nullptr;
	DWORD Region2Size = 0;

    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput.SecondaryBufferSize, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
        uint8* DestSample = (uint8*)Region1;

		for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ByteIndex++)
        {
            *DestSample++ = 0;
		}

		DestSample = (uint8*)Region2;

		for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ByteIndex++)
		{
			*DestSample++ = 0;
		}

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void Win32FillSoundBuffer(Win32SoundOutput& SoundOutput, DWORD BytesToLock, DWORD BytesToWrite, GameSoundOutputBuffer& SoundBuffer)
{
	void* Region1 = nullptr;
	DWORD Region1Size = 0;
    void* Region2 = nullptr;
	DWORD Region2Size = 0;

    HRESULT HResult = GlobalSecondaryBuffer->Lock(BytesToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0);

	if (SUCCEEDED(HResult))
	{
		// TODO(Princerin): Assert that Region1Size/Region2Size is valid
		int16* SampleOut = (int16*)Region1;
        int16* SourceSample = SoundBuffer.Samples;

		DWORD Region1SampleCount = Region1Size / SoundOutput.BytesPerSample;

		for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++)
		{
			// TODO(Princerin): Draw this out for people
			*SampleOut++ = *SourceSample++;
			*SampleOut++ = *SourceSample++;

			SoundOutput.RunningSampleIndex++;
		}

		SampleOut = (int16*)Region2;
		DWORD Region2SampleCount = Region2Size / SoundOutput.BytesPerSample;

		for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++)
		{
			*SampleOut++ = *SourceSample++;
			*SampleOut++ = *SourceSample++;

			SoundOutput.RunningSampleIndex++;
		}

		HResult = GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);

        if (FAILED(HResult))
        {
            OutputDebugString(L"Unlock sound buffer failed.");
        }
	}
    else
    {
        OutputDebugString(L"Lock sound buffer failed.");
    }
}

internal HRESULT Win32InitXAudio()
{
    HMODULE XAUDIO2Library = LoadLibrary(XAUDIO2_DLL_W);

    if (XAUDIO2Library)
    {

    }

    IXAudio2* XAudio2 = nullptr;
    
    HRESULT HResult;
    if (FAILED(HResult = XAudio2Create(&XAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        return HResult;
    }

    IXAudio2MasteringVoice* MasterVoice = nullptr;
    if (FAILED(HResult = XAudio2->CreateMasteringVoice(&MasterVoice)))
    {
        return HResult;
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
  
        Activate = WParam;
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
    case WM_SYSKEYDOWN:
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
            
        case VK_F4:
        {
            bool AltKeyWasDown = ((LParam & (1 << 29)) != 0);

            if (AltKeyWasDown)
            {
                Running = false;
            }
        }

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

internal void Win32ProccessXInputDigitalButton(DWORD XInputButtonState, const GameButtonState& OldState, DWORD ButtonBit, GameButtonState& newState)
{
    if (XInputButtonState & ButtonBit)
    {
        newState.EndedDown = true;
    }

    newState.EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    newState.HalfTransitionCount = (OldState.EndedDown != newState.EndedDown) ? 1 : 0;
}

void ProccessInput(GameInput& OldInput, GameInput& NewInput, Win32SoundOutput& SoundOutput)
{
    DWORD MaxControllerCount = XUSER_MAX_COUNT;

    if (MaxControllerCount > ArrayCount(NewInput.Controllers))
    {
        MaxControllerCount = ArrayCount(NewInput.Controllers);
    }

	for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ControllerIndex++)
	{
        GameControllerInput OldController = OldInput.Controllers[ControllerIndex];
        GameControllerInput& NewController = NewInput.Controllers[ControllerIndex];

        NewController.IsAnalog = true;

		XINPUT_STATE ControllerState;
		DWORD Success = XInputGetState(ControllerIndex, &ControllerState);
		if (Success == ERROR_SUCCESS)
		{
			// NOTE(Princerin): This controller is plugged in.
			// TODO(Princerin): See if ControllerState.dwPacketNumber increments.
			XINPUT_GAMEPAD GamePad = ControllerState.Gamepad;

            bool AButton = GamePad.wButtons & XINPUT_GAMEPAD_A;
            bool BButton = GamePad.wButtons & XINPUT_GAMEPAD_B;
            bool XButton = GamePad.wButtons & XINPUT_GAMEPAD_X;
            bool YButton = GamePad.wButtons & XINPUT_GAMEPAD_Y;

            bool LeftShoulder = GamePad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
            bool RightShoulder = GamePad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

            bool StartButton = GamePad.wButtons & XINPUT_GAMEPAD_START;
            bool BackButton = GamePad.wButtons & XINPUT_GAMEPAD_BACK;

			bool Up = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
			bool Down = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
			bool Left = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
			bool Right = GamePad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

            Win32ProccessXInputDigitalButton(GamePad.wButtons, OldController.Down, XINPUT_GAMEPAD_A, NewController.Down);
            Win32ProccessXInputDigitalButton(GamePad.wButtons, OldController.Right, XINPUT_GAMEPAD_B, NewController.Right);
            Win32ProccessXInputDigitalButton(GamePad.wButtons, OldController.Left, XINPUT_GAMEPAD_X, NewController.Left);
            Win32ProccessXInputDigitalButton(GamePad.wButtons, OldController.Up, XINPUT_GAMEPAD_Y, NewController.Up);
            Win32ProccessXInputDigitalButton(GamePad.wButtons, OldController.LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER , NewController.LeftShoulder);
            Win32ProccessXInputDigitalButton(GamePad.wButtons, OldController.RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, NewController.RightShoulder);

            real32 X = 0.0f;

            if (GamePad.sThumbLX < 0)
            {
                X = (real32)GamePad.sThumbLX / 32768.0f;
            }
            else
            {
                X = (real32)GamePad.sThumbLX / 32767.0f;
            }

            NewController.StartX = OldController.EndX;

            NewController.MinX = NewController.MaxX = NewController.EndX = X;

			real32 Y = 0.0f;

			if (GamePad.sThumbLY < 0)
			{
                Y = (real32)GamePad.sThumbLY / 32768.0f;
			}
			else
			{
                Y = (real32)GamePad.sThumbLY / 32767.0f;
			}

			NewController.StartY = OldController.EndY;

			NewController.MinY = NewController.MaxY = NewController.EndY = Y;


            // XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849 
            // XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
			int16 ThumbLeftX = GamePad.sThumbLX;
            int16 ThumbLeftY = GamePad.sThumbLY;

            XOffset += -(ThumbLeftX >> 12);
            YOffset += ThumbLeftY >> 12;

            // Joystick deadzone fix.
            if (ThumbLeftY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                ThumbLeftY -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            }
            else if (ThumbLeftY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                ThumbLeftY += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            }
            else
            {
                ThumbLeftY = 0;
            }

			SoundOutput.ToneHertz = 480 + (int)(240.0f * ((real32)ThumbLeftY / 30000.0f));
			SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHertz;

            uint8 LeftTrigger = GamePad.bLeftTrigger;
            uint8 RightTrigger = GamePad.bRightTrigger;

            if (LeftTrigger)
            {
                OutputDebugString(L"Left Trigger pressed.\n");
            }

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

            if (Left)
            {
                XOffset++;
            }

            if (Right)
            {
                XOffset--;
            }

            if (Up)
            {
                YOffset++;
            }

            if (Down)
            {
                YOffset--;
            }

            XINPUT_VIBRATION Vibration;
            Vibration.wLeftMotorSpeed = 10000;
            Vibration.wRightMotorSpeed = 10000;
            //XInputSetState(0, &Vibration);
		}
		else
		{
			// NOTE(Princerin): The controller is not available.
		}
	}

    GameInput TempInput = NewInput;
    NewInput = OldInput;
    OldInput = TempInput;
}

void Render()
{

}

// NOTE(Princerin): Square wave
//void PlaySound()
//{
//    int16 ToneVolume = 16000;
//    int Hertz = 256;
//    int SamplesPerSecond = 48000;
//    int SquareWavePeriod = SamplesPerSecond / Hertz;
//    int HalfSquareWavePeriod = SquareWavePeriod / 2;
//    int BytesPerSample = sizeof(int16) * 2; // 16-bit stereo format, this gives you 2 channels * 2 byte = 4 bytes.
//    local_persist uint32 RunningSampleIndex = 0;
//    int32 SecondaryBufferSize = SamplesPerSecond * BytesPerSample;
//
//    // NOTE(Princerin): DirectSound output test.
//    void* Region1;
//    DWORD Region1Size;
//    void* Region2;
//    DWORD Region2Size;
//
//    DWORD WritePointer = 0;
//    DWORD BytesToWrite = 0;
//
//    DWORD PlayCursor;
//    DWORD WriteCursor;
//
//    if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
//    {
//        DWORD BytesToLock = RunningSampleIndex * BytesPerSample % SecondaryBufferSize;
//
//        // NOTE(Princerin): Startup situation
//        if (BytesToLock == PlayCursor)
//        {
//            BytesToWrite = SecondaryBufferSize;
//        }
//        // NOTE(Princerin): Case 1
//        else if (BytesToLock > PlayCursor)
//        {
//            BytesToWrite = SecondaryBufferSize - BytesToLock;
//            BytesToWrite += PlayCursor;
//        }
//        // NOTE(Princerin): Case 2
//        else
//        {
//            BytesToWrite = PlayCursor - BytesToLock;
//        }
//
//		if (SUCCEEDED(GlobalSecondaryBuffer->Lock(BytesToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
//		{
//			// TODO(Princerin): Assert that Region1Size/Region2Size is valid
//			int16* SampleOut = (int16*)Region1;
//			DWORD Region1SampleCount = Region1Size / BytesPerSample;
//			DWORD Region2SampleCount = Region2Size / BytesPerSample;
//
//			for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++)
//			{
//				int16 SampleValue = ((RunningSampleIndex / HalfSquareWavePeriod)) % 2 ? ToneVolume : -ToneVolume;
//
//				*SampleOut++ = SampleValue;
//				*SampleOut++ = SampleValue;
//                RunningSampleIndex++;
//			}
//
//            SampleOut = (int16*)Region2;
//
//			for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++)
//			{
//				int16 SampleValue = ((RunningSampleIndex / HalfSquareWavePeriod)) % 2 ? ToneVolume : -ToneVolume;
//
//				*SampleOut++ = SampleValue;
//				*SampleOut++ = SampleValue;
//                RunningSampleIndex++;
//			}
//
//            HRESULT HResult = GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
//		}
//
//        GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
//    }
//}


void PrintFPS(HDC DeviceContext, double FramePerSecond)
{
    wchar_t buffer[11]{ L"" };

    swprintf_s(buffer, L"%f", FramePerSecond);
    RECT TextRect{ 0, 0, 100, 20 };
    DrawText(DeviceContext, buffer, lstrlen(buffer), &TextRect, DT_SINGLELINE);
}

void Win32PreFillSoundBuffer(DWORD& BytesToLock, const Win32SoundOutput& SoundOutput, DWORD& BytesToWrite, bool& SoundIsValid)
{
    DWORD PlayCursor = 0;
    DWORD TargetCursor = 0;
    DWORD WriteCursor = 0;

    // TODO(Princerin): Tighten up sound logic so that we know where we should be
    // writing to and can anticipate the time spent in the game update.
    if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
    {
        BytesToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;

        TargetCursor = (PlayCursor + SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;

        // NOTE(Princerin): Case 1
        if (BytesToLock > TargetCursor)
        {
            BytesToWrite = SoundOutput.SecondaryBufferSize - BytesToLock;
            BytesToWrite += TargetCursor;
        }
        // NOTE(Princerin): Case 2
        else
        {
            BytesToWrite = TargetCursor - BytesToLock;
        }

        SoundIsValid = true;
    }
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

            GameInput Input[2];

            GameInput NewInput = Input[0];
            GameInput OldInput = Input[1];

            srand((uint32)time(nullptr));

            Timer GameTimer;
            GameTimer.Start();

            real64 RealTime = GameTimer.Now();
            real64 SimulationTime = 0.0;

            Win32SoundOutput SoundOutput {};
            int16 Samples[48000 * 2];

            Win32LoadXInput();
            Win32InitDirectSound(WindowHandle, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearSoundBuffer(SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            DWORD FrameCount = 0;

            int64 LastCycleCount = __rdtsc();

#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif      

            GameMemroy Memory{ };

            Memory.PermanentStorageSize = Megabytes(64);
            Memory.TransientStorageSize = Gigabytes(4);

            uint64 TotalSize = Memory.PermanentStorageSize + Memory.TransientStorageSize;

            Memory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            Memory.TransientStorage = (uint8*)Memory.PermanentStorage + Memory.PermanentStorageSize;

            while (Message.message != WM_QUIT && Running) 
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
					if (Activate)
					{
                        real64 Now = GameTimer.Now();

                        real64 GameTime = Now - RealTime;

                        while (SimulationTime < GameTime)
                        {
                            SimulationTime += 16;

                            //RenderWeirdGradient();
						    //RenderNoises(10000);
                            //RenderPureColorUInt8(255, 0, 0);
                            //RenderPureColorUInt32(255, 0, 0);

                            ProccessInput(OldInput, NewInput, SoundOutput);

							// NOTE(Princerin): DirectSound output test.
                            DWORD BytesToLock = 0;
                            DWORD BytesToWrite = 0;
							bool SoundIsValid = false;

                            Win32PreFillSoundBuffer(BytesToLock, SoundOutput, BytesToWrite, SoundIsValid);

							GameSoundOutputBuffer SoundBuffer{};

							SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = BytesToWrite >> 2; // / SoundOutput.BytesPerSample;
							SoundBuffer.Samples = Samples;

							GameOffScreenBuffer Buffer;
							Buffer.Memory = GlobalBackBuffer.Memory;
							Buffer.Width = GlobalBackBuffer.Width;
							Buffer.Height = GlobalBackBuffer.Height;
							Buffer.Pitch = GlobalBackBuffer.Pitch;

                            XOffset = (int)(sin(GameTimer.GetElapsedTime() / 1000.0) * 1000.0);
                            //YOffset = sin(GameTimer.GetElapsedTime() / 1000.0) * 1000.0;

							GameUpdateAndRender(Memory, NewInput, Buffer, SoundBuffer);

                            if (SoundIsValid)
                            {
                                Win32FillSoundBuffer(SoundOutput, BytesToLock, BytesToWrite, SoundBuffer);
                            }
                        }

                        HDC DeviceContext = GetDC(GetActiveWindow());

						Win32UpdateWindow(DeviceContext);

						//CleanBitmap(100, 149, 237);

                        GameTimer.Update();

						FrameCount++;

                        int64 EndCycleCount = __rdtsc();

                        int64 CyclesElapsed = EndCycleCount - LastCycleCount;

                        real64 ElapsedTime = GameTimer.GetElapsedTime();

                        real64 FrameTime = ElapsedTime / FrameCount;
                        real64 FramePerSecond = 1000 / FrameTime;

                        real64 MegaCyclePerFrame = (CyclesElapsed / (1000.0f * 1000.0f));

						wchar_t buffer[256]{ L"" };

						swprintf_s(buffer, L"%.02fms/f, %df/s, %.02fmc/f", FrameTime, (int32)FramePerSecond, MegaCyclePerFrame);

                        SetTextColor(DeviceContext, RGB(255, 255, 255));
                        SetBkMode(DeviceContext, TRANSPARENT);

						RECT TextRect{ 0, 0, 300, 20 };
						DrawText(DeviceContext, buffer, lstrlen(buffer), &TextRect, DT_SINGLELINE);

						ReleaseDC(WindowHandle, DeviceContext);

                        LastCycleCount = EndCycleCount;
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

