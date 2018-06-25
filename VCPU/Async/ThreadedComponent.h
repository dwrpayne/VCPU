#pragma once
#include <mutex>
#include <condition_variable>
#include <thread>

#include "Component.h"


class ThreadedAsyncComponent : public Component
{
public:
	ThreadedAsyncComponent(const wchar_t* name);

	void DoOneUpdate();
	void UpdateForever();
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
	bool mUpdatingUntilExit;
	std::thread mThread;
	int numBeforeWires;

	std::chrono::microseconds mElapsedTime;
};