// file XRADOpenCL.h
//--------------------------------------------------------------
#ifndef __OpenCL_h
#define __OpenCL_h
//--------------------------------------------------------------

#ifndef XRAD_USE_OPENCL
	#error XRAD_USE_OPENCL must be defined
#endif

#include <string>
#include <type_traits>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

//! \brief Initialize OpenCL computations
void InitOpenCL(const std::string &params = std::string());

//! \brief Uninitialize OpenCL computations
void FinishOpenCL();

//! \brief Check if OpenCL computations are enabled
bool OpenCLEnabled();

//! \brief Free memory occupied by temporary buffers
void FreeOpenCLBuffers();

//--------------------------------------------------------------

//! \brief Internal namespace, not for public use
namespace Implementation
{

void BiexpBlur2D_OpenCL_raw(float *data, size_t vsize, size_t hsize, ptrdiff_t vstep, ptrdiff_t hstep, double radius_v, double radius_h);

template<class AR2D, class B = void>
class BiexpBlur2D_OpenCL_helper
{
	public:
		static bool process(AR2D &data, double radius_v, double radius_h)
		{
			return false;
		}
};

template<class AR2D>
class BiexpBlur2D_OpenCL_helper<AR2D, typename enable_if<std::is_same<typename AR2D::value_type, float>::value, void>::type>
{
	public:
		static bool process(AR2D &data, double radius_v, double radius_h)
		{
			if( !OpenCLEnabled())
				return false;
			ptrdiff_t vstep = data.vstep_raw();
			ptrdiff_t hstep = data.hstep_raw();
			size_t vsize = data.vsize();
			size_t hsize = data.hsize();
			BiexpBlur2D_OpenCL_raw(&data.at(0, 0), vsize, hsize, vstep, hstep, radius_v, radius_h);
			return true;
		}
};

} // namespace Implementation

//--------------------------------------------------------------

/*!
	\brief Perform BiexpBlur2D if possible

	Template function. Returns false for improper data type or if OpenCL is not initialized.
	Currently supported: containers based on float type.

	\return
		- true - result computed
		- false - data not changed, OpenCL computing is impossible
*/
template<class AR2D>
inline bool BiexpBlur2D_OpenCL(AR2D &data, double radius_v, double radius_h)
{
	return Implementation::BiexpBlur2D_OpenCL_helper<AR2D>::process(data, radius_v, radius_h);
}

//--------------------------------------------------------------

XRAD_END

#endif // __OpenCL_h
