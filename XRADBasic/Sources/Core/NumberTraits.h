//	file number_traits.h
//	Created by ACS on 22.02.02
//--------------------------------------------------------------
#ifndef XRAD__File_number_traits_h
#define XRAD__File_number_traits_h
//--------------------------------------------------------------

#include "Config.h"
#include "BasicMacros.h"
#include <cstdlib>
#include <cstdint>
#include <cmath>

//--------------------------------------------------------------
//
//	здесь следует описывать особенности всех основных типов чисел,
//	с которыми приходится работать. на момент создания речь идет о
//	1. скалярах
//	2. комплексных
//	3. трехкомпонентных цветах
//	4. векторах
//
//	для каждого типа должны быть определены следующие особенности:
//
//	1. "сложность" -- параметр, используемый в операциях ввода-вывода,
//	различает в настоящее время скаляры, комплексные и трехкомпонентные
//	величины. теоретически видится пока возможность "векторной" сложности
//	с непостоянным числом компонент
//
//	2. количество компонент -- связана со "сложностью", но не тождественна ей.
//	возможно, эту "сложность" можно будет со временем упразднить и свести
//	к этому же параметру
//
//	3. доступ к компонентам по их номеру.
//
//	4. алгоритмы вычисления нормы. должно быть задано три алгоритма для каждого
//	случая: евклидова, квадратичная и "быстрая" норма. последняя используется в алгоритмах
//	сравнения, чтобы не вычислять квадратный корень. квадратичная играет вспомогательлную роль.
//	нужна при вычислении
//	евклидовой нормы векторов неопределенной размерности
//
//	5. алгоритмы получения нулевого числа. подробнее описать, см. TODO
//
//	таким образом, для составного класса C, состоящего из компонент типа T должны быть заданы несколько функций (см. ниже):
//
//
//	важно: в этом файле остаются определения только для встроенных типов.
//	для всех остальных определения помещаются сразу вслед за определением класса.
//	это классы:
//
//	ComplexSample
//	ColorSample
//	ColorPixel(частично -- см. комментарий в конце файла ColorSample.h)
//	DataArray<>
//
//--------------------------------------------------------------



XRAD_BEGIN

//TODO два способа оценки сложности элемента. понять, нельзя ли обойтись одним

namespace number_complexity
{
	struct boolean_logical{using tag_t = boolean_logical;};	// единичная булева величина
	struct boolean_bitwise{using tag_t = boolean_bitwise;};	// битовая маска

	struct scalar{using tag_t = scalar;};	// для скалярного числа
	struct complex { using tag_t = complex; };	// для комплексного числа
	struct rgb{ using tag_t = rgb; };	// для трехкомпонентного цвета
	struct rgba{ using tag_t = rgba; };	// для трехкомпонентного цвета с альфа-каналом

	struct array{using tag_t = array;};	// для одномерного массива
	struct array2D{ using tag_t = array2D; };	// для двумерного массива
	struct arrayMD{ using tag_t = arrayMD; };	// для многомерного массива
};

enum number_complexity_e
{
	boolean_logical,
	boolean_bitwise,
	scalar,
	complex,
	rgb,
	array,
	array2D,
	arrayMD
};



//--------------------------------------------------------------
//
// scalar types number traits definitions
//
//--------------------------------------------------------------



#define	define_number_traits_for_scalar(type, abs_function)\
	inline number_complexity_e complexity_e(const type&){ return number_complexity_e::scalar; }\
	inline const number_complexity::scalar *complexity_t( const type &) {return nullptr;}\
	inline size_t	n_components(const type &) {return 1;}\
	inline type	&component(type &x, size_t) {return x;}\
	inline const type &component(const type &x, size_t){return x;}\
	inline void	scalar_product_action(type &result, const type &x, const type &y){result += x*y;}\
	inline double	quadratic_norma(const type &x) {return double(x)*double(x);}\
	inline type	norma(const type &x) {return abs_function(x);}\
	inline type	fast_norma(const type &x) {return abs_function(x);}\
	inline type zero_value(const type&){return (type)(0);}\
	inline void make_zero(type &value){value=(type)(0);}

//NB квадратичная норма, во избежание переполнения, считается через double,
// а не через текущий тип данных
//--------------------------------------------------------------



//--------------------------------------------------------------
//
// действие для скалярного произведения: в общем случае умножение,
// но для комплексных типов -- сопряженное. вместе с объявлением
// комплексных типов будет соответствующая функция
//

//TODO написать комментарий по поводу zero_value и make_zero

#define	check_if_number_traits_defined(...)\
inline void	check_if_number_traits_defined_f(__VA_ARGS__)\
	{\
	typedef __VA_ARGS__ type;\
	type	y;\
	const type x=zero_value(y);\
	\
	number_complexity_e c = complexity_e(x); c;\
	complexity_t(x);\
	n_components(x);\
	component(x,0);\
	component(y,0);\
	quadratic_norma(x);\
	norma(x);\
	fast_norma(x);\
	y=zero_value(x);\
	make_zero(y);\
	}

//	scalar_product_action(y, x, x);
//TODO	исключаем до времени эту проверку scalar_product_action, есть сложности. подумать

define_number_traits_for_scalar(char, abs)
check_if_number_traits_defined(char)

define_number_traits_for_scalar(float, fabs)
define_number_traits_for_scalar(double, fabs)

// Здесь использовать int8_t и т.п. нельзя, т.к. среди них не быть алиаса для int или
// для long, причем неизвестно, для какого именно из них (ср. MSVC и GCC).
// Соответственно, мы не получим ни одного определения для int или для long.
define_number_traits_for_scalar(signed char, abs)
define_number_traits_for_scalar(short, abs)
define_number_traits_for_scalar(int, abs)
define_number_traits_for_scalar(long, labs)
define_number_traits_for_scalar(long long, llabs)

define_number_traits_for_scalar(unsigned char, )
define_number_traits_for_scalar(unsigned short, )
define_number_traits_for_scalar(unsigned int, )
define_number_traits_for_scalar(unsigned long, )
define_number_traits_for_scalar(unsigned long long, )

#undef define_number_traits_for_scalar

check_if_number_traits_defined(float)
check_if_number_traits_defined(double)

check_if_number_traits_defined(signed char)
check_if_number_traits_defined(short)
check_if_number_traits_defined(int)
check_if_number_traits_defined(long)
check_if_number_traits_defined(long long)

check_if_number_traits_defined(unsigned char)
check_if_number_traits_defined(unsigned short)
check_if_number_traits_defined(unsigned int)
check_if_number_traits_defined(unsigned long)
check_if_number_traits_defined(unsigned long long)



XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_number_traits_h
