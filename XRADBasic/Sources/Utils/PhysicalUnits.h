#ifndef XRAD__File_physical_units_h
#define XRAD__File_physical_units_h

#include <XRADBasic/Sources/Core/Config.h>
#include <XRADBasic/Sources/Core/BasicMacros.h>
#include <XRADBasic/Sources/Core/MathConstants.h>
#include <XRADBasic/Sources/Core/NumberTraits.h>
#include <XRADBasic/Sources/Algebra/FieldTraits.h>
#include <ctime>
#include <chrono>

XRAD_BEGIN




//--------------------------------------------------------------
//
//	контейнеры физических величин с контролем единиц измерения
//



template<class UNIT_T, class T = double>
class	physical_unit
{
public:
	typedef UNIT_T unit_type;

private:
	physical_unit(const T &x) : value(x){}

	unit_type &unit_ref(){ return static_cast<unit_type&>(*this); }
	const unit_type &unit_ref() const { return static_cast<const unit_type&>(*this); }


	/// используем явное преобразование к ссылке на дочерний тип. нужно для определяемых
	/// ниже операций вида +=(). дочерний тип отличается от родительского только
	/// методами; все данные такие же; виртуальных функций здесь не используем
	//
	// TODO убран dynamic_cast и полиморфизм, т.к. теоретически может испортить производительность.
	// посмотреть, как работает теперь на разных компиляторах
	// если не будет работать, вернуть динамическое преобразование
	// для корректной работы dynamic_cast понадобится полиморфизм, достигаем этого созданием
	// фиктивной виртуальной функции:
	// virtual	void	make_this_class_polymorphic__(){}

protected:
	T value;

public:
	// вообще-то должно быть protected. однако под gcc не захотела компилироваться. потом лечить
	static	unit_type default_conversion_util(double x){ return physical_unit<unit_type>(x); }
	double	default_inverse_conversion_util() const { return value; }

	static	unit_type conversion_util(double x, double factor){ return physical_unit<unit_type>(x*factor); }
	double	inverse_conversion_util(double factor) const { return value/factor; }

	static	unit_type conversion_util(double x, double factor, double offset){ return physical_unit<unit_type>((x-offset)*factor); }
	double	inverse_conversion_util(double factor, double offset) const { return offset + value/factor; }

public:
	physical_unit(){ value=0; }
	unit_type operator - (void) const { unit_type result(-value); return result; }

	unit_type &operator += (const unit_type &x){ value+=x.value; return unit_ref(); }
	unit_type &operator -= (const unit_type &x){ value-=x.value; return unit_ref(); }
	unit_type &operator *= (double x){ value*=x; return unit_ref(); }
	unit_type &operator /= (double x){ value/=x; return unit_ref(); }

	unit_type operator + (const unit_type &x) const { unit_type result((*this)); result.value += x.value; return result; }
	unit_type operator - (const unit_type &x) const { unit_type result((*this)); result.value -= x.value; return result; }
	unit_type operator * (double x) const { unit_type result((*this)); result.value *= x; return result; }
	unit_type operator / (double x) const { unit_type result((*this)); result.value /= x; return result; }

	double operator / (const unit_type& x) const { return value/x.value; }

	bool	operator == (const unit_type& x) const { return value == x.value; }
	bool	operator != (const unit_type& x) const { return value != x.value; }
	bool	operator >  (const unit_type& x) const { return value > x.value; }
	bool	operator <  (const unit_type& x) const { return value < x.value; }
	bool	operator >= (const unit_type& x) const { return value >= x.value; }
	bool	operator <= (const unit_type& x) const { return value <= x.value; }

	friend	physical_unit<unit_type> absolute_value(const physical_unit<unit_type>& x){ return physical_unit<unit_type>(fabs(x.value)); }
	friend	unit_type operator * (double x, const unit_type &x2) { unit_type result(x2); result.value *= x; return result; }
	friend	double sign(const unit_type &x) { return x.value==0 ? 0: x.value>0 ? 1: -1; }

	friend	inline bool is_number(const physical_unit<UNIT_T> &x)
	{
		if(!xrad::is_number(x.value)) return false;
		return true;
	}

	friend	inline UNIT_T	hypot(const physical_unit<UNIT_T> &x, const physical_unit<UNIT_T> &y)
	{
		UNIT_T	result;
		result.value = std::hypot(x.value, y.value);
		return result;
	}

	friend	inline double	atan2(const physical_unit<UNIT_T> &x, const physical_unit<UNIT_T> &y)
	{
		return std::atan2(x.value, y.value);
	}

};

using std::hypot;
using std::atan2;

#define declare_unit_conversion(unit_name)\
	double	unit_name() const;\
	friend inline unit_type	unit_name(double x);


// определяет преобразование по умолчанию (без умножений). также задает функцию zero_value. кажется, не очень красиво это смешивать.
#define define_default_unit_conversion(unit_type, unit_name)\
	inline	double	unit_type::unit_name() const{return default_inverse_conversion_util();}\
	inline	unit_type	unit_name(double x){return physical_unit<unit_type>::default_conversion_util(x);}\
	\
	inline 	unit_type zero_value(const unit_type&){return unit_name(0);}\
	inline 	void make_zero(unit_type &datum){datum = unit_name(0);}\
	inline	number_complexity_e complexity_e(const unit_type &){return number_complexity_e::scalar;}\
	inline	number_complexity::scalar *complexity_t(const unit_type &){return nullptr;}\
	inline	size_t n_components(const unit_type &){return 1;}\
	inline	const unit_type& component(const unit_type &x, size_t){return x;}\
	inline	unit_type& component(unit_type &x, size_t){return x;}\
	inline	double norma(const unit_type &x){return norma(x.unit_name());}\
	inline	double quadratic_norma(const unit_type &x){return quadratic_norma(x.unit_name());}\
	inline	double fast_norma(const unit_type &x){return fast_norma(x.unit_name());}\
	namespace	AlgebraicStructures{template<> struct FieldTraits<xrad::unit_type> : FieldTagScalar {};}
	//TODO последняя строчка это заплатка для компилятора MSVS 2015 update. ее следует потом удалить

#define define_unit_conversion(unit_type, unit_name, factor)\
	inline double	unit_type::unit_name() const{return inverse_conversion_util(factor);}\
	inline unit_type	unit_name(double x){return physical_unit<unit_type>::conversion_util(x,factor);}


#define define_unit_conversion_with_offset(unit_type, unit_name, factor, offset)\
	inline double	unit_type::unit_name() const{return inverse_conversion_util(factor,offset);}\
	inline unit_type	unit_name(double x){return physical_unit<unit_type>::conversion_util(x,factor, offset);}


//--------------------------------------------------------------
//
//	длина
//

class	physical_length : public physical_unit<physical_length>
{
///internal value is in cm
	PARENT(physical_unit<physical_length>);

public:
	physical_length(parent p) : parent(p){}
	physical_length(){}

	declare_unit_conversion(cm)

		declare_unit_conversion(km)
		declare_unit_conversion(meter)
		declare_unit_conversion(dm)

		declare_unit_conversion(mm)
		declare_unit_conversion(mkm)
		declare_unit_conversion(nm)

		declare_unit_conversion(inch)
		declare_unit_conversion(foot)
};

define_default_unit_conversion(physical_length, cm)

define_unit_conversion(physical_length, km, 1e5)
define_unit_conversion(physical_length, meter, 100)
define_unit_conversion(physical_length, dm, 10)

define_unit_conversion(physical_length, mm, 0.1)
define_unit_conversion(physical_length, mkm, 1e-4)
define_unit_conversion(physical_length, nm, 1e-7)

define_unit_conversion(physical_length, inch, 2.54)
define_unit_conversion(physical_length, foot, 30.48)



//--------------------------------------------------------------
//
//	площадь
//

class	physical_area : public physical_unit<physical_area>
{
///internal value is in square cm
	PARENT(physical_unit<physical_area>);
public:
	physical_area(parent p) : parent(p){}
	physical_area(){}

	declare_unit_conversion(square_cm)

	declare_unit_conversion(square_m)
	declare_unit_conversion(square_meter)
	declare_unit_conversion(square_mm)

	declare_unit_conversion(square_inch)
	declare_unit_conversion(square_ft)
};

define_default_unit_conversion(physical_area, square_cm)

define_unit_conversion(physical_area, square_m, 1e4)
define_unit_conversion(physical_area, square_meter, 1e4)
define_unit_conversion(physical_area, square_mm, 1e-2)

define_unit_conversion(physical_area, square_inch, 6.4516)
define_unit_conversion(physical_area, square_ft, 929.0304)



//--------------------------------------------------------------
//
//	скорость
//

class	physical_speed : public physical_unit<physical_speed>
{
///internal value is in cm/s
	PARENT(physical_unit<physical_speed>);
public:
	physical_speed(){}
	physical_speed(parent p) : parent(p){}

	declare_unit_conversion(cm_sec)

		declare_unit_conversion(mm_sec)
		declare_unit_conversion(m_sec)
		declare_unit_conversion(km_sec)

		declare_unit_conversion(mm_mksec)
		declare_unit_conversion(cm_mksec)
		declare_unit_conversion(km_h)
};

define_default_unit_conversion(physical_speed, cm_sec)

define_unit_conversion(physical_speed, mm_sec, 0.1)
define_unit_conversion(physical_speed, m_sec, 100)
define_unit_conversion(physical_speed, km_sec, 1e5)

define_unit_conversion(physical_speed, mm_mksec, 1e5)
define_unit_conversion(physical_speed, cm_mksec, 1e6)
define_unit_conversion(physical_speed, km_h, 100./3.6)



//--------------------------------------------------------------
//
//	время
//

class	physical_time : public physical_unit<physical_time>
{
///internal value is in sec
	PARENT(physical_unit<physical_time>);
public:
	physical_time(){}
	physical_time(parent p) : parent(p){}

	template <class Rep, class Period>
	physical_time(chrono::duration<Rep, Period> d):
		parent(parent::default_conversion_util(chrono::duration_cast<chrono::duration<double>>(d).count()))
	{
	}

	declare_unit_conversion(sec)

		declare_unit_conversion(msec)
		declare_unit_conversion(mksec)
		declare_unit_conversion(nsec)
		declare_unit_conversion(minute)
		declare_unit_conversion(hour)

		declare_unit_conversion(clocks)
};

define_default_unit_conversion(physical_time, sec)

define_unit_conversion(physical_time, clocks, 1./CLOCKS_PER_SEC)

define_unit_conversion(physical_time, msec, 1e-3)
define_unit_conversion(physical_time, mksec, 1e-6)
define_unit_conversion(physical_time, nsec, 1e-9)
define_unit_conversion(physical_time, minute, 60)
define_unit_conversion(physical_time, hour, 3600)

/*!
	\brief Processor time after program start. Differs in different operating systems (see notes)

	Внимание! Тип счетчика различается в разных операционных системах.
	- В Windows это время соответствует времени UTC.
	- В Linux это время процессора, фактически использованное текущим процессом.
		Т.е. если процесс ожидает ввода-вывода, это время заморожено, значение не меняется.
		Если у процесса 4 потока, производящих вычисления, это время идет в 4 раза
		быстрее UTC.
*/
inline physical_time	current_clock(){ return clocks(clock()); }
inline physical_time	current_time(){ return sec(time(NULL)); }// time after 01.01.1970



//--------------------------------------------------------------
//
//	частота
//

class	physical_frequency : public physical_unit<physical_frequency>
{
///internal value is in Hz
	PARENT(physical_unit<physical_frequency>);
public:
	physical_frequency(){}
	physical_frequency(parent p) : parent(p){}

	declare_unit_conversion(Hz);

	declare_unit_conversion(KHz)
		declare_unit_conversion(MHz)
		declare_unit_conversion(rad_sec)
};

define_default_unit_conversion(physical_frequency, Hz)

define_unit_conversion(physical_frequency, KHz, 1e3)
define_unit_conversion(physical_frequency, MHz, 1e6)
define_unit_conversion(physical_frequency, rad_sec, 1./two_pi()/*0.159154943091895*/)



//--------------------------------------------------------------
//
//	температура
//

class	physical_temperature : public physical_unit<physical_temperature>
{
///internal value is in sec
	PARENT(physical_unit<physical_temperature>);
public:
	physical_temperature(){}
	physical_temperature(parent p) : parent(p){}

	declare_unit_conversion(celsius)

		declare_unit_conversion(kelvin)
		declare_unit_conversion(fahrenheit)
};

define_default_unit_conversion(physical_temperature, celsius)

define_unit_conversion_with_offset(physical_temperature, kelvin, 1, 273.16)
define_unit_conversion_with_offset(physical_temperature, fahrenheit, 5./9, 32)



//--------------------------------------------------------------
//
//	вес
//

class	physical_weight : public physical_unit<physical_weight>
{
///internal value is in gram
	PARENT(physical_unit<physical_weight>);
public:
	physical_weight(){}
	physical_weight(parent p) : parent(p){}

	declare_unit_conversion(gram)
		declare_unit_conversion(kg)
		declare_unit_conversion(mg)
		declare_unit_conversion(mkg)
};

define_default_unit_conversion(physical_weight, gram)
define_unit_conversion(physical_weight, kg, 1e3)
define_unit_conversion(physical_weight, mg, 1e-3)
define_unit_conversion(physical_weight, mkg, 1e-6)



//--------------------------------------------------------------
//
//	сила (заготовка)
//

class	physical_force : public physical_unit<physical_force>
{
///internal value is in newton
	PARENT(physical_unit<physical_force>);
public:
	physical_force(){}
	physical_force(parent p) : parent(p){}

	declare_unit_conversion(newton)
};

define_default_unit_conversion(physical_force, newton)



//--------------------------------------------------------------
//
//	давление  (заготовка)
//

class	physical_pressure : public physical_unit<physical_pressure>
{
///internal value is in Pa
	PARENT(physical_unit<physical_pressure>);
public:
	physical_pressure(){}
	physical_pressure(parent p) : parent(p){}

	declare_unit_conversion(Pa)
};

define_default_unit_conversion(physical_pressure, Pa)



//--------------------------------------------------------------
//
//	угол
//

class	physical_angle : public physical_unit<physical_angle>
{
///internal value is in radians
	PARENT(physical_unit<physical_angle>);
public:
	physical_angle(){}
	physical_angle(parent p) : parent(p){}

	declare_unit_conversion(radians)
		declare_unit_conversion(degrees)
		declare_unit_conversion(grades)
		declare_unit_conversion(turns)

};

define_default_unit_conversion(physical_angle, radians)
define_unit_conversion(physical_angle, degrees, radians_per_degree())
define_unit_conversion(physical_angle, grades, pi()/200)
define_unit_conversion(physical_angle, turns, two_pi())

//--------------------------------------------------------------

#undef declare_unit_conversion
#undef define_default_unit_conversion
#undef define_unit_conversion
#undef define_unit_conversion_with_offset

//--------------------------------------------------------------
//
//	формулы соотношений между единицами, нужно расширить и систематизировать
//

/// длина-площадь
inline physical_area operator * (physical_length x, physical_length y){ return square_cm(x.cm()*y.cm()); }
inline physical_length operator / (physical_area s, physical_length y){ return cm(s.square_cm()/y.cm()); }

/// время-длина-скорость
inline physical_speed operator / (physical_length x, const physical_time t){ return cm_sec(x.cm()/t.sec()); }
inline physical_time operator / (physical_length x, const physical_speed v){ return sec(x.cm()/v.cm_sec()); }

inline physical_length operator * (physical_speed v, physical_time t){ return cm(v.cm_sec()*t.sec()); };
inline physical_length operator * (physical_time t, physical_speed v){ return cm(v.cm_sec()*t.sec()); };

/// частота-время
inline physical_frequency operator / (double n, physical_time t){ return Hz(n/t.sec()); } /// дано число колебаний и время t, определяем частоту
inline physical_time operator / (double n, physical_frequency f){ return sec(n/f.Hz()); } /// дано число колебаний и частота, находим время
inline double operator * (physical_time t, physical_frequency f){ return t.sec()*f.Hz(); } ///дано время и частота, находим число колебаний
inline double operator * (physical_frequency f, physical_time t){ return t.sec()*f.Hz(); } ///дано время и частота, находим число колебаний

/// длина-скорость-частота (например, длина волны, частота и скорость звука)
inline physical_speed operator * (const physical_length x, physical_frequency f){ return cm_sec(x.cm()*f.Hz()); }		//v=lambda*f
inline physical_speed operator * (physical_frequency f, const physical_length x){ return cm_sec(x.cm()*f.Hz()); }		//v=lambda*f
inline physical_length operator / (const physical_speed v, physical_frequency f){ return cm(v.cm_sec()/f.Hz()); }	//lambda=v/f
inline physical_frequency operator / (const physical_speed v, physical_length x){ return Hz(v.cm_sec()/x.cm()); }	//lambda=v/f



//--------------------------------------------------------------
//
//	функции от единиц
//
inline double cosine(physical_angle x){ return ::cos(x.radians()); }
inline double sine(physical_angle x){ return ::sin(x.radians()); }
inline double tangent(physical_angle x){ return ::tan(x.radians()); }

//--------------------------------------------------------------

check_if_number_traits_defined(physical_length)
check_if_number_traits_defined(physical_area)
check_if_number_traits_defined(physical_speed)
check_if_number_traits_defined(physical_frequency)
check_if_number_traits_defined(physical_temperature)
check_if_number_traits_defined(physical_weight)
check_if_number_traits_defined(physical_force)
check_if_number_traits_defined(physical_pressure)
check_if_number_traits_defined(physical_angle)



namespace	AlgebraicStructures
{
/*!
	\brief Перегрузка GetFieldElementTag для physical_unit и его наследников

	Поскольку тип данных в physical_unit — double, возвращаем тег для double.
*/


template<class T>
FieldTraits<double>::tag_t GetFieldElementTag(physical_unit<T>*);
}

XRAD_END

#endif //XRAD__File_physical_units_h
