#ifndef BooleanSample_h__
#define BooleanSample_h__
//--------------------------------------------------------------
/*!
	\file
	\brief Типы булевой алгебры

	Реализованы в двух вариантах: логический (аналогично встроенному bool,
	но с произвольной разрядностью), а также побитовый.

	Основное назначение этих типов -- не использование их по отдельности (для этого довольно
	встроенного bool), а создание контейнеров-масок, являющихся объектами булевой алгебры.
	Реализацию алгебры контейнерных объектов см. в "AlgebraicStructures1D.h", "...2D.h".

	Важное отличие от встроенного bool в том, что операторы &, |, ^ для логического типа
	реализуют логическую операцию, а не побитовую. По этой причине, во избежание путаницы,
	оператор неявного преобразования к bool для логического типа не реализуется, также как
	смешанные логические операции со встроенным bool.
	Впрочем, операторы &&, || всё же определены.
*/

#include <XRADBasic/Sources/Core/Config.h>
#include <XRADBasic/Sources/Core/BasicMacros.h>
#include <cstdint>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Целочисленный тип для задания аргумента последующих шаблонов
enum class BooleanComplexity
{
	bitwise,
	logical
};

//--------------------------------------------------------------
/*!
	\brief Базовый логический тип

	Содержит переменную и операцию сравнения
	с подобными объектами (аргумент шаблона C должен совпадать).
	Создавать эти объекты нельзя, поэтому все констукторы protected.
*/
template<class T, BooleanComplexity C>
class BooleanGeneric
{
	protected:
		T	datum;
		BooleanGeneric() = default;
		BooleanGeneric(const T&x): datum(x){}

	public:
		typedef T datum_t;

		template<class T2>
		bool	operator==(const BooleanGeneric<T2,C> &x) const {return datum==x.value();}
		template<class T2>
		bool	operator!=(const BooleanGeneric<T2,C> &x) const {return datum!=x.value();}

		T& value(){return datum;}
		const T& value() const {return datum;}
};

//--------------------------------------------------------------

//! \brief Побитовый булев тип
template<class T>
class BooleanBitwise : public BooleanGeneric<T,BooleanComplexity::bitwise>
{
		PARENT(BooleanGeneric<T,BooleanComplexity::bitwise>);
		typedef BooleanBitwise<T> self;

	protected:
		using parent::datum;

	public:
		BooleanBitwise() = default;
		BooleanBitwise(const T& x): parent(x){}

		self	operator&(const self& x) const {return self(datum & x.value());}
		self	operator|(const self& x) const {return self(datum | x.value());}
		self	operator^(const self& x) const {return self(datum ^ x.value());}
		self	operator<<(int n) const {return self(datum << n);}
		self	operator>>(int n) const {return self(datum >> n);}
		self	operator~() const {return self(~datum);}

		self	&operator&=(const self& x){datum &= x.value(); return *this;}
		self	&operator|=(const self& x){datum |= x.value(); return *this;}
		self	&operator^=(const self& x){datum ^= x.value(); return *this;}
		self	&operator<<=(int n){datum <<= n; return *this;}
		self	&operator>>=(int n){datum <<= n; return *this;}

		self	&inverse(){datum = ~datum; return *this;}
};

//--------------------------------------------------------------

//! \brief Логический булев тип
template<class T>
class BooleanLogical : public BooleanGeneric<T,BooleanComplexity::logical>
{
		PARENT(BooleanGeneric<T,BooleanComplexity::logical>);
		typedef BooleanLogical<T> self;
	protected:
		using parent::datum;
	public:
		using parent::value;
	public:
		BooleanLogical() = default;
		BooleanLogical(const T& x): parent(x){}
		template<class T2>
		BooleanLogical(const BooleanLogical<T2>& x): parent(x.value()){}

		template<class T2>
		self	operator&(const BooleanLogical<T2>& x) const {return self(datum && x.value());}
		template<class T2>
		self	operator&&(const BooleanLogical<T2>& x) const {return self(datum && x.value());}

		template<class T2>
		self	operator|(const BooleanLogical<T2>& x) const {return self(datum || x.value());}
		template<class T2>
		self	operator||(const BooleanLogical<T2>& x) const {return self(datum || x.value());}

		template<class T2>
		self	operator^(const BooleanLogical<T2>& x) const {return self(bool(datum) ^ bool(x.value()));}

		template<class T2>
		self	&operator&=(const BooleanLogical<T2>& x){datum = (datum&&x.value()); return *this;}
		template<class T2>
		self	&operator|=(const BooleanLogical<T2>& x){datum = (datum||x.value()); return *this;}
		template<class T2>
		self	&operator^=(const BooleanLogical<T2>& x){datum = (bool(datum)^bool(x.value())); return *this;}

		self	operator!() const {return self(!datum);}
		self	&inverse(){datum = !datum; return *this;}

		bool	operator==(bool x) const {return datum==x;}
		bool	is_true() const {return value();}
		bool	is_false() const {return !value();}
};

//--------------------------------------------------------------

//! \name Вспомогательные функции, используемые в алгоритмах над контейнерами. См. NumberTraits.h
//! @{

//! \brief Компонентная сложность для логической величины
template<class T>
inline const number_complexity::boolean_logical *complexity_t( const BooleanLogical<T> &) {return nullptr;}

//! \brief Компонентная сложность для логической величины
template<class T>
inline number_complexity_e complexity_e(const BooleanLogical<T> &) { return number_complexity_e::boolean_logical; }

//! \brief Компонентная сложность для битовой маски
template<class T>
inline const number_complexity::boolean_bitwise *complexity_t( const BooleanBitwise<T> &) {return nullptr;}

//! \brief Компонентная сложность для битовой маски
template<class T>
inline number_complexity_e complexity_e(const BooleanBitwise<T> &) { return number_complexity_e::boolean_bitwise; }

//! @}

//! \name Доступ к компонентам (используется в алгоритмах над контейнерами)
//! @{

template<class T, BooleanComplexity C>
inline size_t	n_components(const BooleanGeneric<T,C> &) {return 1;}

template<class T, BooleanComplexity C>
inline T &component(BooleanGeneric<T,C> &x, size_t){return x.value();}

template<class T, BooleanComplexity C>
inline const T	&component(const BooleanGeneric<T,C> &x, size_t) {return x.value();}

//! @}

//! \name Нормировочные функции, используемые в алгоритмах над контейнерами
//! @{

template<class T, BooleanComplexity C>
inline double	norma(const BooleanGeneric<T,C> &x) {return x.value();}

template<class T, BooleanComplexity C>
inline double	fast_norma(const BooleanGeneric<T,C> &x) {return x.value();}

template<class T, BooleanComplexity C>
inline double	quadratic_norma(const BooleanGeneric<T,C> &x) {return x.value();}

//! @}

//--------------------------------------------------------------

/*!
	\name Задание типов для непосредственного использования
	@{
	Для логического типа обеспечивают:
	- минимальный размер (8 бит);
	- максимальное быстродействие (совпадает со встроенным int);
	- совпадение со встроенным в систему логическим типом.

	Для побитовых типов используются все доступные размеры целого.
*/

typedef	BooleanLogical<uint8_t> bool8_t;
typedef	BooleanLogical<int> bool_int_t;
typedef	BooleanLogical<bool> bool_native_t;

typedef	BooleanBitwise<uint8_t> bitbool8_t;
typedef	BooleanBitwise<uint16_t> bitbool16_t;
typedef	BooleanBitwise<uint32_t> bitbool32_t;
typedef	BooleanBitwise<uint64_t> bitbool64_t;

//! @}

//--------------------------------------------------------------

XRAD_END

#endif // BooleanSample_h__
