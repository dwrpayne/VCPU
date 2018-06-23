#include "ThreadedComponent.h"
#include <windows.h>


ThreadedAsyncComponent::ThreadedAsyncComponent(const wchar_t* name)
	: numBeforeWires(Wire::WireCount())
	, mUpdating(false)
	, mThread(&ThreadedAsyncComponent::ThreadedUpdate, this)
{
	SetThreadDescription((HANDLE)mThread.native_handle(), name);
}

void ThreadedAsyncComponent::DoOneUpdate()
{
	{
		std::lock_guard<std::mutex> lk(mMutex);
		mUpdating = true;
	}
	mCV.notify_one();
}

void ThreadedAsyncComponent::WaitUntilDone()
{
	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCV.wait(lk, [this] {return !mUpdating; });
	}
}

void ThreadedAsyncComponent::Exit()
{
	mExit = true;
	mCV.notify_one();
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
		{
			std::unique_lock<std::mutex> lk(mMutex);
			mUpdating = false;
			lk.unlock();
			mCV.notify_all();
		}
	}
}
