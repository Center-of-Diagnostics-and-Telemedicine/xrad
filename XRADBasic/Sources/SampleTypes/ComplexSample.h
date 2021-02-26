// file ComplexSample.h
//
// A part of XRAD
// Complex numbers library for scientific calculations.
//
// 2014 KNS typedefs complexF/complexD --> complexF32/complexF64
// 2013 KNS class complexF --> template<class PT, class ST> ComplexSample{}
// 1999 Modified by ACS (Алексей Борисович Елизаров) (added constructor with SNoInit)
// 19?? class complexF created by Nicholas S. Kulberg (Кульберг Николай Сергеевич)
//--------------------------------------------------------------
#ifndef XRAD__File_complex_sample_h
#define XRAD__File_complex_sample_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/Core/Config.h>
#include <XRADBasic/Sources/Core/BasicMacros.h>
#include <XRADBasic/Sources/Core/NumberTraits.h>
#include "HomomorphSamples.h"
#include <type_traits>


XRAD_BEGIN



//--------------------------------------------------------------
//
// шаблон ComplexSample<scalar>
// "~" комплексное сопряжение
// "%", "%=" умножение на комплексно сопряженное
//
//--------------------------------------------------------------


// enum	ComplexNumberPart
// рудиментарно используется где-то в фильтре преобразования Гильберта,
// должно уйти
enum	ComplexNumberPart
{
	real_part,
	imag_part
};


/*!
	\brief Комплексное число

	PT должен быть неконстантным.
	Использование ComplexSample<const PT, ST> считаем неправильным.
	Там, где требуется константность, следует использовать const ComplexSample<PT, ST>.
*/
template<class PT, class ST>
class ComplexSample
{
		// Запрещаем использование ComplexSample<const PT, ST> явным образом.
		static_assert(!std::is_const<PT>::value, "Error: Using ComplexSample<PT, ST> with const PT type. PT must not be const. Use const ComplexSample<PT, ST> instead.");
	public:
		typedef	PT part_type;
		typedef	ST scalar_type;
		typedef ComplexSample<PT,ST> self;

	public:
		PT re,im;

	public:
		//	constructors

		ComplexSample(){}
			// конструктор по умолчанию -- без инициализации

		explicit ComplexSample(PT r) {re=r; im=0;}
			// explicit обязательно во избежание
			// случайных появлений комплексных чисел вместо действительных

		ComplexSample(PT r, PT i) {re = r; im = i;}

		template<class PT1, class ST1>
		ComplexSample(const ComplexSample<PT1,ST1> &c) {re = c.re; im = c.im;}

		// assignments

		template<class PT1, class ST1>
		ComplexSample &operator = (const ComplexSample<PT1,ST1> &y) {re = y.re; im = y.im; return *this;}

		ComplexSample<PT,ST> &operator = (PT y) {re = y; im = 0; return *this;}

		// unary arithmetics

		ComplexSample<PT,ST> operator-() const {return ComplexSample<PT,ST>(-re, -im);}
		ComplexSample<PT,ST> operator~() const {return ComplexSample<PT,ST>(re, -im);}

		ComplexSample<PT,ST> &operator++() {++re; return *this;}
		ComplexSample<PT,ST> &operator--() {--re; return *this;}

		ComplexSample<PT,ST> operator++(int) {ComplexSample<PT,ST> result(*this);re++; return result;}
		ComplexSample<PT,ST> operator--(int) {ComplexSample<PT,ST> result(*this);re--; return result;}

		// assignment arithmetic

		template<class PT1, class ST1>
		ComplexSample<PT,ST> &operator += (const ComplexSample<PT1,ST1> &y);

		template<class PT1, class ST1>
		ComplexSample<PT,ST> &operator -= (const ComplexSample<PT1,ST1> &y);

		template<class PT1, class ST1>
		ComplexSample<PT,ST> &operator *= (const ComplexSample<PT1,ST1> &c);

		template<class PT1, class ST1>
		ComplexSample<PT,ST> &operator %= (const ComplexSample<PT1,ST1> &y);

		template<class PT1, class ST1>
		ComplexSample<PT,ST> &operator /= (const ComplexSample<PT1,ST1> &y);
		//
		ComplexSample<PT,ST> &operator += (PT y){re += y; return *this;}
		ComplexSample<PT,ST> &operator -= (PT y){re -= y; return *this;}
		ComplexSample<PT,ST> &operator *= (ST y){re *= y; im *= y; return *this;}
		ComplexSample<PT,ST> &operator /= (ST y){re /= y; im /= y; return *this;}
		//

		// arithmetic

		template<class PT1, class ST1>
		ComplexSample<PT,ST> operator + (const ComplexSample<PT1,ST1> &y) const {return ComplexSample<PT,ST>(*this) += y;}
		template<class PT1, class ST1>
		ComplexSample<PT,ST> operator - (const ComplexSample<PT1,ST1> &y) const {return ComplexSample<PT,ST>(*this) -= y;}
		template<class PT1, class ST1>
		ComplexSample<PT,ST> operator * (const ComplexSample<PT1,ST1> &y) const {return ComplexSample<PT,ST>(*this) *= y;}
		template<class PT1, class ST1>
		ComplexSample<PT,ST> operator % (const ComplexSample<PT1,ST1> &y) const {return ComplexSample<PT,ST>(*this) %= y;}
		template<class PT1, class ST1>
		ComplexSample<PT,ST> operator / (const ComplexSample<PT1,ST1> &y) const {return ComplexSample<PT,ST>(*this) /= y;}
		//
		ComplexSample<PT,ST> operator + (PT y) const {return ComplexSample<PT,ST>(*this) += y;}
		ComplexSample<PT,ST> operator - (PT y) const {return ComplexSample<PT,ST>(*this) -= y;}
		ComplexSample<PT,ST> operator * (ST y) const {return ComplexSample<PT,ST>(*this) *= y;}
		ComplexSample<PT,ST> operator / (ST y) const {return ComplexSample<PT,ST>(*this) /= y;}



		// тернарные действия, результат над двумя аргументами помещается в *this. позволяет избежать
		// создания буферных переменных при операциях вида a=b+c;

		template<class PT1, class ST1>
		ComplexSample<PT,ST>	&conjugate(const ComplexSample<PT1,ST1> &x){re = x.re; im=-im.re; return *this;}

		// действия вида (*this)=x*y с комплексными числами
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&add(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&add_i(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);

		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&subtract(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&subtract_i(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);

		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT,ST>	&multiply(const ComplexSample<PT1,ST1> &x, const ComplexSample<PT2,ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&multiply_conj(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);

		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&divide(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&divide_conj(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);



		// действия вида (*this)+=x*y с комплексными числами
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT,ST>	&add_multiply(const ComplexSample<PT1,ST1> &x, const ComplexSample<PT2,ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&add_multiply_conj(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);

		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT,ST>	&add_divide(const ComplexSample<PT1,ST1> &x, const ComplexSample<PT2,ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&add_divide_conj(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);

		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT,ST>	&subtract_multiply(const ComplexSample<PT1,ST1> &x, const ComplexSample<PT2,ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&subtract_multiply_conj(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);

		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT,ST>	&subtract_divide(const ComplexSample<PT1,ST1> &x, const ComplexSample<PT2,ST2> &y);
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT, ST>	&subtract_divide_conj(const ComplexSample<PT1, ST1> &x, const ComplexSample<PT2, ST2> &y);



		// действия вида (*this)=x*y с комплексным и скаляром
		template<class PT1, class ST1>
		ComplexSample<PT,ST>	&multiply(const ComplexSample<PT1,ST1> &x, const scalar_type &a);

		template<class PT1, class ST1>
		ComplexSample<PT,ST>	&divide(const ComplexSample<PT1,ST1> &x, const scalar_type &a);



		// действия вида (*this)+=x*y с комплексным и скаляром
		template<class PT1, class ST1>
		ComplexSample<PT,ST>	&add_multiply(const ComplexSample<PT1,ST1> &x, const scalar_type &a);

		template<class PT1, class ST1>
		ComplexSample<PT,ST>	&add_divide(const ComplexSample<PT1,ST1> &x, const scalar_type &a);

		template<class PT1, class ST1>
		ComplexSample<PT,ST>	&subtract_multiply(const ComplexSample<PT1,ST1> &x, const scalar_type &a);

		template<class PT1, class ST1>
		ComplexSample<PT,ST>	&subtract_divide(const ComplexSample<PT1,ST1> &x, const scalar_type &a);



		// взвешенное сложение двух массивов, (*this) = x*a1 + y*a2
		template<class PT1, class ST1, class PT2, class ST2>
		ComplexSample<PT,ST>	&mix(const ComplexSample<PT1,ST1> &x, const ComplexSample<PT2,ST2> &y, scalar_type a1, scalar_type a2);



		// compare

		template<class PT1, class ST1>
		bool operator == (const ComplexSample<PT1,ST1> &x) const {return (re == x.re && im == x.im);}
		template<class PT1, class ST1>
		bool operator != (const ComplexSample<PT1,ST1> &x) const {return (re != x.re || im != x.im);}
		template<class PT1, class ST1>
		bool operator < (const ComplexSample<PT1,ST1> &x) const {return (re*re+im*im < x.re*x.re+x.im*x.im);}
		template<class PT1, class ST1>
		bool operator > (const ComplexSample<PT1,ST1> &x) const {return (re*re+im*im > x.re*x.re+x.im*x.im);}
		template<class PT1, class ST1>
		bool operator <= (const ComplexSample<PT1,ST1> &x) const {return (re*re+im*im <= x.re*x.re+x.im*x.im);}
		template<class PT1, class ST1>
		bool operator >= (const ComplexSample<PT1,ST1> &x) const {return (re*re+im*im >= x.re*x.re+x.im*x.im);}
		//
		bool operator == (PT x) const {return (re == x && !im);}
		bool operator != (PT x) const {return (re != x || im);}
		bool operator < (PT x) const {return ((re*re + im*im) < x*fabs(x));}
		bool operator > (PT x) const {return ((re*re + im*im) > x*fabs(x));}
		bool operator <= (PT x) const {return ((re*re + im*im) <= x*fabs(x));}
		bool operator >= (PT x) const {return ((re*re + im*im) >= x*fabs(x));}



		//	определение расстояния между компонентами комплексного числа (вспомогательная функция)
		//	чаще всего оно равно 1, но выравнивание полей внутри структуры может дать и большее значение
		//
		//	функция используется при вычислении шага действительной или мнимой компоненты комплексного массива MathFunctionC.
		//	писать новый шаг как 2*step() нехорошо, т.к., в общем случае не гарантируется, что sizeof(ComplexSample<T>) == 2*sizeof(T).
		//	теоретически это расстояние может оказаться даже некратным размеру T (sizeof(self) != n*sizeof(T));
		//	в таком случае использовать функции real(MathFunctionC) и imag(MathFunctionC) нельзя.
		//	в этом случае при попытке использовать эти функции возникнет ошибка компилятора.
		static ptrdiff_t parts_distance()
		{
			enum
			{
				self_size = sizeof(self),
				part_size = sizeof(part_type),
				distance = self_size/part_size
			};
			static_assert(distance*part_size == self_size, "ComplexSample layout problem.");
				// если компилятор здесь выдаст ошибку, см. комментарий выше

			return distance;
		}
	private:
		template<class PT1, class ST1>
		ComplexSample<PT,ST> complex_division_algorithm(const ComplexSample<PT,ST> x, const ComplexSample<PT1,ST1> y) const;
};



//--------------------------------------------------------------
//
// number traits definitions. see comment in NumberTraits.h
//

template<class PT, class ST>
number_complexity_e complexity_e(const ComplexSample<PT,ST> &){return number_complexity_e::complex;}

template<class PT, class ST>
const number_complexity::complex *complexity_t(const ComplexSample<PT,ST> &) {return nullptr;}

template<class PT, class ST>
size_t	n_components(const ComplexSample<PT,ST> &) {return 2;}

template<class PT, class ST>
PT &component(ComplexSample<PT,ST> &x, size_t n)
{
	if(!n) return x.re;
	return x.im;
}

template<class PT, class ST>
const PT	&component(const ComplexSample<PT,ST> &x, size_t n)
{
	if(!n) return x.re;
	return x.im;
}

template<class PT, class ST>
double	norma(const ComplexSample<PT,ST> &x) {return std::hypot(x.re, x.im);}

template<class PT, class ST>
double	fast_norma(const ComplexSample<PT,ST> &x) {return (x.re*x.re + x.im*x.im);}

template<class PT, class ST>
double	quadratic_norma(const ComplexSample<PT,ST> &x) {return (x.re*x.re + x.im*x.im);}

template<class PT, class ST>
ComplexSample<PT,ST> zero_value(const ComplexSample<PT,ST>&){return ComplexSample<PT,ST>(0);}\

template<class PT, class ST>
void make_zero(ComplexSample<PT,ST> &value){value=ComplexSample<PT,ST>(0);}

//
//--------------------------------------------------------------



//--------------------------------------------------------------
//
//	сопряженное умножение для вычисления
//	скалярного произведения. при использовании комплексных
//	векторов. занимает место универсального шаблона, объявленного
//	в файле NumberTraits.h
//
template<class PT, class PT1, class PT2, class ST, class ST1, class ST2>
void	scalar_product_action(ComplexSample<PT,ST> &result, const ComplexSample<PT1,ST1> &x, const ComplexSample<PT2,ST2> &y)
{
	// сопряженное перемножение компонент комплексных векторов
	result.add_multiply_conj(x,y);
}

template<class PT, class PT1, class ST, class ST1, class ST2>
void	scalar_product_action(ComplexSample<PT,ST> &result, const ComplexSample<PT1,ST1> &x, const ST2 &y)
{
	// умножение комплексного числа на действительное
	result.add_multiply(x,y);
}

template<class PT, class PT1, class ST, class ST1, class ST2>
void	scalar_product_action(ComplexSample<PT,ST> &result, const ST2 &x, const ComplexSample<PT1,ST1> &y)
{
	// умножение действительного числа на комплексное сопряженное
	result.add_multiply(~y,x);
}

template<class PT, class ST, class T1, class T2>
void	scalar_product_action(ComplexSample<PT,ST> &result, const T1 &x, const T2 &y)
{
	// перемножение действительных чисел, запись в комплексный результат. такое тоже иногда бывает нужно
	result += x*y;
}

//
//--------------------------------------------------------------



template<class T, class ST>
inline double	amplitude_to_decibel(const ComplexSample<T,ST> a)
{
	return 10.*log10(cabs2(a));//5*(...cabs2) небольшая экономия на вычислении квадратного корня
}

template<class T, class ST>
inline double	power_to_decibel(const ComplexSample<T,ST> a)
{
	return 5.*log10(cabs2(a));//5*(...cabs2) небольшая экономия на вычислении квадратного корня
}



//--------------------------------------------------------------



typedef	ComplexSample<float, double> complexF32;
typedef	ComplexSample<double, double> complexF64;

typedef	ComplexSample<int32_t, double> complexI32F;
typedef	ComplexSample<int16_t, double> complexI16F;
typedef	ComplexSample<int8_t, double> complexI8F;

typedef	ComplexSample<int32_t, int> complexI32;
typedef	ComplexSample<int16_t, int> complexI16;
typedef	ComplexSample<int8_t, int> complexI8;
// для unsigned определений не задаем, нонсенс



//--------------------------------------------------------------

//! \addtogroup gr_FloatingAnalog
//! @{

template<class T, class ST> struct FloatingAnalog32<ComplexSample<T, ST>, typename enable_if<is_arithmetic_but_bool<T>::value>::type> { typedef complexF32 type; };
template<class T, class ST> struct FloatingAnalog64<ComplexSample<T, ST>, typename enable_if<is_arithmetic_but_bool<T>::value>::type> { typedef complexF64 type; };

//! @} <!-- ^group gr_FloatingAnalog -->
//! \addtogroup gr_ReducedWidth
//! @{

template <class T, class ST>
struct	ReducedWidth<ComplexSample<T, ST>>
{ using type = ComplexSample<typename ReducedWidth<T>::type, ST>; };

//! @} <!-- ^group gr_ReducedWidth -->

//--------------------------------------------------------------

check_if_number_traits_defined(complexF64)



XRAD_END

#include "ComplexSample.hh"

//--------------------------------------------------------------
#endif // XRAD__File_complex_sample_h
