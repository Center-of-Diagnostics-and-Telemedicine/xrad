//--------------------------------------------------------------
#ifndef XRAD__File_ComplexArrayAnalyzeFunctors_h
#define XRAD__File_ComplexArrayAnalyzeFunctors_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>

XRAD_BEGIN

namespace Functors
{

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

} // namespace Functors

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_ComplexArrayAnalyzeFunctors_h
