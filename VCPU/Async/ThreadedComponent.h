#pragma once
#include <mutex>
#include <condition_variable>
#include <thread>

#include "Component.h"


class ThreadedAsyncComponent : public Component
{
public:
	ThreadedAsyncComponent(const wchar_t* name);

	virtual ~ThreadedAsyncComponent();

	void DoOneUpdate();
	void UpdateForever();
	void StopUpdating();
	void WaitUntilDone();
	void Exit();
	inline bool IsRunning() const { return mUpdating; }
	virtual void Update() {} 
	std::chrono::microseconds GetElapsedTime() { return mElapsedTime; }
	void ThreadedUpdate();

protected:
	bool mExit;

private:
	std::condition_variable mCV;
	std::mutex mMutex;
	bool mUpdating;
	bool mUpdatingUntilExit;
	std::thread mThread;
	int numBeforeWires;

	std::chrono::microseconds mElapsedTime;
};