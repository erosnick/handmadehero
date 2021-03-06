#include "handmade.h"

internal void GameOutputSound(const GameSoundOutputBuffer& SoundBuffer, int32 ToneHerz)
{
    local_persist real32 tSine;
    int16 ToneVolume = 3000;
    int32 WavePeriod = SoundBuffer.SamplesPerSecond / ToneHerz;

    int16* SampleOut = SoundBuffer.Samples;

    real32 Delta = 2.0f * PI * 1.0f / (real32)WavePeriod;

    for (int SampleIndex = 0; SampleIndex < SoundBuffer.SampleCount; SampleIndex++)
    {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        tSine += Delta;
    }
}

internal void RenderWeirdGradient(GameOffScreenBuffer& Buffer, int GreenOffset, int BlueOffset)
{
    uint8* Row = (uint8*)Buffer.Memory;

    for (int Y = 0; Y < Buffer.Height; Y++)
    {
        uint32* Pixel = (uint32*)Row;

        for (int X = 0; X < Buffer.Width; X++)
        {
            // Pixel in memory: BB GG RR XX
            // Little Endian Architecture 
            // 0xRRGGBBXX
            //*Pixel = (255 << 16) + (255 << 8) + 255;
            //*Pixel = ((uint8)X << 16) + ((uint8)Y << 8);
            *Pixel = ((uint8)(X + GreenOffset) << 8) | ((uint8)(Y + BlueOffset));
            //*Pixel = (255 << 16);   // 255 0 0 0
            //*Pixel = 255;   // 0 0 255 0

            Pixel++;
        }

        Row += Buffer.Pitch;
    }
}

void GameUpdateAndRender(GameMemroy& Memory, const GameInput& Input, GameOffScreenBuffer& Buffer, const GameSoundOutputBuffer& SoundBuffer)
{
    Assert(sizeof(GameState) <= Memory.PermanentStorageSize);

    GameState* State = (GameState*)Memory.PermanentStorage;

    if (!Memory.IsInitialized)
    {
        State->ToneHerz = 240;
        State->GreenOffset = 0;
        State->BlueOffset = 0;

        // TODO(Princerin): This may be more appropriate to do in the platform layer
        Memory.IsInitialized = true;
    }

    GameControllerInput Input0 = Input.Controllers[0];
    
    if (Input0.IsAnalog)
    {
        // NOTE(Princerin): Use analog movement tuning.
        State->ToneHerz = 240 + (int)(128.0f * Input0.EndY);
        State->BlueOffset += (int)(4.0f * Input0.EndX);
    }
    else
    {
        // NOTE(Princerin): Use digital movement tuning.
    }

    if (Input0.Down.EndedDown)
    {
        State->GreenOffset += 1;
    }

    RenderWeirdGradient(Buffer, State->GreenOffset, State->BlueOffset);
    GameOutputSound(SoundBuffer, State->ToneHerz);
}
