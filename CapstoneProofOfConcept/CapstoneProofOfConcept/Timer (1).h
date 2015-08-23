#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

class Timer
{
	LARGE_INTEGER startInt, stopInt, lastIntervalInt, frequencyInt;
	float LIToSecs( LARGE_INTEGER & l );
	float getDifference( LARGE_INTEGER& prev, LARGE_INTEGER& next );
public:
	Timer();
	void start();
	float stop();
	float interval();
};

#endif