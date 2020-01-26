struct Win32WindowInfo
{
	int X;
	int Y;
	int Width;
	int Height;
};

struct Win32SoundOutput
{
	int16 ToneVolume = 3000;
	int ToneHertz = 240;
	int SamplesPerSecond = 48000;
	int WavePeriod = SamplesPerSecond / ToneHertz;
	int BytesPerSample = sizeof(int16) * 2; // 16-bit stereo format, this gives you 2 channels * 2 byte = 4 bytes.
	uint32 RunningSampleIndex = 0;
	int32 SecondaryBufferSize = SamplesPerSecond * BytesPerSample;
	real32 tSine = 0.0f;
	int LatencySampleCount = SamplesPerSecond / 15;
};

struct Win32OffScreenBuffer
{
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel = 4;
};