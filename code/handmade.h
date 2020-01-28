#pragma once

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) ((Value) * 1024 * 1024)
#define Gigabytes(Value) ((Value) * 1024 * 1024 * 1024)
/*
TODO(Princerin): Services that the platform layer provides to the game.
*/
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
	void* PermanentStorage = nullptr;
};

void GameOutputSound(const GameSoundOutputBuffer& SoundBuffer, int32 ToneHerz);

void GameUpdateAndRender(GameMemroy& Memory, const GameInput& Input, GameOffScreenBuffer& Buffer, const GameSoundOutputBuffer& SoundBuffer, int32 ToneHerz);