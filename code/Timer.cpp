#include "Timer.h"


Timer::Timer()
	: ElapsedTime(0.0),
	  TotalTime(0.0),
	  Active(false)
{
	QueryPerformanceFrequency(&Frequency);
}

void Timer::Start()
{
	QueryPerformanceCounter(&StartTime);
	Active = true;
}

void Timer::Stop()
{
	QueryPerformanceCounter(&EndTime);

	ElapsedTime = (EndTime.QuadPart - StartTime.QuadPart) * 1000.0f / Frequency.QuadPart;

	Active = false;
}

void Timer::Update()
{
	QueryPerformanceCounter(&NowTime);

	ElapsedTime = (NowTime.QuadPart - StartTime.QuadPart) * 1000.0f / Frequency.QuadPart;
}

void Timer::Reset()
{
	ElapsedTime = 0.0;
	ZeroMemory(&StartTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&EndTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&NowTime, sizeof(LARGE_INTEGER));
	Active = false;
}

double Timer::Now()
{
	QueryPerformanceCounter(&NowTime);

	return NowTime.QuadPart * 1000.0 / Frequency.QuadPart;
}

bool Timer::IsActive() const
{
	return Active;
}

double Timer::GetElapsedTime() const
{
	return ElapsedTime;
}
