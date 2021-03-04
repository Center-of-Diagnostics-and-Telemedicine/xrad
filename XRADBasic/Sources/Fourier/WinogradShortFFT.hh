/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file WinogradShortFFT.hh
//--------------------------------------------------------------

XRAD_BEGIN

namespace WinogradFFT
{

template <class COMPLEX_T>
COMPLEX_T add_i(const COMPLEX_T &x, const COMPLEX_T &y)
{
	return COMPLEX_T(x.re - y.im, x.im + y.re);
}

template <class COMPLEX_T>
COMPLEX_T subtract_i(const COMPLEX_T &x, const COMPLEX_T &y)
{
	return COMPLEX_T(x.re + y.im, x.im - y.re);
}

//--------------------------------------------------------------
//	преобразование длины 2

template<class COMPLEX_T>
inline void fft_2(COMPLEX_T *v, ptrdiff_t step, ftDirection)
{
	auto b = *v - v[step];
	*v += v[step];
	v[step] = b;
}

//--------------------------------------------------------------

template<class COMPLEX_T>
inline void fft_2(COMPLEX_T *v, ftDirection)
{
	auto b = *v - v[1];
	*v += v[1];
	v[1] = b;
}

//--------------------------------------------------------------
//	преобразование длины 3

const double S_2pi3 = sin(two_pi()/3.);
const double C_2pi31 = cos(two_pi()/3.)-1.;

//--------------------------------------------------------------

template<class COMPLEX_T>
inline void fft_3(COMPLEX_T *v, ptrdiff_t step, ftDirection dir)
{
	auto a1 = v[step] + v[2*step];
	// у Блейхута (стр. 429) опечатка
	// написано:	a1 = v1-v2, a2 = v1+v2
	// должно быть:	a1 = v1+v2, a2 = v1-v2
	// матрица Ђ написана правильно
	auto a2 = (v[step] - v[2*step]) * S_2pi3;
	auto a0 = *v + a1;

	*v = a0;

	auto T0 = a0 + a1 * C_2pi31;

	if(dir == ftForward)
	{
		v[step] = subtract_i(T0, a2);
		v[2*step] = add_i(T0, a2);
	}
	else
	{
		v[step] = add_i(T0, a2);
		v[2*step] = subtract_i(T0, a2);
	}
}

//--------------------------------------------------------------

template<class COMPLEX_T>
inline void fft_3(COMPLEX_T *v, ftDirection dir)
{
	auto a1 = v[1] + v[2];
	// у Блейхута (стр. 429) опечатка
	// написано:	a1 = v1-v2, a2 = v1+v2
	// должно быть:	a1 = v1+v2, a2 = v1-v2
	// матрица Ђ написана правильно
	auto a2 = (v[1] - v[2]) * S_2pi3;
	auto a0 = *v + a1;

	*v = a0;

	auto T0 = a0 + a1 * C_2pi31;

	if(dir == ftForward)
	{
		v[1] = subtract_i(T0, a2);
		v[2] = add_i(T0, a2);
	}
	else
	{
		v[1] = add_i(T0, a2);
		v[2] = subtract_i(T0, a2);
	}
}

//--------------------------------------------------------------
//	преобразование длины 4

template<class COMPLEX_T>
inline void fft_4(COMPLEX_T *v, ptrdiff_t step, ftDirection dir)
{
	auto t0 = *v + v[2*step];
	auto a2 = *v - v[2*step];

	auto t1 = v[step] + v[3*step];
	auto a3 = v[step] - v[3*step];

	*v = t0 + t1;
	v[2*step] = t0 - t1;

	if(dir == ftForward)
	{
		v[step] = subtract_i(a2, a3);
		v[3*step] = add_i(a2, a3);
	}
	else
	{
		v[step] = add_i(a2, a3);
		v[3*step] = subtract_i(a2, a3);
	}
}

//--------------------------------------------------------------

template<class COMPLEX_T>
inline void fft_4(COMPLEX_T *v, ftDirection dir)
{
	auto t0 = *v + v[2];
	auto a2 = *v - v[2];

	auto t1 = v[1] + v[3];
	auto a3 = v[1] - v[3];

	*v = t0 + t1;
	v[2] = t0 - t1;

	if(dir == ftForward)
	{
		v[1] = subtract_i(a2, a3);
		v[3] = add_i(a2, a3);
	}
	else
	{
		v[1] = add_i(a2, a3);
		v[3] = subtract_i(a2, a3);
	}
}

//--------------------------------------------------------------
//	преобразование длины 8

template<class COMPLEX_T>
inline void fft_8(COMPLEX_T *v, ptrdiff_t step, ftDirection direction)
{
	constexpr	double	cos_2PI_8 = 0.707106781186547544;

	auto t0 = *v + v[4*step];
	auto t1 = v[step] + v[5*step];
	auto t2 = v[step] - v[5*step];
	auto t3 = v[2*step] + v[6*step];
	auto t4 = v[3*step] + v[7*step];
	auto t5 = v[3*step] - v[7*step];
	auto t6 = t0 + t3;
	auto t7 = t1 + t4;

	auto a2 = t0 - t3;
	auto a3 = *v - v[4*step];
	auto a4 = (t2 - t5)*cos_2PI_8;
	auto a5 = t1 - t4;
	auto a6 = v[2*step] - v[6*step];

	auto a7 = (t2 + t5)*cos_2PI_8;

	*v = t6 + t7;
	v[4*step] = t6 - t7;

	auto T0 = a3 + a4;
	auto T1 = a3 - a4;
	auto T2 = a6 + a7;
	auto T3 = a6 - a7;

	if(direction == ftForward)
	{
		v[step] = subtract_i(T0, T2);
		v[2*step] = subtract_i(a2, a5);
		v[3*step] = add_i(T1, T3);

		// в книге Блейхута (стр. 432) на этих двух строчках опечатки (перепутаны + и -)
		v[5*step] = subtract_i(T1, T3);
		v[7*step] = add_i(T0, T2);

		v[6*step] = add_i(a2, a5);
	}
	else
	{
		v[step] = add_i(T0, T2);
		v[2*step] = add_i(a2, a5);
		v[3*step] = subtract_i(T1, T3);

		// в книге Блейхута (стр. 432) на этих двух строчках опечатки (перепутаны + и -)
		v[5*step] = add_i(T1, T3);
		v[7*step] = subtract_i(T0, T2);
		//

		v[6*step] = subtract_i(a2, a5);
	}
}

//--------------------------------------------------------------

template<class COMPLEX_T>
inline void fft_8(COMPLEX_T *v, ftDirection direction)
{
	constexpr	double	cos_2PI_8 = 0.707106781186547544;

	auto t0 = *v + v[4];
	auto t1 = v[1] + v[5];
	auto t2 = v[1] - v[5];
	auto t3 = v[2] + v[6];
	auto t4 = v[3] + v[7];
	auto t5 = v[3] - v[7];
	auto t6 = t0 + t3;
	auto t7 = t1 + t4;

	auto a2 = t0 - t3;
	auto a3 = *v - v[4];
	auto a4 = (t2 - t5)*cos_2PI_8;
	auto a5 = t1 - t4;
	auto a6 = v[2] - v[6];

	auto a7 = (t2 + t5)*cos_2PI_8;

	*v = t6 + t7;
	v[4] = t6 - t7;

	auto T0 = a3 + a4;
	auto T1 = a3 - a4;
	auto T2 = a6 + a7;
	auto T3 = a6 - a7;

	if(direction == ftForward)
	{
		v[1] = subtract_i(T0, T2);
		v[2] = subtract_i(a2, a5);
		v[3] = add_i(T1, T3);

		// в книге Блейхута (стр. 432) на этих двух строчках опечатки (перепутаны + и -)
		v[5] = subtract_i(T1, T3);
		v[7] = add_i(T0, T2);

		v[6] = add_i(a2, a5);
	}
	else
	{
		v[1] = add_i(T0, T2);
		v[2] = add_i(a2, a5);
		v[3] = subtract_i(T1, T3);

		// в книге Блейхута (стр. 432) на этих двух строчках опечатки (перепутаны + и -)
		v[5] = add_i(T1, T3);
		v[7] = subtract_i(T0, T2);
		//

		v[6] = subtract_i(a2, a5);
	}
}

//--------------------------------------------------------------

} // namespace WinogradFFT

XRAD_END

//--------------------------------------------------------------
