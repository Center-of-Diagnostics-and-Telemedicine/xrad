//--------------------------------------------------------------
#ifndef __DataArrayIOFunctions_h
#define __DataArrayIOFunctions_h
//--------------------------------------------------------------

#include "DataArrayIOTypes.h"
#include "DataArrayIOEnum.h"

#include <XRADBasic/Core.h>
#include <XRADBasic/Sources/Containers/DataArray.h>
#include <XRADBasic/Sources/Containers/DataArray2D.h>

XRAD_BEGIN

//--------------------------------------------------------------
//
//	функции с типом данных.
//
//--------------------------------------------------------------

//--------------------------------------------------------------
// функции, возвращающие строку-идентификатор, для всех типов данных
// const char *iot_to_str( const io_type* dummy)
// не менять!!! изменения должны быть согласованы с "str_to_io()"
//--------------------------------------------------------------

namespace DataArrayIOAuxiliaries
{

const char *io_to_str(ioNumberOptions dataF); // throw if error
ioNumberOptions str_to_io(const char *format_name); // return ioInvalid if error

bool is_io_scalar(ioNumberOptions dataF); // false if invalid
bool is_io_complex(ioNumberOptions dataF); // false if invalid
	// определяют, является ли тип скалярным, комплексным

size_t	io_sample_size(ioNumberOptions io);

}//namespace DataArrayIOAuxiliaries

XRAD_END

#include "DataArrayIOFunctions.hh"

//--------------------------------------------------------------
//
//	comments
//
//--------------------------------------------------------------

/*------------------------------------------------------

	пример использования:

	ioNumberOptions ioFmt = ioPCInt16;
	complexF64	*ptr;
	int	count;
	FILE	*theFile

	fread_numbers(ptr, count, theFile, ioFmt);

	(то есть, чтение массива комплексных чисел из файла 16-битных целых,
	причем читается только действительная часть, а мнимая приравнивается к нулю)


-------------------------------------------------------*/

//--------------------------------------------------------------

#endif // __DataArrayIOFunctions_h
