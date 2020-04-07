// file ThreadSync.hh
//--------------------------------------------------------------

namespace XRAD_GUI
{

//--------------------------------------------------------------



template <class Result>
class ThreadUnlocker<Result>::Helper
{
	public:
		Helper(const shared_ptr<ThreadLockData<Result>> &lock);
		Helper(const Helper &) = delete;
		Helper &operator=(const Helper &) = delete;
		~Helper();
		bool Waited() const;
		void Unlock(const Result &result);
		void Unlock(Result &&result);
	private:
		shared_ptr<ThreadLockData<Result>> lock_data;
};

//--------------------------------------------------------------

template <class Result>
ThreadUnlocker<Result>::Helper::Helper(const shared_ptr<ThreadLockData<Result>> &lock):
	lock_data(lock)
{
}

template <class Result>
ThreadUnlocker<Result>::Helper::~Helper()
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->unlocked = true;
	lock_data->cond.notify_one();
}

template <class Result>
bool ThreadUnlocker<Result>::Helper::Waited() const
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	return lock_data->waited && !lock_data->unlocked;
}

template <class Result>
void ThreadUnlocker<Result>::Helper::Unlock(const Result &result)
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->unlocked = true;
	lock_data->result = result;
	lock_data->cond.notify_one();
}

template <class Result>
void ThreadUnlocker<Result>::Helper::Unlock(Result &&result)
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->unlocked = true;
	lock_data->result = std::move(result);
	lock_data->cond.notify_one();
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class Result>
ThreadUnlocker<Result>::ThreadUnlocker(const shared_ptr<ThreadLockData<Result>> &lock):
	helper(make_shared<Helper>(lock))
{
}

template <class Result>
bool ThreadUnlocker<Result>::Waited() const
{
	if (!helper)
		return false;
	return helper->Waited();
}

template <class Result>
void ThreadUnlocker<Result>::Unlock(const Result &result, bool allow_reuse)
{
	if (helper)
	{
		helper->Unlock(result);
		if (!allow_reuse)
			helper.reset();
	}
}

template <class Result>
void ThreadUnlocker<Result>::Unlock(Result &&result, bool allow_reuse)
{
	if (helper)
	{
		helper->Unlock(std::move(result));
		if (!allow_reuse)
			helper.reset();
	}
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class Result>
ThreadLock<Result>::ThreadLock()
{
	lock_data = make_shared<ThreadLockData<Result>>();
}

template <class Result>
ThreadLock<Result>::~ThreadLock()
{
}

template <class Result>
Result ThreadLock<Result>::Wait()
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	while (!lock_data->unlocked)
	{
		lock_data->waited = true;
		try
		{
			lock_data->cond.wait(locker);
		}
		catch (...)
		{
			lock_data->waited = false;
			throw;
		}
		lock_data->waited = false;
	}
	return std::move(lock_data->result);
}

template <class Result>
void ThreadLock<Result>::Reset(const Result &default_result)
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	lock_data->result = default_result;
	lock_data->unlocked = false;
}

template <class Result>
ThreadUnlocker<Result> ThreadLock<Result>::GetUnlocker(const Result &default_result)
{
	lock_data->result = default_result;
	lock_data->unlocked = false;
	return ThreadUnlocker<Result>(lock_data);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class Result>
class ThreadQueueWriter<Result>::Helper
{
	public:
		Helper(const shared_ptr<ThreadQueueData<Result>> &lock);
		Helper(const Helper &) = delete;
		Helper &operator=(const Helper &) = delete;
		~Helper();
		void Stop();
		void Push(const Result &result);
		void Push(Result &&result);
	private:
		shared_ptr<ThreadQueueData<Result>> lock_data;
};

//--------------------------------------------------------------

template <class Result>
ThreadQueueWriter<Result>::Helper::Helper(const shared_ptr<ThreadQueueData<Result>> &lock):
	lock_data(lock)
{
}

template <class Result>
ThreadQueueWriter<Result>::Helper::~Helper()
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->unlocked = true;
	lock_data->cond.notify_one();
}

template <class Result>
void ThreadQueueWriter<Result>::Helper::Stop()
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->unlocked = true;
	lock_data->cond.notify_one();
}

template <class Result>
void ThreadQueueWriter<Result>::Helper::Push(const Result &result)
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->queue.push_back(result);
	lock_data->cond.notify_one();
}

template <class Result>
void ThreadQueueWriter<Result>::Helper::Push(Result &&result)
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	if (lock_data->unlocked)
		return;
	lock_data->queue.push_back(std::move(result));
	lock_data->cond.notify_one();
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class Result>
ThreadQueueWriter<Result>::ThreadQueueWriter(const shared_ptr<ThreadQueueData<Result>> &lock_data):
	helper(make_shared<Helper>(lock_data))
{
}

template <class Result>
void ThreadQueueWriter<Result>::Stop()
{
	if (!helper)
		return;
	helper->Stop();
	helper.reset();
}

template <class Result>
void ThreadQueueWriter<Result>::Push(const Result &value)
{
	if (!helper)
		return;
	helper->Push(value);
}

template <class Result>
void ThreadQueueWriter<Result>::Push(Result &&value)
{
	if (!helper)
		return;
	helper->Push(std::move(value));
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class Result>
ThreadQueue<Result>::ThreadQueue():
	lock_data(make_shared<ThreadQueueData<Result>>())
{
}

template <class Result>
ThreadQueue<Result>::~ThreadQueue()
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	lock_data->unlocked = true;
	lock_data->queue.clear();
}

template <class Result>
ThreadQueueWriter<Result> ThreadQueue<Result>::GetWriter(const Result &default_result)
{
	this->default_result = default_result;
	lock_data->unlocked = false;
	return ThreadQueueWriter<Result>(lock_data);
}

template <class Result>
bool ThreadQueue<Result>::Read(Result *result)
{
	std::unique_lock<std::mutex> locker(lock_data->mt);
	for (;;)
	{
		if (!lock_data->queue.empty())
		{
			*result = lock_data->queue.front();
			lock_data->queue.pop_front();
			return true;
		}
		if (lock_data->unlocked)
		{
			*result = default_result;
			return false;
		}
		lock_data->cond.wait(locker);
	}
}



//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------
