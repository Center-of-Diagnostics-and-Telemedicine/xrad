#ifndef XRAD__WinogradShortFFT_h
#define XRAD__WinogradShortFFT_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>
#include "FourierDefs.h"

XRAD_BEGIN

namespace WinogradFFT
{

//--------------------------------------------------------------

//! \brief FFT длины 2
template<class COMPLEX_T>
inline void fft_2(COMPLEX_T *v, ptrdiff_t step, ftDirection);

//! \brief FFT длины 2
template<class COMPLEX_T>
inline void fft_2(COMPLEX_T *v, ftDirection);

//--------------------------------------------------------------

//! \brief FFT длины 3
template<class COMPLEX_T>
inline void fft_3(COMPLEX_T *v, ptrdiff_t step, ftDirection);

//! \brief FFT длины 3
template<class COMPLEX_T>
inline void fft_3(COMPLEX_T *v, ftDirection);

//--------------------------------------------------------------

//! \brief FFT длины 4
template<class COMPLEX_T>
inline void fft_4(COMPLEX_T *v, ptrdiff_t step, ftDirection dir);

//! \brief FFT длины 4
template<class COMPLEX_T>
inline void fft_4(COMPLEX_T *v, ftDirection dir);

//--------------------------------------------------------------

//! \brief FFT длины 8
template<class COMPLEX_T>
inline void fft_8(COMPLEX_T *v, ptrdiff_t step, ftDirection direction);

//! \brief FFT длины 8
template<class COMPLEX_T>
inline void fft_8(COMPLEX_T *v, ftDirection direction);

//--------------------------------------------------------------

} //namespace WinogradFFT

XRAD_END

#include "WinogradShortFFT.hh"

//--------------------------------------------------------------
#endif // XRAD__WinogradShortFFT_h
