#pragma once
#include <mutex>
#include <condition_variable>
#include <thread>

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

class ThreadedAsyncComponent : public Component
{
public:
	ThreadedAsyncComponent(const wchar_t* name);

	void DoOneUpdate();
	void WaitUntilDone();
	void Exit();
	inline bool IsRunning() const { return mUpdating; }
	virtual void Update() {} 
	std::chrono::microseconds GetElapsedTime() { return mElapsedTime; }
	void ThreadedUpdate();

private:
	std::condition_variable mCV;
	std::mutex mMutex;
	bool mUpdating;
	bool mExit;
	std::thread mThread;
	int numBeforeWires;

	std::chrono::microseconds mElapsedTime;
};