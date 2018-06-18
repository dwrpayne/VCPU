#include "ThreadedComponent.h"

void ThreadedComponent::ThreadedUpdate()
{
	while (!mExit)
	{
		{
			std::unique_lock<std::mutex> lk(mMutex);
			mCV.wait(lk, [this] {return mReady || mExit; });
			if (mExit)
			{
				break;
			}
		}

		auto t1 = std::chrono::high_resolution_clock::now();
		Update();
		auto t2 = std::chrono::high_resolution_clock::now();
		mElapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
		{
			std::unique_lock<std::mutex> lk(mMutex);
			mReady = false;
			lk.unlock();
			mCV.notify_all();
		}
	}
}
