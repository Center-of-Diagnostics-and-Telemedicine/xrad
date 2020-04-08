// file ThreadSync.h
//--------------------------------------------------------------
#ifndef __ThreadSync_h
#define __ThreadSync_h
//--------------------------------------------------------------

#include <type_traits>
#include <mutex>
#include <condition_variable>
#include <deque>

namespace XRAD_GUI
{

using namespace std;

//--------------------------------------------------------------



template <class Result>
class ResultHolder
{
	public:
		Result result;
};

template <>
class ResultHolder<void>
{
};

template <class Result>
struct ThreadLockData: ResultHolder<Result>
{
	std::mutex mt;
	std::condition_variable cond;
	bool unlocked = false;
	bool waited = false;
};



//--------------------------------------------------------------
//
//--------------------------------------------------------------



/*!
	\brief Класс для передачи ответственности за разблокировку потока

	Штатная разблокировка потока производится методом Unlock().

	Поток также разблокируется при уничтожении последнего объекта ThreadUnlocker,
	созданными из одного объекта ThreadLockData. (Первый объект создается конструктором
	от shared_ptr&lt;ThreadLockData&gt;, остальные копированием объекта.)
*/
template <class Result>
class ThreadUnlocker
{
	public:
		ThreadUnlocker() {}
		ThreadUnlocker(const shared_ptr<ThreadLockData<Result>> &lock);

	public:
		bool Valid() const { return helper.operator bool(); }
		bool Waited() const;
		void Unlock(const Result &result, bool allow_reuse = false);
		void Unlock(Result &&result, bool allow_reuse = false);

	private:
		class Helper;
		shared_ptr<Helper> helper;
};

//--------------------------------------------------------------

template <>
class ThreadUnlocker<void>
{
	public:
		ThreadUnlocker() {}
		ThreadUnlocker(const shared_ptr<ThreadLockData<void>> &lock);

	public:
		bool Valid() const { return helper.operator bool(); }
		void Unlock(bool allow_reuse = false);

	private:
		class Helper;
		shared_ptr<Helper> helper;
};



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class Result>
class ThreadLock
{
	public:
		ThreadLock();
		ThreadLock(const ThreadLock &) = delete;
		ThreadLock &operator=(const ThreadLock &) = delete;
		~ThreadLock();
	public:
		Result Wait();
		void Reset(const Result &default_result = Result());

		ThreadUnlocker<Result> GetUnlocker(const Result &default_result = Result());
	private:
		shared_ptr<ThreadLockData<Result>> lock_data;
};

//--------------------------------------------------------------

template <>
class ThreadLock<void>
{
	public:
		ThreadLock();
		ThreadLock(const ThreadLock &) = delete;
		ThreadLock &operator=(const ThreadLock &) = delete;
		~ThreadLock();
	public:
		void Wait();

		ThreadUnlocker<void> GetUnlocker();
	private:
		shared_ptr<ThreadLockData<void>> lock_data;
};



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class Result>
struct ThreadQueueData
{
	std::mutex mt;
	std::condition_variable cond;
	bool unlocked = false;
	std::deque<Result> queue;
};

//--------------------------------------------------------------

template <class Result>
class ThreadQueueWriter
{
	public:
		ThreadQueueWriter(const shared_ptr<ThreadQueueData<Result>> &lock_data);
	public:
		void Stop();
		void Push(const Result &value);
		void Push(Result &&value);
	private:
		class Helper;
		shared_ptr<Helper> helper;
};

//--------------------------------------------------------------

template <class Result>
class ThreadQueue
{
	public:
		ThreadQueue();
		ThreadQueue(const ThreadQueue &) = delete;
		ThreadQueue &operator=(const ThreadQueue &) = delete;
		~ThreadQueue();
	public:
		ThreadQueueWriter<Result> GetWriter(const Result &default_result = Result());
	public:
		bool Read(Result *result);
	private:
		shared_ptr<ThreadQueueData<Result>> lock_data;
		Result default_result;
};



//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------

#include "ThreadSync.hh"

//--------------------------------------------------------------
#endif // __ThreadSync_h
