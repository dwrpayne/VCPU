#pragma once
#include <mutex>
#include <condition_variable>

#include "Component.h"

class ThreadedComponent : public Component
{
public:
	ThreadedComponent(std::mutex& mutex, std::condition_variable& cv, bool& ready, bool& exit)
		: mMutex(mutex)
		, mCV(cv)
		, mReady(ready)
		, mExit(exit)
		, numBeforeWires(Wire::WireCount())
	{}

	void ThreadedUpdate();
	virtual void Update() = 0;
	std::chrono::microseconds GetElapsedTime() { return mElapsedTime; }

private:
	std::condition_variable& mCV;
	std::mutex& mMutex;
	bool& mReady;
	bool& mExit;
	int numBeforeWires;

	std::chrono::microseconds mElapsedTime;
};