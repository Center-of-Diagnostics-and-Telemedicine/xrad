#ifndef XRAD__File_table_function_h
#define XRAD__File_table_function_h

#include <XRADBasic/Sources/Containers/MathFunction.h>

XRAD_BEGIN

template<class MATH_F, class F>
class	TableFunction : public MATH_F
{
private:
	typedef F functor_t;
	PARENT(MATH_F);

public:
	using value_type = typename parent::value_type;
	using iterator = typename parent::iterator;
public:
	using parent::begin;
	using parent::size;
	using parent::at;

	double	x0, dx, x_max;
	functor_t	functor;

	TableFunction(const functor_t &f, size_t size, double in_x0, double in_dx);
	TableFunction(size_t size, double in_x0, double in_dx);

	void	SetFunctor(const functor_t &f);

	inline typename MATH_F::value_type operator()(double x) const;		// для использования в виде функции
	inline typename MATH_F::value_type &operator()(value_type& y, double x) const{ return y = operator()(x); }	// для использования в виде функтора

private:
	value_type*	base;
};




XRAD_END

#include "TableFunction.hh"

#endif //XRAD__File_table_function_h
