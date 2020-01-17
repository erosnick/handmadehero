#pragma once

#include <Windows.h>

class Timer
{
public:

	Timer();

	void Start();
	void Stop();
	void Update();
	void Reset();
	double Now();
	bool IsActive() const;
	double GetElapsedTime() const;

private:

	LARGE_INTEGER StartTime;
	LARGE_INTEGER NowTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Frequency;

	double ElapsedTime;
	double TotalTime;

	bool Active;
};