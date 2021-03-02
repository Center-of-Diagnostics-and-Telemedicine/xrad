/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file Exceptions.h
//--------------------------------------------------------------
#ifndef Exceptions_h__
#define Exceptions_h__
/*!
	\addtogroup gr_XRAD_Basic
	@{

	\file
	\author kns
	\date 2014-12-22 17:49
	\brief Базовые классы исключений

	Внутренний файл библиотеки.
*/

#include "Config.h"
#include "BasicMacros.h"
#include <exception>
#include <string>
#include <tuple>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Базовый класс для классов исключений библиотеки
class xrad_exception : public std::exception
{
	public:
		explicit xrad_exception(const string& in_message)
			: message(in_message)
		{
		}

		virtual  ~xrad_exception()
		{
		}

		virtual const char * what() const noexcept
		{
			return (message.c_str());
		}

	private:
		//! \brief The stored message string
		string message;
};

//--------------------------------------------------------------

//!	\brief Отмена действий по любому условию
//!	(при подключенной интерфейсной библиотеке - обработка кнопки cancel)
class canceled_operation : public xrad_exception
{
	public:
		explicit canceled_operation(const char* what_arg) : xrad_exception(what_arg) {}
		explicit canceled_operation(const string& what_arg) : xrad_exception(what_arg) {}
};

//--------------------------------------------------------------

//!	\brief Выход из приложения
//!	(при подключенной интерфейсной библиотеке - обработка кнопки команды Quit)
class quit_application: public xrad_exception
{
	public:
		const int exit_code;
		explicit quit_application(const char* what_arg, int in_exit_code) : xrad_exception(what_arg), exit_code(in_exit_code) {}
		explicit quit_application(const string& what_arg, int in_exit_code) : xrad_exception(what_arg), exit_code(in_exit_code) {}
};

//--------------------------------------------------------------

/*!
	\brief Получить строку сообщения исключения в блоке catch

	Допускается вызывать только изнутри блока catch.
	В противном случае программа аварийно завершится.
*/
string GetExceptionString();

//! \brief Вспомогательная структура для GetExceptionStringOrRethrow
template <class... Exceptions>
struct RethrowSelectedExceptions;

//! \brief Вспомогательная структура для GetExceptionStringOrRethrow
template <class E, class... Exceptions>
struct RethrowSelectedExceptions<E, Exceptions...>
{
	//! \brief Обработка одного типа. Общий шаблон — для типа исключения.
	//! Для std::tuple отдельная специализация
	template <class E2>
	struct Tester
	{
		static void Process(std::exception *e)
		{
			if (dynamic_cast<E2*>(e)) // Полагается, что E2 — наследник std::exception.
				throw;
		}
	};

	template <class... Types>
	struct Tester<std::tuple<Types...>>
	{
		static void Process(std::exception *e)
		{
			RethrowSelectedExceptions<Types...>::Process(e);
		}
	};

	static void Process(std::exception *e)
	{
		Tester<E>::Process(e);
		RethrowSelectedExceptions<Exceptions...>::Process(e);
	}
};

//! \brief Вспомогательная структура для GetExceptionStringOrRethrow
template <>
struct RethrowSelectedExceptions<>
{
	static void Process(std::exception *e)
	{
		(void)e;
	}
};

/*!
	\brief Получить строку сообщения исключения в блоке catch, кроме исключений из списка,
	для исключений из списка пробросить исключение дальше

	Все исключения должны быть наследниками std::exception.

	Список может включать также std::tuple со списком исключений. См. StandardPassThroughExceptions.

	Допускается вызывать только изнутри блока catch.
	В противном случае программа аварийно завершится.

	\note
	Можно сделать обработку любых типов исключений, не наследников std::exception,
	но это замедлит выполнение функции. Сейчас проверка типа исключения производится
	относительно дешевым вызовом dynamic_cast. Вызов try { throw; } catch (Type &e) {} значительно
	более дорогой.
*/
template <class... Exceptions>
string GetExceptionStringOrRethrow()
{
	try
	{
		throw;
	}
	catch (std::exception &e)
	{
		RethrowSelectedExceptions<Exceptions...>::Process(&e);
		return GetExceptionString();
	}
	catch (...)
	{
		return GetExceptionString();
	}
}

//! \brief Такое употребление скорее всего является признаком ошибки: должна использоваться
//! функция GetExceptionString
template <>
string GetExceptionStringOrRethrow<>() = delete;

using StandardPassThroughExceptions = std::tuple<canceled_operation, quit_application>;

/*!
	\brief Получить строку сообщения исключения в блоке catch или пробросить исключение дальше,
	эквивалентно GetExceptionStringOrRethrow<StandardPassThrougExceptions>()
*/
inline string GetExceptionStringOrRethrow()
{
	return GetExceptionStringOrRethrow<StandardPassThroughExceptions>();
}

//--------------------------------------------------------------

XRAD_END

//! @} <!-- ^group gr_XRAD_Basic -->
#endif // Exceptions_h__
