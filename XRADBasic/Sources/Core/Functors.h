// file Functors.h
//--------------------------------------------------------------
#ifndef __Functors_h
#define __Functors_h
//--------------------------------------------------------------

#include "Config.h"
#include "BasicMacros.h"

XRAD_BEGIN

namespace Functors
{

//--------------------------------------------------------------

class increment
{
	public:
		template <class T>
		void operator() (T &result) const
		{
			++result;
		}
};

//--------------------------------------------------------------

class decrement
{
	public:
		template <class T>
		void operator() (T &result) const
		{
			--result;
		}
};

//--------------------------------------------------------------

class unary_minus_inplace
{
public:
	template <class T>
	void operator() (T &result) const
	{
		result = -result;
	}
};

//--------------------------------------------------------------

class bitwise_not_inplace
{
public:
	template <class T>
	void operator() (T &result) const
	{
		result = ~result;
	}
};

//--------------------------------------------------------------

class assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &&result, T2 &&value) const
		{
			result = value;
		}
};

//--------------------------------------------------------------

class assign_unary_minus
{
	public:
		template <class T1, class T2>
		void operator() (T1 &&result, T2 &&value) const
		{
			result = -value;
		}
};

//--------------------------------------------------------------

class assign_logical_not
{
	public:
		template <class T1, class T2>
		void operator() (T1 &&result, T2 &&value) const
		{
			result = !value;
		}
};

//--------------------------------------------------------------

class assign_bitwise_not
{
	public:
		template <class T1, class T2>
		void operator() (T1 &&result, T2 &&value) const
		{
			result = ~value;
		}
};

//--------------------------------------------------------------

class plus_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result += value;
		}
};

//--------------------------------------------------------------

class minus_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result -= value;
		}
};

//--------------------------------------------------------------

class multiply_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result *= value;
		}
};

//--------------------------------------------------------------

class divide_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result /= value;
		}
};

//--------------------------------------------------------------

class percent_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result %= value;
		}
};

//--------------------------------------------------------------

class bitwise_and_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result &= value;
		}
};

//--------------------------------------------------------------

struct bitwise_or_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result |= value;
		}
};

//--------------------------------------------------------------

struct bitwise_xor_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result ^= value;
		}
};

//--------------------------------------------------------------

class logical_and_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result = result && value;
		}
};

//--------------------------------------------------------------

struct logical_or_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result = result || value;
		}
};

//--------------------------------------------------------------

struct logical_xor_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result = (bool)result ^ (bool)value;
		}
};

//--------------------------------------------------------------

struct shl_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result <<= value;
		}
};

//--------------------------------------------------------------

struct shr_assign
{
	public:
		template <class T1, class T2>
		void operator() (T1 &result, const T2 &value) const
		{
			result >>= value;
		}
};

//--------------------------------------------------------------

class assign_plus
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 + value_2;
		}
};

//--------------------------------------------------------------

class assign_minus
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 - value_2;
		}
};

//--------------------------------------------------------------

class assign_multiply
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 * value_2;
		}
};

//--------------------------------------------------------------

class assign_divide
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 / value_2;
		}
};

//--------------------------------------------------------------

class assign_percent
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 % value_2;
		}
};

//--------------------------------------------------------------

class assign_logical_and
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 && value_2;
		}
};

//--------------------------------------------------------------

class assign_logical_or
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 || value_2;
		}
};

//--------------------------------------------------------------

class assign_logical_xor
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = (bool)value_1 ^ (bool)value_2;
		}
};

//--------------------------------------------------------------

class assign_bitwise_and
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 & value_2;
		}
};

//--------------------------------------------------------------

class assign_bitwise_or
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 | value_2;
		}
};

//--------------------------------------------------------------

class assign_bitwise_xor
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 ^ value_2;
		}
};

//--------------------------------------------------------------

class assign_shl
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 << value_2;
		}
};

//--------------------------------------------------------------

class assign_shr
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result = value_1 >> value_2;
		}
};

//--------------------------------------------------------------

class plus_assign_multiply
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result += value_1 * value_2;
		}
};

//--------------------------------------------------------------

class plus_assign_divide
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result += value_1 / value_2;
		}
};

//--------------------------------------------------------------

class minus_assign_multiply
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result -= value_1 * value_2;
		}
};

//--------------------------------------------------------------

class minus_assign_divide
{
	public:
		template <class R, class T1, class T2>
		void operator() (R &result, const T1 &value_1, const T2 &value_2) const
		{
			result -= value_1 / value_2;
		}
};

//--------------------------------------------------------------

template <class R>
class scalar_product_acquire
{
	public:
		scalar_product_acquire(R *result): result(result) {}

		template <class T1, class T2>
		void operator() (const T1 &value_1, const T2 &value_2) const
		{
			//*result += value_1 * value_2;
			scalar_product_action (*result, value_1, value_2);
		}
	private:
		R *result;
};

//--------------------------------------------------------------

class not_equal
{
	public:
		template <class T1, class T2>
		bool operator() (const T1 &value_1, const T2 &value_2) const
		{
			return value_1 != value_2;
		}
};

//--------------------------------------------------------------

//! \brief Взвешенная сумма mix_functor (a, b); z=f(x,y) то же, что z=ax+by
template <class TA, class TB>
class assign_mix
{
	public:
		template <class TTA, class TTB>
		assign_mix(TTA &&a, TTB &&b): a(a), b(b) {}

		template <class R, class TX, class TY>
		void operator() (R &result, const TX &x, const TY &y) const
		{
			result = a * x + b * y;
		}
	private:
		TA a;
		TB b;
};

template <class TA, class TB>
assign_mix<
		std::remove_cv_t<std::remove_reference_t<TA>>,
		std::remove_cv_t<std::remove_reference_t<TB>>>
	create_assign_mix(TA &&a, TB &&b)
{
	return assign_mix<
			std::remove_cv_t<std::remove_reference_t<TA>>,
			std::remove_cv_t<std::remove_reference_t<TB>>>
			(std::move(a), std::move(b));
}

//--------------------------------------------------------------

class identity
{
	public:
		template <class T>
		auto operator() (T &&x) const
		{
			return std::forward<T>(x);
		}
};

//--------------------------------------------------------------

//! \brief Вспомогательный функтор для \ref assign_f1().
template <class F>
class assign_f1_functor
{
	public:
		assign_f1_functor(const F &f): f(f) {}

		template <class R, class T>
		void operator() (R &result, const T &x) const
		{
			result = f(x);
		}
	private:
		F f;
};

//! \brief Конвертер функтора f(x) в g(r,x):
//!   f(x) =&gt; g(r,x) { r = f(x); }
template <class F>
assign_f1_functor<F> assign_f1(const F &f)
{
	return assign_f1_functor<F>(f);
}

//--------------------------------------------------------------

template <class Functor, class Argument>
struct functor_result_type
{
	using type = typename std::remove_cv<typename std::remove_reference<decltype(declval<Functor>()(declval<Argument>()))>::type>::type;
};

template <class Functor, class Argument>
using functor_result_type_t = typename functor_result_type<Functor, Argument>::type;
// This gives a compiler error (MSVC 2015):
//using functor_result_type_t = typename std::remove_cv<decltype(declval<Functor>()(declval<Argument>()))>::type;

//--------------------------------------------------------------

} // namespace Functors

XRAD_END

//--------------------------------------------------------------
#endif // __Functors_h
