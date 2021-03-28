/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include <chrono>

XRAD_BEGIN

template<class T>
void	FibonacciRandomGenerator<T>::SeedBuiltInRand()
	{
	srand(chrono::steady_clock::now().time_since_epoch().count());
	}


template<class T>
void	FibonacciRandomGenerator<T>::InitIterators(uint32_t a_offset, uint32_t b_offset)
	{
	std::lock_guard<std::mutex> lock(m_mutex);
	i0 = state_vector.begin();
	ie = state_vector.end();
	// вершина стека всегда итератор a;
	// каждый вызов увеличивает a и b на 1, изменяет значение по a и возвращает результат
	// ie==i0 по модулю state_vector.size(), инициализация в ie-1 означает, что при первом
	// вызове в дело пойдет нулевой элемент статусного вектора

	if(a_offset>b_offset) //для любого итератора it-a_offset==it
		{
		a = ie-1;
		b = ie-1 - b_offset;
		}
	else //для любого итератора it-b_offset==it
		{
		a = ie-1 - a_offset;
		b = ie-1;
		}
	}


template<class T>
void	FibonacciRandomGenerator<T>::InitRandom( uint32_t a_offset, uint32_t b_offset )
	{
	state_vector.realloc(max(a_offset, b_offset));
	RandomizeStateVector();
	InitIterators(a_offset, b_offset);
	}


template<class T>
void	FibonacciRandomGenerator<T>::InitDefinite(const DataArray<T> &in_state_vector, uint32_t a_offset, uint32_t b_offset)
	{
	if(max(b_offset, a_offset) != in_state_vector.size())
		{
		string problem_description = ssprintf("FibonacciRandomGenerator<%s>::Init(const DataArray<T> &in_state_vector, uint32_t a_offset, uint32_t b_offset), a=%d, b=%d, size=%d",
			typeid(T).name(), a_offset, b_offset, in_state_vector.size());
		throw invalid_argument(problem_description);
		}

	state_vector.MakeCopy(in_state_vector);
	InitIterators(a_offset, b_offset);
	}

template<class T>
FibonacciRandomGenerator<T>::FibonacciRandomGenerator(uint32_t a_offset, uint32_t b_offset)
	{
	InitRandom(a_offset, b_offset);
	}

template<class T>
const T &FibonacciRandomGenerator<T>::Generate() const
	{
	std::lock_guard<std::mutex>	lock(m_mutex);
	if(++a==ie) a=i0;
	if(++b==ie) b=i0;

	return GenerateInternal();
	}



XRAD_END