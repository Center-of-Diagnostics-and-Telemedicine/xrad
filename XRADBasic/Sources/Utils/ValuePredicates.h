#ifndef value_predicate_h__
#define value_predicate_h__

/*!
	\file
	\date 2018/02/19 17:42
	\author kulberg

	\brief
	Предикаты проверки значения объекта.
	Классы и функции, собранные в этом файле, большого самостоятельного значения не имеют, но могут использоваться как примеры для создания аналогичных фильтров Dicom и для отладки
*/

#include "Predicate.h"

XRAD_BEGIN


namespace predicate
{

//! Предикат, который сравнивает входное значение с заданным значениям. Наследники 5 видов (==, <, >, <=, >=).
//! Эти классы напрямую создавать не следует, только через нижеобъявленные функции equal(T, bool) и т.д.
template<class T, class CHILD>
class value_condition : public condition<T>
{
public:
	using typename condition<T>::value_type;

protected:
	typedef	value_condition<T, CHILD> value_condition_t;
	value_type	mod;
	value_condition(value_type in_mod) : mod(in_mod){}
	virtual	condition<T> *clone() const override { return new CHILD(mod); };
};

//! \brief возвращает true, если проверяемое значение равно заданному
template<class T>
class equal_condition : public value_condition<T, equal_condition<T>>
{
private:
	using parent = value_condition<T, equal_condition<T>>;
protected:
	using parent::mod;
public:
	using typename parent::value_type;
	equal_condition(value_type in_mod) : parent(in_mod){}
private:
	virtual	bool	check(const value_type &m) const override { return mod==m; }
};

//! \brief возвращает true, если проверяемое значение больше заданного
template<class T>
class greater_condition : public value_condition<T, greater_condition<T>>
{
private:
	using parent = value_condition<T, greater_condition<T>>;
protected:
	using parent::mod;
public:
	using typename parent::value_type;
	greater_condition(value_type in_mod) : parent(in_mod){}
private:
	virtual	bool	check(const value_type &m) const override{return m>mod;}
};

//! \brief возвращает true, если проверяемое значение больше или равно заданному
template<class T>
class greater_or_equal_condition : public value_condition<T, greater_or_equal_condition <T>>
{
private:
	using parent = value_condition<T, greater_or_equal_condition <T>>;
protected:
	using parent::mod;
public:
	using typename parent::value_type;
	greater_or_equal_condition(value_type in_mod) : parent(in_mod){}
private:
	virtual	bool	check(const value_type &m) const override { return m>=mod; }
};


//! \brief возвращает true, если проверяемое значение меньше заданного
template<class T>
class less_condition : public value_condition<T, less_condition<T>>
{
private:
	using parent = value_condition<T, less_condition<T>>;
protected:
	using parent::mod;
public:
	using typename parent::value_type;
	less_condition(value_type in_mod) : parent(in_mod){}
private:
	virtual	bool	check(const value_type &m) const override { return m<mod; }
};

//! \brief возвращает true, если проверяемое значение меньше или равно заданному
template<class T>
class less_or_equal_condition : public value_condition<T, less_or_equal_condition<T>>
{
private:
	using parent = value_condition<T, less_or_equal_condition<T>>;
protected:
	using parent::mod;
public:
	using typename parent::value_type;
	less_or_equal_condition(value_type in_mod) : parent(in_mod){}
private:
	virtual	bool	check(const value_type &m) const override { return m<=mod; }
};




//! Пользовательские функции для создания предикатов
template<class T>
checker<T>	equals(T x, bool in_direct = true)
{
	return checker<T>(equal_condition<T>(x), in_direct);
}

template<class T>
checker<T>	greater(T x, bool in_direct = true)
{

	return checker<T>(greater_condition<T>(x), in_direct);
}

template<class T>
checker<T>	less(T x, bool in_direct = true)
{
	return checker<T>(less_condition<T>(x), in_direct);
}

template<class T>
checker<T>	less_or_equal(T x, bool in_direct = true)
{
	return checker<T>(less_or_equal_condition<T>(x), in_direct);
}

template<class T>
checker<T>	greater_or_equal(T x, bool in_direct = true)
{
	return checker<T>(greater_or_equal_condition<T>(x), in_direct);
}

}//namespace predicate

XRAD_END

#endif // value_predicate_h__
