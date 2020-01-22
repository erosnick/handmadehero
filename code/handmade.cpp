#include "handmade.h"

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

void GameUpdateAndRender(GameOffScreenBuffer& Buffer, int GreenOffset, int BlueOffset)
{
    RenderWeirdGradient(Buffer, GreenOffset, BlueOffset);
}
