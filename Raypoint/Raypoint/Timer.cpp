#include "Timer.h"

Timer::Timer(void)
{
	QueryPerformanceFrequency( &frequencyInt );
	QueryPerformanceCounter(&startInt);
	QueryPerformanceCounter(&lastIntervalInt);
}

float Timer::LIToSecs( LARGE_INTEGER & l )
{
	return ((float)l.QuadPart / (float)frequencyInt.QuadPart);
}

void Timer::start()
{
	QueryPerformanceCounter(&startInt);
}

float Timer::stop() 
{
	QueryPerformanceCounter(&stopInt);
	return getDifference( startInt, stopInt );
}

float Timer::interval()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	float result = getDifference( lastIntervalInt, currentTime );
	lastIntervalInt = currentTime;
	return result;
}

float Timer::getDifference( LARGE_INTEGER& prev, LARGE_INTEGER& next )
{
	LARGE_INTEGER time;
	time.QuadPart = next.QuadPart - prev.QuadPart;
	return LIToSecs( time ) ;
}