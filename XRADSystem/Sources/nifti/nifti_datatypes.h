#ifndef nifti_datatypes_h__
#define nifti_datatypes_h__

/*!
	\file
	\date 2019/10/21 17:48
	\author kulberg

	\brief  
*/


#include <XRADSystem/ThirdParty/nifti/niftilib/nifti1.h>
#include <XRADBasic/Sources/DataArrayIO/DataArrayIOEnum.h>

XRAD_BEGIN


auto	nifti_format_to_io_enum(int16_t nifti_datatype, size_t bitpix)
{
	switch(nifti_datatype)
	{
		case DT_FLOAT32: 
			XRAD_ASSERT_THROW(sizeof(float)*CHAR_BIT==bitpix);
			return ioF32_LE;

		case DT_FLOAT64: 
			XRAD_ASSERT_THROW(sizeof(double)*CHAR_BIT==bitpix);
			return ioF64_LE;

		case DT_UINT8:	
			XRAD_ASSERT_THROW(sizeof(uint8_t)*CHAR_BIT==bitpix);
			return ioUI8;

		case DT_INT8:	
			XRAD_ASSERT_THROW(sizeof(int8_t)*CHAR_BIT==bitpix);
			return ioI8;

		case DT_UINT16:	
			XRAD_ASSERT_THROW(sizeof(uint16_t)*CHAR_BIT==bitpix);
			return ioUI16_LE;

		case DT_INT16:	
			XRAD_ASSERT_THROW(sizeof(int16_t)*CHAR_BIT==bitpix);
			return ioI16_LE;

		case DT_UINT32:	
			XRAD_ASSERT_THROW(sizeof(uint32_t)*CHAR_BIT==bitpix);
			return ioUI32_LE;

		case DT_INT32:	
			XRAD_ASSERT_THROW(sizeof(int32_t)*CHAR_BIT==bitpix);
			return ioI32_LE;

		case DT_UINT64:	
			XRAD_ASSERT_THROW(sizeof(uint64_t)*CHAR_BIT==bitpix);
			return ioUI64_LE;

		case DT_INT64:	
			XRAD_ASSERT_THROW(sizeof(int64_t)*CHAR_BIT==bitpix);
			return ioI64_LE;

		case DT_COMPLEX64:
			XRAD_ASSERT_THROW(sizeof(complexF32)*CHAR_BIT==bitpix);
			return ioComplexF32_LE;

		case DT_COMPLEX128:	
			XRAD_ASSERT_THROW(sizeof(complexF64)*CHAR_BIT==bitpix);
			return ioComplexF64_LE;

		case DT_RGB24: 
			XRAD_ASSERT_THROW(sizeof(ColorSampleUI8)*CHAR_BIT==bitpix);
			return ioRGB_UI8;

		case DT_RGBA32: 
			XRAD_ASSERT_THROW(sizeof(ColorPixel)*CHAR_BIT==bitpix);
			return ioRGBA_UI8_LE;


		case DT_FLOAT128:
		case DT_COMPLEX256:
		default: 
			throw invalid_argument("Unknown or unsupported nifti data format");
	}
}


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
