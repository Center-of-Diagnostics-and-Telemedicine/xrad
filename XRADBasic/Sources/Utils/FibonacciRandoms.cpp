/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "FibonacciRandoms.h"
#include <cstdlib>

/********************************************************************
	created:	2015/06/23
	created:	23:6:2015   9:44
	author:		kns
*********************************************************************/

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{
uint16_t	urand16()
{
#if RAND_MAX==0x7FFF
	// старший бит rand() всегда равен нулю. исправляем это, подмешивая дополнительные биты из
	// дополнительного случайного числа.
	// написано небрежно в надежде, что для засева начального статусного вектора пойдет и так.
	// потом можно еще подумать, как сделать лучше.
	return uint16_t(rand()) | ((uint16_t(rand())&0x1u) << 15);
#elif RAND_MAX==0x7FFFFFFF
	return uint16_t(rand());
#else
	// для других встроенных ГПСЧ следует учитывать их особенности.
	// при возникновении ошибки вставить перед #else #elif с проработкой ситуации
	// #else с ошибкой обязательно оставить
#error
#endif
}

uint32_t	urand32()
{
#if RAND_MAX==0x7FFF
	return uint32_t(urand16()) + (uint32_t(urand16())<<16);
#elif RAND_MAX==0x7FFFFFFF
	return uint32_t(rand()) | ((uint32_t(rand())&0x1u) << 31);;
#else
	// для других встроенных ГПСЧ следует учитывать их особенности.
	// при возникновении ошибки вставить перед #else #elif с проработкой ситуации
	// #else с ошибкой обязательно оставить
#error
#endif
}

}


template<>
void	FibonacciRandomGenerator<uint32_t>::RandomizeStateVector()
{
//начальное состояние инициализируем встроенным линейно-конгруэнтным ГПСЧ
	SeedBuiltInRand();
	for(a = state_vector.begin(); a<state_vector.end(); ++a)
	{
		*a = urand32();
	}
}

template<>
const uint32_t	&FibonacciRandomGenerator<uint32_t>::GenerateInternal() const
{
	return const_cast<uint32_t &>(*a) -= *b;
}

template<>
uint32_t	FibonacciRandomGenerator<uint32_t>::MaxValue() const
{
	return numeric_limits<uint32_t >::max();
}

template<>
uint32_t	FibonacciRandomGenerator<uint32_t>::MinValue() const
{
	return numeric_limits<uint32_t >::min();
}


//--------------------------------------------------------------

template<>
void	FibonacciRandomGenerator<uint16_t>::RandomizeStateVector()
{
//начальное состояние инициализируем встроенным линейно-конгруэнтным ГПСЧ
	SeedBuiltInRand();
	for(a = state_vector.begin(); a<state_vector.end(); ++a)
	{
		*a = urand16();
	}
}

template<>
const uint16_t	&FibonacciRandomGenerator<uint16_t>::GenerateInternal() const
{
	return const_cast<uint16_t &>(*a) -= *b;
}

template<>
uint16_t	FibonacciRandomGenerator<uint16_t>::MaxValue() const
{
	return numeric_limits<uint16_t >::max();
}

template<>
uint16_t	FibonacciRandomGenerator<uint16_t>::MinValue() const
{
	return numeric_limits<uint16_t >::min();
}


//--------------------------------------------------------------

template<>
void	FibonacciRandomGenerator<double>::RandomizeStateVector()
{
//начальное состояние инициализируем встроенным линейно-конгруэнтным ГПСЧ
	SeedBuiltInRand();
	for(a = state_vector.begin(); a<state_vector.end(); ++a)
	{
	// инициализируем таким образом, чтобы все биты мантиссы были проинициализированы
		static double	div_1 = 1./double(RAND_MAX);
		static double	div_2 = square(div_1);
		static double	div_3 = cube(div_1);
		*a = double(rand())*div_1 + double(rand())*div_2 + double(rand())*div_3;
	}
}

template<>
const double	&FibonacciRandomGenerator<double>::GenerateInternal() const
{
	const_cast<double &>(*a) -= *b;
	if(*a<0) return ++const_cast<double &>(*a);
	else return *a;
}

template<>
double	FibonacciRandomGenerator<double>::MaxValue() const
{
	return 1;
}

template<>
double	FibonacciRandomGenerator<double>::MinValue() const
{
	return 0;
}


XRAD_END