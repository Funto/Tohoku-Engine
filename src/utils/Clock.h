// Clock.h

#ifndef CLOCK_H
#define CLOCK_H

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
	#include <sys/timeb.h>
#endif

#include <cstdlib>	// for NULL

class Clock
{
private:
	unsigned int start_time, end_time;

public:
	// Return current time in milliseconds
	static unsigned int getMilliSeconds()
	{
#ifdef WIN32
		return GetTickCount();
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
	}

	// Return current time in seconds
	static double getSeconds()
	{
		return double(getMilliSeconds()) / 1000.0;
	}

	Clock()
	{
		reset();
	}

	// Start the clock
	void start()
	{
		start_time = getMilliSeconds();
		end_time = 0;
	}

	// End the clock
	void end()
	{
		end_time = getMilliSeconds();
	}

	// Reset the clock
	void reset()
	{
		start_time = end_time = 0;
	}

	// Get the elapsed time between the calls to start() and end()
	double getElapsedSeconds()
	{
		return double(end_time - start_time) / 1000.0;
	}
};

#endif // CLOCK_H
