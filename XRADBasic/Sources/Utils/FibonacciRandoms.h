#ifndef FibonacciRandoms_h__
#define FibonacciRandoms_h__

/********************************************************************
	created:	2015/06/23
	created:	23:6:2015   9:44
	author:		kns

	purpose:	генератор псевдослучайных чисел по методу Фибоначчи с запазываниями.
	источник:	https://ru.wikipedia.org/wiki/%D0%9C%D0%B5%D1%82%D0%BE%D0%B4_%D0%A4%D0%B8%D0%B1%D0%BE%D0%BD%D0%B0%D1%87%D1%87%D0%B8_%D1%81_%D0%B7%D0%B0%D0%BF%D0%B0%D0%B7%D0%B4%D1%8B%D0%B2%D0%B0%D0%BD%D0%B8%D1%8F%D0%BC%D0%B8
	некоторые "волшебные пары" из той же статьи:
				(17,5), (55,24), (97,33), (38,89),
				(37,100), (30,127), (83,258),
				(107,378), (273,607), (1029,2281),
				(576,3217), (4178,9689)

*********************************************************************/

#include <XRADBasic/MathFunctionTypes.h>

XRAD_BEGIN


template<class T>
class	FibonacciRandomGenerator
	{
	private:
		typedef DataArray<T>	state_vector_t;
		mutable typename state_vector_t::iterator	i0, ie;
		mutable typename state_vector_t::iterator	a, b;
		state_vector_t	state_vector;

		void	SeedBuiltInRand();
		void	RandomizeStateVector();
		void	InitIterators(uint32_t a_offset, uint32_t b_offset);
		const T& GenerateInternal() const;

		mutable	std::mutex	m_mutex;

	public:

		FibonacciRandomGenerator(){}
		FibonacciRandomGenerator(uint32_t a_offset, uint32_t b_offset);

		void	InitRandom(uint32_t a_offset, uint32_t b_offset);
		void	InitDefinite(const DataArray<T> &in_state, uint32_t a_offset, uint32_t b_offset);

		const T& Generate() const;
		T MaxValue() const;
		T MinValue() const;
	};



XRAD_END

#include "FibonacciRandoms.hh"

#endif // FibonacciRandoms_h__