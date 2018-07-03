#include <windows.h>
#include <assert.h>

#include "ThreadedComponent.h"
#include "Wire.h"

ThreadedAsyncComponent::ThreadedAsyncComponent(const wchar_t* name)
	: numBeforeWires(Wire::WireCount())
	, mUpdating(false)
	, mThread(&ThreadedAsyncComponent::ThreadedUpdate, this)
{
	SetThreadDescription((HANDLE)mThread.native_handle(), name);
}

ThreadedAsyncComponent::~ThreadedAsyncComponent()
{
	Exit();
}

void ThreadedAsyncComponent::DoOneUpdate()
{
	if (!IsRunning())
	{
		std::lock_guard<std::mutex> lk(mMutex);
		mUpdating = true;
	}
	mCV.notify_one();
}

void ThreadedAsyncComponent::UpdateForever()
{
	mUpdatingUntilExit = true;
	{
		std::lock_guard<std::mutex> lk(mMutex);
		mUpdating = true;
	}
	mCV.notify_one();
}

void ThreadedAsyncComponent::StopUpdating()
{
	mUpdatingUntilExit = false;
	mExit = true;
	WaitUntilDone();
}

void ThreadedAsyncComponent::WaitUntilDone()
{
	assert(!mUpdatingUntilExit || mExit && "Not safe to call WaitUntilDone when updating forever and not exit yet!");
	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCV.wait(lk, [this] {return !mUpdating; });
	}
}

void ThreadedAsyncComponent::Exit()
{
	mUpdatingUntilExit = false;
	mExit = true;
	mCV.notify_one();
	WaitUntilDone();
	mThread.join();
}

void ThreadedAsyncComponent::ThreadedUpdate()
{
	while (!mExit)
	{
		{
			std::unique_lock<std::mutex> lk(mMutex);
			mCV.wait(lk, [this] {return mUpdating || mExit; });
			if (mExit)
			{
				break;
			}
		}

		auto t1 = std::chrono::high_resolution_clock::now();
		Update();
		auto t2 = std::chrono::high_resolution_clock::now();
		mElapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
		if (!mUpdatingUntilExit)
		{
			{
				std::unique_lock<std::mutex> lk(mMutex);
				mUpdating = false;
				lk.unlock();
				mCV.notify_all();
			}
		}
	}
}
