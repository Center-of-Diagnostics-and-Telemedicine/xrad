#ifndef nifti_datatypes_h__
#define nifti_datatypes_h__

/*!
	\file
	\date 2019/10/21 17:48
	\author kulberg

	\brief  
*/


#include <XRADSystem/ThirdParty/nifti/niftilib/nifti1.h>

XRAD_BEGIN



#define bitsizeof(...) sizeof(__VA_ARGS__)*CHAR_BIT

template<typename T> short	nifti_datatype() = delete;


template<> short nifti_datatype<float>(){ static_assert(bitsizeof(float)==32, "Invalid nifti sample size"); return NIFTI_TYPE_FLOAT32; }

template<> short nifti_datatype<double>(){ static_assert(bitsizeof(double)==64, "Invalid nifti sample size"); return NIFTI_TYPE_FLOAT64; }

template<> short nifti_datatype<uint8_t>(){ return NIFTI_TYPE_UINT8; }
template<> short nifti_datatype<int8_t>(){ return NIFTI_TYPE_INT8; }
template<> short nifti_datatype<char>(){ static_assert(bitsizeof(char)==8, "Invalid nifti sample size"); return NIFTI_TYPE_INT8; }

template<> short nifti_datatype<uint16_t>(){ return NIFTI_TYPE_UINT16; }
template<> short nifti_datatype<int16_t>(){ return NIFTI_TYPE_INT16; }

template<> short nifti_datatype<uint32_t>(){ return NIFTI_TYPE_UINT32; }
template<> short nifti_datatype<int32_t>(){ return NIFTI_TYPE_INT32; }

template<> short nifti_datatype<uint64_t>(){ return NIFTI_TYPE_UINT64; }
template<> short nifti_datatype<int64_t>(){ return NIFTI_TYPE_INT64; }

template<> short nifti_datatype<complexF32>(){ static_assert(bitsizeof(complexF32)==64, "Invalid nifti sample size"); return NIFTI_TYPE_COMPLEX64; }
template<> short nifti_datatype<complexF64>(){ static_assert(bitsizeof(complexF64)==128, "Invalid nifti sample size"); return NIFTI_TYPE_COMPLEX128; }

template<> short nifti_datatype<ColorSampleUI8>(){ static_assert(bitsizeof(ColorSampleUI8)==24, "Invalid nifti sample size"); return NIFTI_TYPE_RGB24; }
template<> short nifti_datatype<ColorPixel>(){ static_assert(bitsizeof(ColorPixel)==32, "Invalid nifti sample size"); return NIFTI_TYPE_RGBA32; }



#if 0
									   /*! 128 bit float = long double. */
#define NIFTI_TYPE_FLOAT128     1536
									   /*! 256 bit complex = 2 128 bit floats */
#define NIFTI_TYPE_COMPLEX256   2048
#endif


template<class CONTAINER_T>
short	nifti_datatype(const CONTAINER_T &)
{
	return nifti_datatype<remove_cv<typename CONTAINER_T::value_type>::type>();
}

template<class CONTAINER_T>
short	nifti_sample_size(const CONTAINER_T &)
{
	return bitsizeof(typename CONTAINER_T::value_type);
}

#undef bitsizeof

enum class nifti_file_type
{
	nii, img_hdr
};




XRAD_END

#endif // nifti_datatypes_h__
