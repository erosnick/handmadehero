#pragma once

/*
	NOTE(Princerin):
	
	HANDMADE_INTERNAL:
		0 - Build for public release
		1 - Build for developer only

	HANDMADE_SLOW:
		0 - Not slow code allowed!
		1 - Slow code welcome
*/

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if _DEBUG
#define Assert(Expression) \
if (!(Expression)) { *(int*)0 = 0; }
#else
#define Assert(Expression)
#endif


#define Kilobytes(Value) ((uint64)(Value) * 1024)		// Force uint64, avoid signed integral constant overflow
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)
/*
TODO(Princerin): Services that the platform layer provides to the game.
*/
#if HANDMADE_INTERNAL
struct DebugReadFileResult
{
	uint32 ContentsSize;
	void* Contents;
};

internal DebugReadFileResult DebugPlatformReadEntireFile(const char* FileName);
internal void DebugPlatformFreeFileMemory(void* Memory);
internal bool DebugPlatformWriteEntireFile(const char* FileName, uint32 MemorySize, void* Memory);
#endif

inline uint32 SafeTruncateUInt64(uint64 Value)
{
	// TODO(Princerin): Defines for maximum values
	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return Result;
}

/*
NOTE(Princerin): Services that the game provides to the platform layer.
*/

// FOUR THINGS - Timing, controller/keyboard input, bitmap buffer to use, sound buffer to use
struct GameOffScreenBuffer
{
	// NOTE(Princerin): Pixels are always 32-bits wide, memory order BB GG RR XX
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct GameSoundOutputBuffer
{
	int16* Samples;
	int32 SamplesPerSecond;
	int32 SampleCount;
};

struct GameButtonState
{
	int HalfTransitionCount = 0;
	bool EndedDown = false;
};

struct GameControllerInput
{
	bool IsAnalog = false;

	real32 StartX = 0.0f;
	real32 StartY = 0.0f;

	real32 MinX = 0.0f;
	real32 MinY = 0.0f;

	real32 MaxX = 0.0f;
	real32 MaxY = 0.0f;

	real32 EndX = 0.0f;
	real32 EndY = 0.0f;

	GameButtonState Up;
	GameButtonState Down;
	GameButtonState Left;
	GameButtonState Right;
	GameButtonState LeftShoulder;
	GameButtonState RightShoulder;
};

struct GameInput
{
	GameControllerInput Controllers[4];
};

struct GameState
{
	int32 BlueOffset = 0;
	int32 GreenOffset = 0;
	int32 ToneHerz = 240;
};

struct GameMemroy
{
	bool IsInitialized = false;
	uint64 PermanentStorageSize = 0;
	// NOTE(Princerin): REQUIRED to be cleared to zero at startup
	void* PermanentStorage = nullptr;

	uint64 TransientStorageSize;
	void* TransientStorage;
};

void GameOutputSound(const GameSoundOutputBuffer& SoundBuffer, int32 ToneHerz);

void GameUpdateAndRender(GameMemroy& Memory, const GameInput& Input, GameOffScreenBuffer& Buffer, const GameSoundOutputBuffer& SoundBuffer, int32 ToneHerz);