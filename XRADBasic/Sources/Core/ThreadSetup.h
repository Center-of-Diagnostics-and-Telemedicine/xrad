// file ThreadSetup.h
//--------------------------------------------------------------
#ifndef XRAD__File_ThreadSetup_h
#define XRAD__File_ThreadSetup_h
/*!
	\file
	Здесь определяются:
	- Тип исключений SystemException (наследник std::exception).
	- Класс инициализации библиотеки для потока ThreadSetup.
*/
//--------------------------------------------------------------

// Этот файл не должен использовать весь <Core.h> во избежание циклических зависимостей.
// Включаем только минимально необходимый набор заголовочных файлов.
#include "Config.h"
#include "BasicMacros.h"

#if defined(XRAD_USE_MS_VERSION)
// !defined(XRAD_USE_PREFER_STD_VERSION):
// XRAD_USE_PREFER_STD_VERSION не проверяем, т.к. STD-реализация неполноценная.
#include <XRADBasic/Sources/PlatformSpecific/MSVC/Internal/ThreadSetup_MS.h>
#else
#define XRAD__ThreadSetup_STD_IMPLEMENTATION
#endif

#include <stdexcept>
#include <mutex>
#include <atomic>
#include <list>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

#ifdef XRAD__ThreadSetup_STD_IMPLEMENTATION

//! \brief Универсальная реализация типа системного исключения
class SystemException: public std::runtime_error
{
	public:
		SystemException(const std::string &message): std::runtime_error(message) {}
};

//! \brief Универсальная реализация инициализатора потока: ничего не делает
class ThreadSetup
{
};

#endif // XRAD__ThreadSetup_STD_IMPLEMENTATION

//--------------------------------------------------------------

namespace
{

//! \brief Проверка корректности определений в PlatformSpecific
void dummy_ThreadSetup_checks__()
{
	std::exception *e = (SystemException*)nullptr;
	(void)e;
	ThreadSetup ths; (void)ths;
}

} // namespace

//--------------------------------------------------------------

/*!
	\brief Список ошибок многопоточной обработки. Хранит сообщение исключения и опционально
	дополнительную информацию, такую как номер шага или идентификатор потока, где произошла ошибка

	В случае сложных ошибок, для которых не удалось сгенерировать сообщение об исключении,
	увеличивается внутренний счетчик ошибок.

	Особые ошибки (canceled_operation, quit_application) не заносятся в общий список ошибок,
	а хранятся отдельно. Они имеют приоритет над остальными ошибками, список остальных ошибок
	при наличии особых ошибок может не заполняться. Счетчик ошибок увеличивается.

	Таким образом, количество сообщений об ошибках может быть
	меньше количества ошибок.

	\sa \ref pg_ThreadSetup.
*/
template <class... Args>
class ThreadErrorCollectorEx
{
	public:
		ThreadErrorCollectorEx(const string &name);

		//! \brief Сбросить состояние для повторного использования объекта
		void Reset(const string &name);

		//! \brief Thread-safe call
		bool HasSpecialErrors() const;
		//! \brief Thread-safe call
		bool HasErrors() const;
		//! \brief Thread-safe call
		size_t ErrorCount() const;

		//! \brief Thread-safe call
		void AddErrorMessage(const string &message, const Args&... args);

		/*!
			\brief Thread-safe call. Поймать и сохранить информацию об исключении.
			Предназначена для вызова из catch-блока

			Увеличивает счетчик ошибок.

			В случае особого исключения (canceled_operation, quit_application) сохраняет его копию.
			При наличии нескольких исключений одного типа хранится первое по времени.

			В случае обычного исключения в список ошибок добавляется текст исключения.
		*/
		void CatchException(const Args&... args);

		//! \brief Выбросить исключение, если есть ошибки. Версия для sizeof...(Args) == 0
		//!
		//! Если есть особое исключение, выбрасывается оно, остальные ошибки игнорируются.
		template <class Void = void>
		auto ThrowIfErrors() -> std::enable_if_t<sizeof...(Args) == 0 && std::is_void<Void>::value, void>;

		// \brief Выбросить исключение, если есть ошибки. Версия для sizeof...(Args) != 0
		//
		// \param formatter Функция string formatter(const string &message, const Args&... args).
		//template <class Formatter>
		//auto ThrowIfErrors(Formatter formatter) -> std::enable_if_t<sizeof...(Args) != 0, void>;

		//! \brief Выбросить исключение, если есть особые ошибки.
		//! Выбрасывается копия оригинального исключения (canceled_operation, quit_application)
		void ThrowIfSpecialExceptions();

		//! \brief Применить функтор по очереди к каждому сообщению об ошибке
		//!
		//! \param f Функтор f(const string &message, const Args&... args).
		//!
		//! Замечание. Особые ошибки здесь не обрабатываются, см. ThrowIfSpecialExceptions().
		template <class Functor>
		void ProcessErrors(Functor f);

	private:
		string name;
		std::atomic<bool> has_special_errors = false;
		std::atomic<size_t> error_count = 0;
		std::mutex mt;
		std::exception_ptr quit_ep;
		std::exception_ptr cancel_ep;
		using tuple_type = std::tuple<string, Args...>;
		list<tuple_type> errors;

	private:
		//! \brief Шаблон для вызова функции с аргументами из std::tuple
		template <size_t n, size_t ...nums>
		struct caller: caller<n-1, n-1, nums...>
		{
		};

		template <size_t ...nums>
		struct caller<0, nums...>
		{
			template <class Functor>
			static void call(Functor f, const tuple_type &t)
			{
				f(std::get<nums>(t)...);
			}
		};
};

//! \brief Специализация ThreadErrorCollectorEx без дополнительных параметров (данных)
//! \sa \ref pg_ThreadSetup.
using ThreadErrorCollector = ThreadErrorCollectorEx<>;

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------

#include "ThreadSetup.hh"

//--------------------------------------------------------------
#endif // XRAD__File_ThreadSetup_h
