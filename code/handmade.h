#pragma once

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

void GameOutputSound(GameSoundOutputBuffer& SoundBuffer, int SampleCountToOutput);

void GameUpdateAndRender(GameOffScreenBuffer& Buffer, int GreenOffset, int BlueOffset, const GameSoundOutputBuffer& SoundBuffer);