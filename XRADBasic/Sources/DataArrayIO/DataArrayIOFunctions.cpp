//--------------------------------------------------------------
#include "pre.h"

#include "DataArrayIOFunctions.h"
#include <XRADBasic/MathFunctionTypes.h>
#include <cstring>

XRAD_BEGIN

namespace DataArrayIOAuxiliaries
{

//const char *io_to_str(ioNumberOptions dataF); // throw if error
//ioNumberOptions str_to_io(const char *format_name); // return ioInvalid if error

bool is_io_scalar(ioNumberOptions dataF); // false if invalid
bool is_io_complex(ioNumberOptions dataF); // false if invalid


#define define_iotype_string(io_type, str) inline const char *iot_to_str( const io_type*) { return str; }


//
// scalar types
//
define_iotype_string(int8_iotype, "SIGNED_INT8")
define_iotype_string(uint8_iotype, "UNSIGNED_INT8")
//
define_iotype_string(int16_le_iotype, "SIGNED_INT16_LE")
define_iotype_string(int16_be_iotype, "SIGNED_INT16_BE")
define_iotype_string(uint16_le_iotype, "UNSIGNED_INT16_LE")
define_iotype_string(uint16_be_iotype, "UNSIGNED_INT16_BE")
//
define_iotype_string(int32_le_iotype, "SIGNED_INT32_LE")
define_iotype_string(int32_be_iotype, "SIGNED_INT32_BE")
define_iotype_string(uint32_le_iotype, "UNSIGNED_INT32_LE")
define_iotype_string(uint32_be_iotype, "UNSIGNED_INT32_BE")
//
define_iotype_string(float32_le_iotype, "FLOAT32_LE")
define_iotype_string(float32_be_iotype, "FLOAT32_BE")
define_iotype_string(float64_le_iotype, "FLOAT64_LE")
define_iotype_string(float64_be_iotype, "FLOAT64_BE")
//
// complex types
//
define_iotype_string(complexF32_le_iotype, "COMPLEX_FLOAT32_LE")
define_iotype_string(complexF32_be_iotype, "COMPLEX_FLOAT32_BE")
define_iotype_string(complexI32_le_iotype, "COMPLEX_SIGNED_INT32_LE")
define_iotype_string(complexI32_be_iotype, "COMPLEX_SIGNED_INT32_BE")
define_iotype_string(complexI16_le_iotype, "COMPLEX_SIGNED_INT16_LE")
define_iotype_string(complexI16_be_iotype, "COMPLEX_SIGNED_INT16_BE")
define_iotype_string(complexI8_iotype, "COMPLEX_SIGNED_INT8")

// rgb

define_iotype_string(rgbUI8_iotype, "RGB_UNSIGNED_INT8")
define_iotype_string(rgbUI16_le_iotype, "RGB_UNSIGNED_INT16_LE")
define_iotype_string(rgbUI16_be_iotype, "RGB_UNSIGNED_INT16_BE")
define_iotype_string(rgbUI32_le_iotype, "RGB_UNSIGNED_INT32_LE")
define_iotype_string(rgbUI32_be_iotype, "RGB_UNSIGNED_INT32_BE")
define_iotype_string(rgbF32_le_iotype, "RGB_FLOAT32_LE")
define_iotype_string(rgbF32_be_iotype, "RGB_FLOAT32_BE")

// text

define_iotype_string(complex_text_iotype, "COMPLEX_TEXT")
define_iotype_string(scalar_text_iotype, "SCALAR_TEXT")



#undef define_iotype_string


const char *io_to_str(ioNumberOptions dataF)
{
	switch(dataF)
	{
	#define handle_iot_to_str(io, iot) case io: return iot_to_str( (iot*)0 )
	//
	// scalar types
	//
		handle_iot_to_str(ioI8, int8_iotype);
		handle_iot_to_str(ioUI8, uint8_iotype);
		handle_iot_to_str(ioI16_BE, int16_be_iotype);
		handle_iot_to_str(ioI16_LE, int16_le_iotype);
		handle_iot_to_str(ioUI16_LE, uint16_le_iotype);
		handle_iot_to_str(ioUI16_BE, uint16_be_iotype);
		handle_iot_to_str(ioI32_LE, int32_le_iotype);
		handle_iot_to_str(ioI32_BE, int32_be_iotype);
		handle_iot_to_str(ioUI32_LE, uint32_le_iotype);
		handle_iot_to_str(ioUI32_BE, uint32_be_iotype);
		handle_iot_to_str(ioF32_LE, float32_le_iotype);
		handle_iot_to_str(ioF32_BE, float32_be_iotype);
		//
		// complex types
		//
		handle_iot_to_str(ioComplexF32_LE, complexF32_le_iotype);
		handle_iot_to_str(ioComplexI32_LE, complexI32_le_iotype);
		handle_iot_to_str(ioComplexI16_LE, complexI16_le_iotype);
		handle_iot_to_str(ioComplexF32_BE, complexF32_be_iotype);
		handle_iot_to_str(ioComplexI32_BE, complexI32_be_iotype);
		handle_iot_to_str(ioComplexI16_BE, complexI16_be_iotype);
		handle_iot_to_str(ioComplexI8, complexI8_iotype);
		//
		// rgb types
		//
		handle_iot_to_str(ioRGB_UI8, rgbUI8_iotype);
		handle_iot_to_str(ioRGB_UI16_LE, rgbUI16_le_iotype);
		handle_iot_to_str(ioRGB_UI16_BE, rgbUI16_be_iotype);
		handle_iot_to_str(ioRGB_UI32_LE, rgbUI32_le_iotype);
		handle_iot_to_str(ioRGB_UI32_BE, rgbUI32_be_iotype);
		handle_iot_to_str(ioRGB_F32_LE, rgbF32_le_iotype);
		handle_iot_to_str(ioRGB_F32_BE, rgbF32_be_iotype);

		//
		// text formats
		//
		handle_iot_to_str(ioScalarText, scalar_text_iotype);
		handle_iot_to_str(ioComplexText, complex_text_iotype);

	#undef handle_iot_to_str
	//
	// error
	//
		default:
			ForceDebugBreak();
			throw(invalid_argument("io_to_str( ioNumberOptions dataF): Unknown data format!"));
	}
}

//--------------------------------------------------------------

ioNumberOptions str_to_io(const char *format_name)
{
	// внимание! имеющиеся строковые названия типов удалять и менять нельзя, т.к. иначе перестанут читаться файлы данных
	{
	#define handle_str_to_iot(io, iot) if( !strcmp(format_name, iot_to_str((iot*)0))) return io
		//
		// scalar types
		//
		handle_str_to_iot(ioI8, int8_iotype);
		handle_str_to_iot(ioUI8, uint8_iotype);
		handle_str_to_iot(ioI16_BE, int16_be_iotype);
		handle_str_to_iot(ioI16_LE, int16_le_iotype);
		handle_str_to_iot(ioUI16_LE, uint16_le_iotype);
		handle_str_to_iot(ioUI16_BE, uint16_be_iotype);
		handle_str_to_iot(ioI32_LE, int32_le_iotype);
		handle_str_to_iot(ioI32_BE, int32_be_iotype);
		handle_str_to_iot(ioUI32_LE, uint32_le_iotype);
		handle_str_to_iot(ioUI32_BE, uint32_be_iotype);
		handle_str_to_iot(ioF32_LE, float32_le_iotype);
		handle_str_to_iot(ioF32_BE, float32_be_iotype);
		//
		// complex types
		//
		handle_str_to_iot(ioComplexF32_LE, complexF32_le_iotype);
		handle_str_to_iot(ioComplexI32_LE, complexI32_le_iotype);
		handle_str_to_iot(ioComplexI16_LE, complexI16_le_iotype);
		handle_str_to_iot(ioComplexF32_BE, complexF32_be_iotype);
		handle_str_to_iot(ioComplexI32_BE, complexI32_be_iotype);
		handle_str_to_iot(ioComplexI16_BE, complexI16_be_iotype);
		handle_str_to_iot(ioComplexI8, complexI8_iotype);
		//
		// rgb types
		//
		handle_str_to_iot(ioRGB_UI8, rgbUI8_iotype);
		handle_str_to_iot(ioRGB_UI16_LE, rgbUI16_le_iotype);
		handle_str_to_iot(ioRGB_UI16_BE, rgbUI16_be_iotype);
		handle_str_to_iot(ioRGB_UI32_LE, rgbUI32_le_iotype);
		handle_str_to_iot(ioRGB_UI32_BE, rgbUI32_be_iotype);
		handle_str_to_iot(ioRGB_F32_LE, rgbF32_le_iotype);
		handle_str_to_iot(ioRGB_F32_BE, rgbF32_be_iotype);

		//
		// text formats
		//
		handle_str_to_iot(ioScalarText, scalar_text_iotype);
		handle_str_to_iot(ioComplexText, complex_text_iotype);

	#undef handle_str_to_iot
	}
	//
	return ioInvalid;
}

//--------------------------------------------------------------

bool is_io_scalar(ioNumberOptions dataF)
{
	switch((dataF & ioNumberType::mask))
	{
		case ioNumberType::scalar:
//		case ioNumberType::grayscale:
			return true;
	}
	return false;
}

//--------------------------------------------------------------

bool is_io_complex(ioNumberOptions dataF)
{
	return (dataF & ioNumberType::mask) == ioNumberType::complex;
}


size_t	io_sample_size(ioNumberOptions dataF)
{
	switch(dataF)
	{
		case ioI8:
		case ioUI8:
//		case ioGrayscale8:
			return 1;

// 		case ioGrayscale16_LE:
// 		case ioGrayscale16_BE:
		case ioI16_LE:
		case ioI16_BE:
		case ioUI16_LE:
		case ioUI16_BE:
			return 2;
		//
		case ioI32_LE:
		case ioI32_BE:
		case ioUI32_LE:
		case ioUI32_BE:
		//
		case ioF32_LE:
		case ioF32_BE:
			return 4;
		//

		// complex types
		case ioComplexF32_LE:
		case ioComplexF32_BE:
		case ioComplexI32_LE:
		case ioComplexI32_BE:
			return 8;
		// редкость
		case ioComplexI8:
			return 2;

		case ioComplexI16_LE:
		case ioComplexI16_BE:
			return 4;


		// pixel types
		// порядок байтов во всех случаях должен соответствовать
		// стандарту Photoshop RAW (от платформы не зависит)

		case ioRGB_UI8:
			return 3;
		case ioRGBA_UI8_LE:
			return 4;
		case ioRGB_UI16_LE:
			return 6;
		case ioRGB_F32_LE:
			return 12;

		case ioDummyTypeEnumEnd:
		case ioComplexText:
		case ioScalarText:
		default:
			ForceDebugBreak();
			throw runtime_error(ssprintf("io_sample_size(ioNumberOptions): unknown sample size for io type %s", DataArrayIOAuxiliaries::io_to_str(dataF)));
			// для текстового int разрядность не различаем

	};
}

}//namespace DataArrayIOAuxiliaries



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



namespace __DataIO_Unnamed
{

	// раньше здесь было просто unnamed namespace,
	// но GCC дает в этом случае предупреждение не по делу:
	// function "xrad::<unnamed>::f_dummy_check_templates" was declared but never referenced
inline void f_dummy_check_templates(void)
{
	FILE *f = 0;
	ComplexFunctionF32	buffer;
	RealFunctionF32		f_buffer;

	fread_numbers(buffer.begin(), 100, f, ioComplexF32_LE);
	fwrite_numbers(buffer.begin(), 100, f, ioComplexF32_LE);
	fread_numbers(f_buffer.begin(), 100, f, ioF32_LE);
	fwrite_numbers(f_buffer.begin(), 100, f, ioComplexF32_LE);
}
} //namespace __DataIO_Unnamed


//--------------------------------------------------------------

XRAD_END
