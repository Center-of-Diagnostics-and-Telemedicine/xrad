// file FieldTraits.h
//--------------------------------------------------------------
#ifndef XRAD__File_FieldTraits_h
#define XRAD__File_FieldTraits_h
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Базовая реализация FieldTraits

	@}
*/
//--------------------------------------------------------------

#include <XRADBasic/Sources/Core/BasicUtils.h>
#include <XRADBasic/Sources/Core/NumberTraits.h>

XRAD_BEGIN
namespace	AlgebraicStructures
{

/*!
	\brief Шаблонный класс-помощник для реализации FieldElement

	Общий шаблон не используется, используются специализации.

	Специализации должны определять тип "tag_t", задающий реализацию FieldElement: скалярную,
	комплексную или какую-то другую (пользовательскую).
	Тип tag_t должен быть отличен от void: в некоторых вспомогательных шаблонах
	void используется в качестве особого значения.

	Использование общего шаблона должно приводить к ошибке компиляции.
	В текущей реализации он задает некий фиктивный tag_t,
	чтобы сообщение компилятора об ошибке было чуть более информативным.
	(Под MSVC 2015 использование static_assert дает более плохой результат:
	важную информацию о типе, приведшем к ошибке, приходится искать в другом сообщении об ошибке.)

	Параметр class Conditional = void нужен для условной перегрузки шаблона
	(с помощью std::enable_if и аналогичных механизмов).
	Используются специализации с Conditional = void.

	\todo Этот механизм определения tag по типу больше не используется. Удалить.
*/
template<class T, class Conditional = void>
struct FieldTraits
{
	struct field_element_tag_is_not_defined_for_this_value_type {};
	using tag_t = field_element_tag_is_not_defined_for_this_value_type;
};

/*!
	Запрещаем использовать FieldElement для типа данных bool.
	FieldElement задает линейное пространство, а линейное пространство
	с bool не имеет смысла использовать.

	См. BooleanAlgebra, BooleanFunction.
*/


template<class T>
struct FieldTraits<T, typename std::enable_if<
		is_same_ignore_cv<T, bool>::value
		>::type>
{
	// Конструкция с !std::is_same_ignore_cv нужна для того, чтобы ошибка возникала только при попытке
	// использовать FieldTraits<bool>.
	static_assert(!is_same_ignore_cv<T, bool>::value, "FieldElement must not be used with \"bool\" value type.");
};

/*!
	\brief Специализация для задания тегов для классов вместе с наследниками

	(*1490625828)

	Использует вспомогательную функцию GetFieldElementTag(T*).
	Механизм работает так. Данная специализация применяется для всех классов,
	для указателей на которые может быть вызвана функция
	GetFieldElementTag (имеется подходящая перегрузка), и тип её возвращаемого
	значения отличен от void.
	Модификаторы const-volatile при этом не используются.
	Типом тега является тип возвращаемого значения функции GetFieldElementTag.

	Достаточно объявления функции GetFieldElementTag, тело функции определять не требуется.

	Функция должна быть объявлена (перегружена) для указателей на те базовые классы,
	для которых вместе с их наследниками требуется специализировать FieldTraits.
	Тип возвращаемого значения функции будет типом tag_t в FieldTraits.

	Если в цепочке потомков класса A для какого-то потомка B с его наследниками потребуется
	отменить специализацию FieldTraits, нужно объявить функцию GetFieldElementTag от
	указателя на B с возвращаемым типом void:

	void GetFieldElementTag(B*);
*/
template<class T>
struct FieldTraits<T,
		typename std::enable_if<
				!std::is_same<
						decltype(GetFieldElementTag(std::declval<typename std::remove_cv<T>::type*>())),
						void>::value,
				void>::type>
{
	using tag_t = decltype(GetFieldElementTag(std::declval<typename std::remove_cv<T>::type*>()));
};

/*!
	\brief Тег для FieldElement, задающий скалярную реализацию

	Внутри структуры определяем тип tag_t = FieldTagScalar для того, чтобы реализации
	FieldTraits<T> могли быть, для краткости записи, унаследованы от этой структуры.
*/
#if 1
typedef xrad::number_complexity::scalar FieldTagScalar;
#else
struct FieldTagScalar
{
	using tag_t = FieldTagScalar;
};
#endif

/*!
	\brief Специализация для встроенных арифметических типов данных (кроме bool):
	задает скалярную реализацию FieldElement
*/
template<class T>
struct FieldTraits<T, typename std::enable_if<
		std::is_arithmetic<T>::value &&
		!is_same_ignore_cv<T, bool>::value
		>::type>: FieldTagScalar {};

} // namespace	AlgebraicStructures
XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FieldTraits_h
