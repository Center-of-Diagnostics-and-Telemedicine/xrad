//	file BasicUtils.h
//	Created by KNS on 20.06.03
//--------------------------------------------------------------
#ifndef XRAD__basic_utils_h
#define XRAD__basic_utils_h
/*!
	\addtogroup gr_XRAD_Basic
	@{

	\file
	\brief Базовые утилиты

	Внутренний файл библиотеки.
*/

#include "Config.h"
#include "BasicMacros.h"
#include "MathConstants.h"

#include <cstdlib>
#include <cmath>
#include <cfenv>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <stdexcept>

XRAD_BEGIN

/*!
	\brief аналог std::is_same, определяющий равенство типов без учета const-volatile
*/

template<class T1, class T2>
using is_same_ignore_cv = std::is_same<typename std::remove_cv<T1>::type, typename std::remove_cv<T2>::type>;

/*!
	\brief Аналог std::is_same, только для произвольного количества типов (true = все типы одинаковые)
*/
template <class T1, class... Ts>
struct are_same: std::bool_constant<
		std::is_same<T1, std::tuple_element_t<0, std::tuple<Ts...>>>::value &&
		are_same<Ts...>::value>
{};

template <class T1, class T2>
struct are_same<T1, T2>: std::bool_constant<std::is_same<T1, T2>::value> {};

template <class T>
struct are_same<T>: std::bool_constant<true> {};

//! \brief Аналог std::tuple_element, для которого существует x_tuple_element<0, tuple<>>, не содержащий type
//!
//! Шаблон std::tuple_element нельзя употреблять в enable_if, если может возникнуть
//! tuple<> (пустой), это приводит к ошибке компиляции из-за static_assert.
template <size_t I, class T>
struct x_tuple_element: std::tuple_element<I, T> {};

template <>
struct x_tuple_element<0, std::tuple<>>
{
};

//! \brief Аналог std::tuple_element_t, не приводящий к безусловной ошибке компиляции для tuple<>, см. x_tuple_element
template <size_t I, class... Ts>
using x_tuple_element_t = typename x_tuple_element<I, Ts...>::type;

/*!
	\brief аналог std::is_arithmetic, дающий false_type для bool
*/

template<class T>
struct is_arithmetic_but_bool_helper : std::is_arithmetic<T>{};

template<>
struct is_arithmetic_but_bool_helper<bool> : std::false_type{};

template<class T>
struct is_arithmetic_but_bool : is_arithmetic_but_bool_helper<typename std::remove_cv<T>::type>{};

//! \brief Скопировать константность с первого типа на второй
template <class Src, class T>
using copy_const_t = std::conditional_t<std::is_const<Src>::value,
		std::add_const_t<T>, std::remove_const_t<T>>;

//--------------------------------------------------------------

/*!
	\brief Шаблонная функция, проверяющая тип своего аргумента.
		Используется для проверки типов аргументов для printf-подобных функций.
		Если тип аргумента не соответствует заданному, вызов функции не скомпилируется

	Применяется в тех случаях, когда аргумент имеет нужный тип
	(приведение типа не требуется),
	чтобы гарантировать, что при изменениях кода тип аргумента
	не поменяется.

	printf("%d", EnsureType<int>(GetValue()));
*/
template<class T, class Arg>
inline typename std::enable_if<std::is_same<T, std::decay_t<Arg>>::value,
		Arg&&>::type EnsureType(Arg&& arg)
{
	return std::forward<Arg&&>(arg);
}

//--------------------------------------------------------------

//! \brief Вспомогательный класс для try_equals(). Общий шаблон обрабатывает типы, для которых нет операции сравнения
template <bool default_result, class T1, class T2, class = std::void_t<> >
struct try_equals_helper
{
	using result_type = bool;
	static constexpr bool equals(const T1 &, const T2 &)
	{
		return default_result;
	}
};

//! \brief Эта специализация шаблона обрабатывает типы, для которых есть операция сравнения
template <bool default_result, class T1, class T2>
struct try_equals_helper<default_result, T1, T2, std::void_t<decltype(std::declval<T1&>() == std::declval<T2&>())>>
{
	using result_type = decltype(std::declval<T1&>() == std::declval<T2&>());
	static result_type equals(const T1 &v1, const T2 &v2)
	{
		return v1 == v2;
	}
};

/*!
	\brief Сравнить два значения, если для них существует operator==. Иначе вернуть default_result

	Пример:
	~~~~
	template <class T> int test(const T &x, const T &y)
	{
		if (try_equals<false>(x, y))
		{
			...
		}
		else
		{
			...
		}
	}
	~~~~
*/
template <bool default_result, class T1, class T2>
inline auto try_equals(const T1 &v1, const T2 &v2) -> typename try_equals_helper<default_result, T1, T2>::result_type
{
	return try_equals_helper<default_result, T1, T2>::equals(v1, v2);
}

//--------------------------------------------------------------

template <class TNew, class T>
std::shared_ptr<TNew> rebind_shared(const std::shared_ptr<T> &sp, TNew *ptr)
{
	return std::shared_ptr<TNew>(sp, ptr);
}

template <class T>
std::weak_ptr<T> to_weak_ptr(const std::shared_ptr<T> &sp)
{
	return sp;
}

//--------------------------------------------------------------

/*!
	\brief Аналог assert, вместо abort() бросается заданное исключение.
	Функция приводит к потере быстродействия, ее вызовы нужно исключить
*/
template<class exception_t>
[[deprecated("Use XRAD_ASSERT_THROW(condition) instead.")]]
void	assert_throw(bool condition, exception_t ex)
{
	if(condition==0)
	{
		ForceDebugBreak();
		throw ex;
	}
}

/*!
	\brief Функция форматирования сообщения для XRAD_ASSERT_THROW. Помимо формирования строки,
	вызывает останов в отладчике

	Функция не шаблонная и не inline, т.к. она может использовать функции работы со строками
	(ssprintf), описания которых находятся в заголовочных файлах, зависящих от данного файла.
*/
string format_assert_message(const string &condition, const string &func,
		const string &message = string());


//! \brief Тип исключения, который генерируется из XRAD_ASSERT_THROW
class assert_exception : public runtime_error
{
public:
	explicit assert_exception(const string &message): runtime_error(message) {}
	explicit assert_exception(const char *message): runtime_error(message) {}
};

//! \brief Аналог assert, вместо abort() бросается исключение assert_exception
#define XRAD_ASSERT_THROW(condition) \
{ if(!(condition)) throw assert_exception(format_assert_message(#condition, __func__)); }

//! \brief Аналог assert, вместо abort() бросается исключение заданного типа
#define XRAD_ASSERT_THROW_EX(condition, exception_type) \
{ if(!(condition)) throw exception_type(format_assert_message(#condition, __func__)); }

//! \brief Аналог assert, вместо abort() бросается исключение заданного типа
//! с определяемым пользователем сообщением
#define XRAD_ASSERT_THROW_M(condition, exception_type, message) \
{ if(!(condition)) throw exception_type(format_assert_message(#condition, __func__, message)); }


/*!
	\brief Оберточная функция для std::malloc().
	Следует использовать только в тех случаях, когда вызов через
	new[] может привести к потере быстродействия из-за вызова конструктора
*/
template<class T>
inline T* CreatePointer(size_t size)
{
	T* result = (T*)std::malloc(sizeof(T) * size);
	if(result) return result;
	else throw std::bad_alloc();
}

/*!
	\brief Оберточная фунция для std::free().
	Освобождает память, выделенную через malloc/calloc и обнуляет указатель.
	Тип аргумента только такой: "T *&".
*/
template<class T>
inline void DestroyPointer(T *&ptr)
{
	if(ptr)
	{
		std::free(ptr);
		ptr = NULL;
	}
}

/*!
	\brief Оберточная фунция для fclose.
	Закрывает файл, открытый через fopen, и обнуляет указатель.
	Тип аргумента только такой: "FILE_T *&"
*/
template< class FILE_T>
inline void DestroyCFile(FILE_T *&ptr)
{
	if(ptr)
	{
		fclose(ptr);
		ptr = NULL;
	}
}

//--------------------------------------------------------------

/*!
	\brief Оберточная фунция для delete.
	Освобождает память, выделенную через new, и обнуляет указатель
*/
template< class T>
inline void DestroyObject(T *&x)
{
	if(x)
	{
		delete x;
		x = 0;
	}
}

/*!
	\brief Оберточная фунция для delete[].
	Освобождает память, выделенную через new[], и обнуляет указатель
*/
template< class T>
inline void DestroyArray(T *&x)
{
	if(x)
	{
		delete[] x;
		x = 0;
	}
}

enum destroy_pointer_method
{
	destroy_free,
	destroy_object,
	destroy_array,
	destroy_c_file,

	destroy_unknown
};

/*!
	\brief Освобождает память способом, зависящим от второго аргумента,
	и обнуляет указатель. Тип аргумента только такой: "T *&"
*/
template<class T>
inline void DestroyPointer(T *&ptr, destroy_pointer_method method)
{
	switch(method)
	{
		case destroy_free:
			DestroyPointer(ptr);
			break;
		case destroy_object:
			DestroyObject(ptr);
			break;
		case destroy_array:
			DestroyArray(ptr);
			break;
		case destroy_c_file:
			DestroyCFile(ptr);
			break;

		default:
			throw invalid_argument("DestroyPointer(), unknown destroy method");
	};
}

//--------------------------------------------------------------

//!	\brief размер массива в элементах (делает то же, что sizeof(array)/sizeof(element))
template<typename T, size_t N>
inline size_t array_length(T(&)[N]){ return N; }

//--------------------------------------------------------------
//
//	проверка диапазонов числа
//

/*!
	\brief Ограничить переданное число заданным диапазоном

	Первоначальная реализация:

			return x<min_val ? (T)min_val : x>max_val ? (T)max_val : x;

	В исходном варианте эта функция
	пропускала NAN, посколько любое условие типа NAN <> x всегда дает false.
	Теперь в случае NAN будет возвращаться min_val. 16.12.2009 кнс
*/
template< class T, class T2, class T3>
inline T range(const T &x, const T2 &min_val, const T3 &max_val)
{
	return x >= static_cast<T>(min_val) ? (x<=static_cast<T>(max_val) ? x : static_cast<T>(max_val)) : static_cast<T>(min_val);
}

/*!
	\brief Находится ли число в заданном диапазоне

	\todo Разобраться с NAN, см. \ref range.
*/
template< class T, class T2, class T3>
inline bool in_range(const T &x, const T2 &min_val, const T3 &max_val)
{
	//return (x <= max_val && x >= min_val) ? true : false;
	return x > static_cast<T>(max_val) ? false : (x < static_cast<T>(min_val) ? false : true);
}

/*!
	\brief Находится ли число вне заданного диапазона

	\todo Разобраться с NAN, см. \ref range.
*/
template< class T, class T2, class T3>
inline bool not_in_range(const T &x, const T2 &min_val, const T3 &max_val)
{
	return x > static_cast<T>(max_val) ? true : (x < static_cast<T>(min_val) ? true : false);
}

// Variadic min / max
//
// Мы требуем аргументы типа const T &, T общий для всех аргументов.
// Это соответствует прототипу std::min.
//
// Функции имеют названия, отличные от min и max, поскольку min и max
// имеют перегрузки от трех аргументов, где последний аргумент — предикат.
//
// Пара vmin и vmax должна работать следующим образом:
// - Для типа T требуется наличие только operator < .
// - Если минимум достигается на нескольких аргументах, vmin возвращает первый из них.
// - Если максимум достигается на нескольких аргументах, vmax возвращает последний из них.
//
// См. также:
// https://stackoverflow.com/questions/23815138/implementing-variadic-min-max-functions
// В версии со stackoverflow допускаются различающиеся типы аргументов функции,
// допускается использование ссылок.

//! \brief Min с переменным числом аргументов: запрещаем вызывать функцию с разными типами аргументов
template <typename Arg1, typename Arg2>
constexpr const Arg1 &vmin(const Arg1 &arg1, const Arg2 &arg2) = delete;

//! \brief Min с переменным числом аргументов, специализация для двух аргументов
template <typename Arg>
constexpr const Arg &vmin(const Arg &arg1, const Arg &arg2)
{
	// arg1 <= arg2 ==> arg1
	return !(arg2 < arg1)? arg1: arg2;
}

//! \brief Min с переменным числом аргументов: запрещаем вызывать функцию с разными типами аргументов
//!
//! Можно бы было вставить static_assert, но тогда дольше добираться до места ошибочного вызова.
template <typename... Args>
constexpr auto vmin(
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				!are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> &arg,
		const Args &... args) ->
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				!are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> & = delete;

//! \brief Min с переменным числом аргументов
template <typename... Args>
constexpr auto vmin(
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> &arg,
		const Args &... args) ->
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> &
{
	return vmin(arg, vmin(args...));
}

//! \brief Max с переменным числом аргументов: запрещаем вызывать функцию с разными типами аргументов
template <typename Arg1, typename Arg2>
constexpr const Arg1 &vmax(const Arg1 &arg1, const Arg2 &arg2) = delete;

//! \brief Max с переменным числом аргументов, специализация для двух аргументов
template <typename Arg>
constexpr const Arg &vmax(const Arg &arg1, const Arg &arg2)
{
	// arg2 >= arg1 ==> arg2
	return !(arg2 < arg1)? arg2: arg1;
}

//! \brief Max с переменным числом аргументов: запрещаем вызывать функцию с разными типами аргументов
//!
//! Можно бы было вставить static_assert, но тогда дольше добираться до места ошибочного вызова.
template <typename... Args>
constexpr auto vmax(
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				!are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> &arg,
		const Args &... args) ->
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				!are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> & = delete;

//! \brief Max с переменным числом аргументов
template <typename... Args>
constexpr auto vmax(
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> &arg,
		const Args &... args) ->
		const std::enable_if_t<(sizeof...(Args) > 1) &&
				are_same<Args...>::value, x_tuple_element_t<0, std::tuple<Args...>>> &
{
	return vmax(arg, vmax(args...));
}


//! \brief Линейная комбинация двух чисел с заданными весами
template<class T1, class T2>
T1 mix(T1 &v0, const T2 &v1, double w0, double w1) { return v0*w0 + v1*w1; }

//! \brief Линейная комбинация двух чисел с весами 1-w, w. Важно: здесь w это вес второго числа
template<class T1, class T2>
T1 mix(T1 &v0, const T2 &v1, double w) { return mix(v0, v1, 1. - w, w); }

//!	\brief Является ли аргумент числом, допустимым в вычислениях. Создана в дополнение к std::isnormal, которая возвращает false для x=0
inline bool is_number(const double x){ return (!isnan(x)) && (!isinf(x)); }

//! \brief Принудительно заменяет недопустимое число (inf или nan) заданным значением
template<class T>
void	reset_not_number(T &x, const T& n = T(0)) { if (!is_number(x)) x = n; }


//--------------------------------------------------------------
//
//	простейшие вычислительные функции
//
//--------------------------------------------------------------

//! \brief Бесконечность в формате double. Сокращение громоздкого вызова из numeric_limits

inline double infinity(){ return numeric_limits<double>::infinity(); }

//!	\brief Вернуть переданное число, если оно больше 0, или 0 в противном случае
template<class T>
inline T positive(const T &x) { return x>0 ? x: 0; }

//!	\brief Вернуть переданное число, если оно меньше 0, или 0 в противном случае
template<class T>
inline T negative(const T &x) { return x<0 ? x: 0; }

//!	\brief Функция "ступенька": 0, если x <= 0, 1, если x > 0
template<class T>
inline T theta(const T &x) { return x>0 ? 1: 0; }

//!	\brief Знак числа: 1, 0, -1
template<class T>
inline T sign(const T &x) { return x==0 ? 0: x>0 ? 1: -1; }

//--------------------------------------------------------------

//!	\brief Квадрат числа
template<class T>
inline T square(const T &x) { return x*x; }

//!	\brief Куб числа
template<class T>
inline T cube(const T &x) { return x*x*x; }

//!	\brief Логарифм по основанию n
inline double logn(const double &x, const double &n) { return log(x)/log(n); }

//--------------------------------------------------------------

// Округление к ближайшему целому; округляет числа вида N+0.5 (N-целое) всегда к N+1
//inline int round(double x) { return int(floor(x+0.5)); }
// Данная функция конфликтует с введенной в C++11 функцией std::round.
// Следует иметь в виду, что работают эти две функции по-разному.
// std::round округляет числа вида N+0.5 в сторону большего _по модулю_ числа
// (т.е. положительные числа в большую сторону, отрицательные в меньшую).
// Следует использовать round_n, iround_n. Эти функции могут иметь оптимизированные реализации.

//! \brief Округление к ближайшему целому (N+0.5 — к четному)
//!
//! \note Числа типа INF, NAN остаются сами собой.
//! \note Структура чисел с плавающей точкой такова, что исключений (нештатных ситуаций)
//! при данной процедуре возникать не может.
template <class TF>
inline TF round_n(TF x)
{
//#pragma STDC FENV_ACCESS ON // не поддерживается в MSVC2015
	static_assert(std::is_floating_point<TF>::value,
			"round_n(x) requires a floating point argument.");
	std::fesetround(FE_TONEAREST);
	return std::nearbyint(x);
}

//! \brief Округление до заданного количества значащих цифр
//!
//! \note Второй аргумент может быть отрицательным, это приводит к обнулению соответствующего количества знаков до запятой.
//! \note round_n(3.14159, 3) даст 3.142
//! \note round_n(123456, -3) даст 123000

template <class TF>
inline TF round_n(TF x, int n)
{
	TF	factor = pow(10, n);
	return round_n(x*factor)/factor;
}



/*!
	\brief Округление к ближайшему целому (N+0.5 — к четному), преобразование к заданному
	целочисленному типу

	Если преобразование проходит без ошибок, заполняется result и возвращается true.
	Если на входе NAN, возвращается false, в result записывается 0.
	Если происходит переполнение при преобразовании в целое,
	возвращается false, в result записывается максимальное значение TI, если x >= 0,
	минимальное значение TI, если x < 0.

	\note В текущей реализации диапазон значений ограничен сверху значением
	numeric_limits<long long>::max(). Таким образом, для TI = unsigned long long (size_t) для чисел
	в диапазоне numeric_limist<long long>::max()+1 ... numeric_limits<unsigned long long>::max()
	будет возвращаться false и result=0, хотя преобразование к целому в этом диапазоне возможно.
*/
template <class TI, class TF>
inline bool try_iround_n(TF x, TI *result)
{
//#pragma STDC FENV_ACCESS ON // не поддерживается в MSVC2015
	static_assert(std::is_integral<TI>::value && std::is_floating_point<TF>::value,
			"try_iround_n<TI>(x) requires a floating point argument (x) and an integral result type (TI).");
	std::fesetround(FE_TONEAREST);
	std::feclearexcept(FE_ALL_EXCEPT);
	long long i_value = std::llrint(x);
	int fe = std::fetestexcept(FE_INVALID|FE_OVERFLOW);
	// Согласно документации, при любой ошибке преобразования выставляется FE_INVALID.
	// FE_OVERFLOW фактически не используется.
	if (fe)
	{
		if (std::isnan(x))
			*result = 0;
		else if (x >= 0)
			*result = numeric_limits<TI>::max();
		else
			*result = numeric_limits<TI>::min();
		return false;
	}
	// 1. Полагаем, что long long является самым широким знаковым целым типом,
	// значения остальных знаковых типов могут быть преобразованы в long long без потерь.
	// 1.1. Значение min() для всех беззнаковых типов == 0 и может быть преобразовано в long long
	// без потерь.
	// 2. Для беззнаковых типов значение max может быть больше max для long long.
	if (i_value < static_cast<long long>(numeric_limits<TI>::min()))
	{
		*result = numeric_limits<TI>::min();
		return false;
	}
	// Если TI беззнаковый, то i_value здесь >= 0, в силу предыдущего if.
	// Поэтому при беззнаковом TI приведение к ulonglong безопасно.
	using compare_type = conditional_t<is_signed<TI>::value, long long, unsigned long long>;
	if (static_cast<compare_type>(i_value) > numeric_limits<TI>::max())
	{
		*result = numeric_limits<TI>::max();
		return false;
	}
	// TODO: Эти две проверки не требуются вообще, если тип TI знаковый и его разрядность совпадает
	// с разрядностью long long. Вторая проверка не нужна также, если TI беззнаковый и его разрядность
	// совпадает с разрядностью long long.

	*result = static_cast<TI>(i_value);
	return true;
}

/*
	\brief Округление к ближайшему целому (N+0.5 — к четному), преобразование к заданному
	целочисленному типу

	\return Значение result функции try_iround_n.
*/
template <class TI, class TF>
inline TI iround_n(TF x)
{
	TI result;
	try_iround_n(x, &result);
	return result;
}

//! \brief Целая часть числа
inline int integral_part(double x) { return int(floor(x)); }

//! \brief Дробная часть числа
inline double fractional_part(double x) { return x - floor(x); }

//--------------------------------------------------------------

/*!
	\brief Вернуть ближайшее не меньшее заданного число, являющееся степенью 2

	Не следует путать эту функцию с \ref ceil_fft_length(int), которая зависиот от реализации бпф алгоритма.
*/
template<class INTEGRAL>
inline INTEGRAL	ceil_power_of_2(INTEGRAL a)
{
	static_assert(is_integral<INTEGRAL>::value, "ceil_power_of_2, only integral argument is allowed");
	//assert_integral_type(INTEGRAL);
	INTEGRAL	b = 1;
	do { b <<= 1; } while(b < a);
	return	b;
}

//! \brief Наибольший общий делитель
template<class INTEGRAL>
inline INTEGRAL	greatest_common_divisor(INTEGRAL x1, INTEGRAL x2)
{
	static_assert(is_integral<INTEGRAL>::value, "greatest_common_divisor, only integral argument is allowed");
	//assert_integral_type(INTEGRAL);

	INTEGRAL	r = min(abs(x1), abs(x2));
	INTEGRAL	s = max(abs(x1), abs(x2));
	INTEGRAL	rn, rn1, rn2;

	rn2 = s%r;
	if(!rn2) return r;

	rn1 = r%rn2;
	if(!rn1) return rn2;

	rn = rn2%rn1;
	if(!rn) return rn1;

	do
	{
		rn2 = rn1;
		rn1 = rn;
		rn = rn2%rn1;
		if(!rn) return rn1;
	} while(true);
}

//!	\brief Факториал
template<class T>
inline T	factorial(int n)
{
	if(n == 0) return 1;
	if(n < 0)
	{
		//ForceDebugBreak();
		throw invalid_argument("T factorial(int), invalid argument");
	}

	T	result = 1;
	for(int i = 1; i <= n; ++i)
	{
		result *= i;
	}
	return result;
}

//--------------------------------------------------------------

//!	\brief Биномиальный коэффициент
inline int	C_nk(int n, int k)
{
	if(k<0) return 0;
	if(k == 0) return 1;

	long long	nominator(1), denominator(1);

	int	n_k = n-k;
	if(n_k < k && n_k>=0) std::swap(n_k, k);// по возможности уменьшаем число умножений

	for(int i = 1; i <= k; ++i)
	{
		denominator *= i;
		nominator *= (n-i+1);
	}
	return int(nominator/denominator);
}



//--------------------------------------------------------------
//
//	некоторые часто употребляемые элементарные функции
//
//--------------------------------------------------------------



//!	\brief Гауссова функция без нормировки: exp(-x*x/(2*s*s))
inline double gauss(double x, double s)
{
	return s ? exp((-x*x)/(2.*s*s)) : 0.;
}

//!	\brief Гауссова функция, нормированная таким образом, чтобы интеграл ее был равен 1: exp(-x*x/(2*s*s))/(s*sqrt(2*pi))
inline double gaussn(double x, double s)
{
	return gauss(x, s)/(sqrt_two_pi()*s);
}

//!	\brief sin(x)/x
inline double	sinc(double x)
{
	return x ? sin(x)/x : 1;
}

//!	\brief Производная sin(x)/x
inline double	sinc_derivative(double x)
{
	return x ? cos(x)/x - sin(x)/(x*x) : 0;
}

//!	\brief Нормированная функция-индикатор интервала (a,b)
inline double rect(double x, double a, double b)
{
	if(b <= a) return 0;
	return in_range(x, a, b) ? 1./(b-a) : 0;
}

//!	\\brief Ненормированная функция-индикатор отрезка (a,b)
inline int indicator(double x, double a, double b)
{
	return in_range(x, a, b) ? 1 : 0;
}

//--------------------------------------------------------------
//
//	децибелы
//
//--------------------------------------------------------------

inline double	amplitude_to_decibel(double a)
{
	return 20.*log10(fabs(a));
}

inline double	power_to_decibel(double p)
{
	return 10.*log10(fabs(p));
}

inline double	decibel_to_amplitude(double db)
{
	return pow(10., db/20.);
}

inline double	decibel_to_power(double db)
{
	return pow(10., db/10.);
}

//--------------------------------------------------------------


//--------------------------------------------------------------
//
//	полиномы
//

//	полином вида sum_n(a_n*x^n)
template<class vector_type, class argument_type, class result_type = argument_type>
result_type	polynom(const vector_type &coefficients, argument_type x)
{
	result_type	result(0);
	argument_type	xpow(1);
	auto it = coefficients.begin(), ie = coefficients.end();

	for(;it<ie;++it)
	{
		result += *it * xpow;
		xpow *= x;
	}
	return result;
}

//	полином вида sum_n(a_n*x^n/n!)
template<class vector_type, class argument_type, class result_type = argument_type>
result_type	polynom_factorial(const vector_type &coefficients, argument_type x)
{
	result_type	result(0);
	argument_type	xpow(1);
	//auto it = coefficients.begin(), ie = coefficients.end();
	int	factorial_factor(0);

	for(auto it = coefficients.begin(); it < coefficients.end(); ++it)
	{
		result += *it * xpow;
		xpow *= x;
		xpow /= ++factorial_factor;
	}
	return result;
}

// флаг использования OpenMP. может быть аргументом функций, определяющим, следует ли использовать omp
enum omp_usage_t
{
	e_use_omp,
	e_dont_use_omp
};


XRAD_END

//! @} <!-- ^group gr_XRAD_Basic -->
#endif // XRAD__basic_utils_h
