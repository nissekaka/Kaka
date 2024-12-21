#pragma once
#include <mutex>

static constexpr int MAX_FPS_SAMPLES = 128;

namespace Kaka
{
	class Timer
	{
	public:
		Timer();

		void Reset();
		float UpdateDeltaTime();
		float PeekDeltaTime() const;
		float GetTotalTime();
		float GetFPS();

	private:
		LARGE_INTEGER frequency;
		LARGE_INTEGER startTime;
		LARGE_INTEGER lastFrameTime;
		float deltaTime;
		float totalTime = 0.0f;
		float fps = 0.0f;
		float latestFps[MAX_FPS_SAMPLES] = {0.0f};
		int latestFpsIndex = 0;
		std::mutex mutex;
	};
}
