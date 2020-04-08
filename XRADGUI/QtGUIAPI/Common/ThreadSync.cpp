// file ThreadSync.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "ThreadSync.h"

//--------------------------------------------------------------

namespace XRAD_GUI
{

//--------------------------------------------------------------

class ThreadUnlocker<void>::Helper
{
	public:
		Helper(const shared_ptr<ThreadLockData<void>> &lock);
		Helper(const Helper &) = delete;
		Helper &operator=(const Helper &) = delete;
		~Helper();
		void Unlock();
	private:
		shared_ptr<ThreadLockData<void>> lock_data;
};

ThreadUnlocker<void>::Helper::Helper(const shared_ptr<ThreadLockData<void>> &lock):
	lock_data(lock)
{
}

ThreadUnlocker<void>::Helper::~Helper()
{
	Unlock();
}

void ThreadUnlocker<void>::Helper::Unlock()
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->unlocked = true;
	lock_data->cond.notify_one();
}

//--------------------------------------------------------------

ThreadUnlocker<void>::ThreadUnlocker(const shared_ptr<ThreadLockData<void>> &lock):
	helper(make_shared<Helper>(lock))
{
}

void ThreadUnlocker<void>::Unlock(bool allow_reuse)
{
	if (helper)
	{
		helper->Unlock();
		if (!allow_reuse)
			helper.reset();
	}
}

//--------------------------------------------------------------

ThreadLock<void>::ThreadLock()
{
	lock_data = make_shared<ThreadLockData<void>>();
}

ThreadLock<void>::~ThreadLock()
{
}

void ThreadLock<void>::Wait()
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	while (!lock_data->unlocked)
		lock_data->cond.wait(locker);
}

ThreadUnlocker<void> ThreadLock<void>::GetUnlocker()
{
	return ThreadUnlocker<void>(lock_data);
}

//--------------------------------------------------------------
} // namespace XRAD_GUI

//--------------------------------------------------------------
