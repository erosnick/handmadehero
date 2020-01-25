#include "handmade.h"

internal void GameOutputSound(const GameSoundOutputBuffer& SoundBuffer)
{
    local_persist real32 tSine;
    int16 ToneVolume = 3000;
    int32 ToneHerz = 240;
    int32 WavePeriod = SoundBuffer.SamplesPerSecond / ToneHerz;

    int16* SampleOut = SoundBuffer.Samples;

    for (int SampleIndex = 0; SampleIndex < SoundBuffer.SampleCount; SampleIndex++)
    {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        tSine += 2.0f * PI * 1.0f / (real32)WavePeriod;
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

void GameUpdateAndRender(GameOffScreenBuffer& Buffer, int GreenOffset, int BlueOffset, const GameSoundOutputBuffer& SoundBuffer)
{
    RenderWeirdGradient(Buffer, GreenOffset, BlueOffset);
    GameOutputSound(SoundBuffer);
}
