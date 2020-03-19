// file ProcessorPoolDispatcher.h
//--------------------------------------------------------------
#ifndef XRAD__ProcessorPoolDispatcher_h
#define XRAD__ProcessorPoolDispatcher_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

/*!
	\brief Класс диспетчеризации многопотоковых запросов к фиксированному набору обработчиков

	Входящие запросы (вызовы Perform()) обрабатываются в порядке поступления.

	Класс Processor должен реализовывать метод operator(), его агрументы должны быть совместимы
	с аргументами вызова Perform().
	В текущей реализации поддерживаются только методы без возвращаемого значения (void).

	Недостаток: Выбор свободного обработчика производится в однопоточном режиме.
	Это плохо с точки зрения масштабируемости по количеству потоков.
*/
template <class Processor>
class ProcessorPoolDispatcher
{
	public:
		using Log = function<void (const string &)>;

	public:
		/*!
			\brief Создать набор обработчиков

			Класс ProcessorCreator должен реализовывать метод operator()(size_t processor_index).
			Параметр processor_index меняется от 0 до processor_count-1.
		*/
		template <class ProcessorCreator>
		ProcessorPoolDispatcher(size_t processor_count, ProcessorCreator processor_creator,
				shared_ptr<Log> log = nullptr);

		template <class... Args>
		void Perform(Args&&... args);

	private:
		using processor_list = list<pair<Processor, size_t>>;
		using processor_list_iterator = typename processor_list::iterator;

	private:
		processor_list_iterator GetProcessorItem();

	private:
		processor_list free_items;
		processor_list busy_items;
		mutex mx;
		struct CVItem
		{
			condition_variable* pcv;
			processor_list_iterator *pit;
			std::thread::id debug_thread_id;

			CVItem(condition_variable* pcv, processor_list_iterator *pit,
					std::thread::id debug_thread_id):
				pcv(pcv), pit(pit), debug_thread_id(debug_thread_id) {}
		};
		list<CVItem> cvs;
		shared_ptr<Log> log;
};

//--------------------------------------------------------------

XRAD_END

#include "ProcessorPoolDispatcher.hh"

//--------------------------------------------------------------
#endif // XRAD__ProcessorPoolDispatcher_h
