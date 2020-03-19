// file ProcessorPoolDispatcher.hh
//--------------------------------------------------------------

#include "ThreadUtils.h"

XRAD_BEGIN

//--------------------------------------------------------------

template <class Processor>
template <class ProcessorCreator>
ProcessorPoolDispatcher<Processor>::ProcessorPoolDispatcher(size_t processor_count,
		ProcessorCreator processor_creator, shared_ptr<Log> log):
	log(std::move(log))
{
	for (size_t i = 0; i < processor_count; ++i)
	{
		free_items.emplace_back(processor_creator(i), i);
	}
}

//--------------------------------------------------------------

template <class Processor>
template <class... Args>
void ProcessorPoolDispatcher<Processor>::Perform(Args&&... args)
{
	auto item_it = GetProcessorItem();

	class free_busy_unlocker
	{
		public:
			free_busy_unlocker(processor_list_iterator item_it, processor_list *free_items,
					processor_list *busy_items,
					mutex *mx, list<CVItem> *cvs,
					Log *log):
				item_it(item_it), free_items(free_items), busy_items(busy_items), mx(mx), cvs(cvs),
				log(log) {}
			free_busy_unlocker(const free_busy_unlocker &) = delete;
			free_busy_unlocker &operator= (const free_busy_unlocker &) = delete;
			~free_busy_unlocker()
			{
				unique_lock<mutex> lock(*mx);
				if (cvs->empty())
				{
					if (log)
					{
						(*log)(ssprintf("ProcessorPoolDispatcher unlocker: [%zu] id = %s -> free\n",
								EnsureType<size_t>(item_it->second),
								EnsureType<const char*>(DebugThreadIdStr().c_str())));
					}
					free_items->splice(free_items->begin(), *busy_items, item_it);
				}
				else
				{
					auto &cv_item = cvs->front();
					if (log)
					{
						(*log)(ssprintf("ProcessorPoolDispatcher unlocker: [%zu] id = %s -> id = %s\n",
								EnsureType<size_t>(item_it->second),
								EnsureType<const char*>(DebugThreadIdStr().c_str()),
								EnsureType<const char*>(ToString(cv_item.debug_thread_id).c_str())));
					}
					*cv_item.pit = item_it;
					auto *cv = cv_item.pcv;
					cvs->pop_front(); // cv_item становится недействительным после этого вызова.
					lock.unlock();
					// Последовательность unlock(); notify_one(); предпочтительнее с точки зрения производительности
					// чем обратная последовательность.
					cv->notify_one();
				}
			}
		private:
			processor_list_iterator item_it;
			processor_list *free_items, *busy_items;
			mutex *mx;
			list<CVItem> *cvs;
			Log *log;
	};

	free_busy_unlocker bl(item_it, &free_items, &busy_items, &mx, &cvs, log.get());
	item_it->first(std::forward<Args>(args)...);
}

//--------------------------------------------------------------

template <class Processor>
auto ProcessorPoolDispatcher<Processor>::GetProcessorItem() -> processor_list_iterator
{
	unique_lock<mutex> lock(mx);
	if (!free_items.empty())
	{
		auto item_it = free_items.begin();
		busy_items.splice(busy_items.begin(), free_items, item_it);
		if (log)
		{
			(*log)(ssprintf("ProcessorPoolDispatcher::GetProcessorItem(): id = %s. Found free.\n",
					EnsureType<const char*>(DebugThreadIdStr().c_str())));
		}
		return item_it;
		// При возврате выполняется lock.unlock().
	}
	if (busy_items.empty())
		throw runtime_error("ProcessorPoolDispatcher::GetProcessorItem(): Perform item array is empty.");
	if (log)
	{
		(*log)(ssprintf("ProcessorPoolDispatcher::GetProcessorItem(): id = %s. Waiting...\n",
				EnsureType<const char*>(DebugThreadIdStr().c_str())));
	}

	condition_variable local_cv;
	auto item_it = busy_items.end();
	cvs.push_back(CVItem(&local_cv, &item_it, std::this_thread::get_id()));
	for (;;)
	{
		local_cv.wait(lock);
		if (item_it != busy_items.end()) // Проверка на spurious wakeup.
		{
			// item_it уже находится в списке busy_items.
			if (log)
			{
				(*log)(ssprintf("ProcessorPoolDispatcher::GetProcessorItem(): id = %s. Found from queue.\n",
						EnsureType<const char*>(DebugThreadIdStr().c_str())));
			}
			return item_it;
			// При возврате выполняется lock.unlock().
		}
	}
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
