// file ArrayAnalyzeFunctors.h
//--------------------------------------------------------------
#ifndef __ArrayAnalyzeFunctors_h
#define __ArrayAnalyzeFunctors_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <cstdlib>
#include <cmath>

XRAD_BEGIN

namespace Functors
{

//--------------------------------------------------------------

class find_predicate_max
{
	public:
		template <class T1, class T2>
		static bool test(const T1 &found_value, const T2 &current_value)
		{
			return found_value < current_value;
		}
};

class find_predicate_min
{
	public:
		template <class T1, class T2>
		static bool test(const T1 &found_value, const T2 &current_value)
		{
			return current_value < found_value;
		}
};

//--------------------------------------------------------------

template <class R>
class acquire_sum
{
	public:
		acquire_sum(R *result): result(result) {}

		template <class T>
		void operator() (const T &v) const
		{
			*result += v;
		}
	private:
		R *result;
};

template <class R, class Functor>
class acquire_transformed_sum
{
	public:
		acquire_transformed_sum(R *result, const Functor &functor): result(result), functor(functor) {}

		template <class T>
		void operator() (const T &v) const
		{
			*result += functor(v);
		}
	private:
		R *result;
		Functor functor;
};

//--------------------------------------------------------------

class absolute_value
{
	public:
		short operator() (short v) const { return static_cast<short>(abs(v)); }
		int operator() (int v) const { return abs(v); }
		long operator() (long v) const { return abs(v); }
		long long operator() (long long v) const { return abs(v); }

		unsigned short operator() (unsigned short v) const { return v; }
		unsigned int operator() (unsigned int v) const { return v; }
		unsigned long operator() (unsigned long v) const { return v; }
		unsigned long long operator() (unsigned long long v) const { return v; }

		float operator() (float v) const { return fabs(v); }
		double operator() (double v) const { return fabs(v); }

		template <class T>
		auto operator() (const T &v) const
		{
			return norma(v);
		}
};

//--------------------------------------------------------------

class pow_value
{
	public:
		pow_value(): power(1) {}
		pow_value(double power): power(power) {}
		double operator() (double x) const { return pow(x, power); }
	private:
		double power;
};

//--------------------------------------------------------------

class amplitude_to_decibel_value
{
	public:
		template <class T>
		auto operator() (const T &v) const
		{
			return amplitude_to_decibel(v);
		}
};

class power_to_decibel_value
{
public:
	template <class T>
	auto operator() (const T &v) const
	{
		return power_to_decibel(v);
	}
};

class decibel_to_power_value
{
public:
	template <class T>
	auto operator() (const T &v) const
	{
		return decibel_to_power(v);
	}
};

class decibel_to_amplitude_value
{
public:
	template <class T>
	auto operator() (const T &v) const
	{
		return decibel_to_amplitude(v);
	}
};


//--------------------------------------------------------------

class real_part
{
	public:
		template <class T>
		auto operator() (const T &v) const
		{
			return real(v);
		}
};

class imag_part
{
	public:
		template <class T>
		auto operator() (const T &v) const
		{
			return imag(v);
		}
};

class phasor_value
{
	public:
		template <class T>
		auto operator() (const T &v) const
		{
			return phasor(v);
		}
};

class phasord_value
{
	public:
		template <class T>
		auto operator() (const T &v) const
		{
			return phasord(v);
		}
};

//--------------------------------------------------------------

/*!
	\brief Вспомогательный функтор для поиска в одномерном контейнере

	Predicate — функтор, предоставляющий метод:

	~~~~
	bool test(const T &found_value, const T &examined_value);
	~~~~
	- found_value — Текущее найденное значение. Начальное значение (перед началом поиска)
		задается внешним кодом (который предоставляет указатель nonconst_R *found_value
		в конструктор класса). Значение следует использовать как const.
	- examined_value — Текущее анализируемое значение из контейнера.
	- Возвращаемое значение: true, если found_value должно быть заменено
		значением examined_value, false в противном случае.
*/
template <class Predicate, class R>
class find_iv
{
	public:
		typedef typename std::remove_cv<R>::type nonconst_R;

		/*!
			\param found_index Указатель на переменную, принимающую индекс найденного элемента.
			\param found_value Указатель на переменную, принимеющую значение найденного элемента.
		*/
		find_iv(size_t *found_index, nonconst_R *found_value):
			found_index(found_index), found_value(found_value)
		{
		}
		template <class T>
		void operator() (size_t i, const T &v) const
		{
			if (!Predicate::test(*found_value, v))
				return;
			*found_index = i;
			*found_value = v;
		}
	private:
		size_t *found_index;
		nonconst_R *found_value;
};

//--------------------------------------------------------------

template <class Histogram>
class acquire_histogram_functor
{
	private:
		Histogram *histogram;
		size_t *less_counter;
		size_t *greater_counter;
		double first;
		double last;
		double step;

	public:
		acquire_histogram_functor(Histogram *h,
				size_t *less_counter, size_t *greater_counter,
				double first, double last):
			histogram(h), less_counter(less_counter), greater_counter(greater_counter),
			first(first), last(last), step((last - first) / h->size()) {}

		void operator()(double t) const
		{
			if(t < first)
			{
				++*less_counter;
				return;
			}
			if(t > last)
			{
				++*greater_counter;
				return;
			}
			double f_index = (t - first)/step;
			if (f_index < 0)
			{
				++*less_counter;
				return;
			}
			size_t i_index = floor(f_index);
			if (i_index >= histogram->size())
			{
				++*greater_counter;
				return;
			}
			++(*histogram)[i_index];
		}
};

//--------------------------------------------------------------

} // namespace Functors

XRAD_END

//--------------------------------------------------------------
#endif // __ArrayAnalyzeFunctors_h
