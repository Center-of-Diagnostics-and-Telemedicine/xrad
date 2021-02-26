// file Functional.h
//--------------------------------------------------------------
#ifndef XRAD__File_Functional_h
#define XRAD__File_Functional_h
//--------------------------------------------------------------
/*!
	\file
	\brief Вспомогательные средства для &lt;functional&gt;
*/
//--------------------------------------------------------------

#include "Config.h"
#include "BasicMacros.h"
#include <functional>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Вспомогательные средства для make_fn()
namespace make_fn_helpers
{

template <class T>
struct type_container
{
	using type = T;
};

template <class C, class R, class ...A>
auto decode_mem_fn_params(R (C::*func)(A...) const) -> type_container<R (A...)>;

template <class C, class R, class ...A>
auto decode_mem_fn_params(R (C::*func)(A...)) -> type_container<R (A...)>;

template <class F>
auto decode(const F &f) -> decltype(decode_mem_fn_params(&F::operator()));

template <class F>
auto decode(const std::reference_wrapper<F> &f) -> decltype(decode_mem_fn_params(&F::operator()));

template <class R, class ...A>
auto decode(R (*func)(A...)) -> type_container<R (A...)>;

} // namespace make_fn_helpers

//--------------------------------------------------------------

/*!
	\brief Создать обертку function&lt;T&gt; для произвольного функтора

	\param functor Указатель на функцию или объект класса с operator().

	Тип аргументов и возвращаемого значения определяются автоматически.

	В случае указателя на функцию:
	- функция не должна быть с переменным числом аргументов (типа printf);
	- если функция шаблонная или перегруженная, должен быть выбран конкретный
	экземпляр функции (например: `make_fn((double (*)(double))std::sin)`).

	В случае класса с operator():
	- operator() не должен быть перегруженным или шаблонным;
	- operator() не должен быть с переменным числом аргументов;
	- operator() должен быть public;
	- класс должен поддерживать копирование (требование std::function: CopyConstructible).

	Если класс не поддерживает копирование, можно использовать обертку
	для ссылки на функтор.

	Чтобы создать обертку для ссылки на функтор, следует использовать
	`std::reference_wrapper`: `make_fn(std::ref(functor))`.
	При этом необходимо учитывать время жизни объекта.
*/
template <class Functor>
auto make_fn(Functor functor)
{
	using func_type_container = decltype(make_fn_helpers::decode(functor));
	return std::function<typename func_type_container::type>(std::move(functor));
	// std::move: Если functor — это std::function&&, то make_fn(functor)
	// выполнит перемещение functor в новое место.
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_Functional_h
