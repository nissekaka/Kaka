#include "stdafx.h"
#include "Timer.h"

/* Note: QueryPerformanceCounter is generally considered to be a better
 * choice for high-precision timing in games and game engines on Windows
 * systems, compared to using the standard library's steady_clock or
 * high_resolution_clock.
 * The hardware performance counter is a feature of modern CPUs that counts
 * the number of clock cycles that have elapsed since the system was started,
 * providing a highly precise and consistent measure of time.*/

namespace Kaka
{
	// Constructor for the Timer class
	Timer::Timer()
	{
		// Query the performance frequency of the system
		QueryPerformanceFrequency(&frequency);

		Reset();
	}

	// Resets the timer
	void Timer::Reset()
	{
		// Lock the mutex to ensure that only one thread is modifying the variables at a time
		std::lock_guard<std::mutex> lock(mutex);

		// Query the performance counter for the current time
		QueryPerformanceCounter(&startTime);

		lastFrameTime = startTime;

		fps = 0.0f;
		totalTime = 0.0f;
	}

	// Gets the delta time (time between frames) since the last time UpdateDeltaTime was called
	float Timer::UpdateDeltaTime()
	{
		// Query the performance counter for the current time
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);

		// Calculate the time since the last frame in seconds
		const float dt = static_cast<float>(currentTime.QuadPart - lastFrameTime.QuadPart) /
			static_cast<float>(frequency.QuadPart);

		{
			std::lock_guard<std::mutex> lock(mutex);

			lastFrameTime = currentTime;

			totalTime += dt;
			fps = 1.0f / dt;
			latestFps[latestFpsIndex] = fps;
			latestFpsIndex = (latestFpsIndex + 1) % MAX_FPS_SAMPLES;
		}

		deltaTime = dt;
		return deltaTime;
	}

	float Timer::PeekDeltaTime() const
	{
		return deltaTime;
	}

	// Gets the total time elapsed since the timer was created or reset
	float Timer::GetTotalTime()
	{
		std::lock_guard<std::mutex> lock(mutex);

		return totalTime;
	}

	// Gets the current frames per second
	float Timer::GetFPS()
	{
		std::lock_guard<std::mutex> lock(mutex);
		float averageFps = 0.0f;

		for (const float latestFp : latestFps)
		{
			averageFps += latestFp;
		}

		return averageFps / MAX_FPS_SAMPLES;
	}
}
