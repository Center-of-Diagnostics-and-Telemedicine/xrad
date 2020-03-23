/*!
	* \file Tools.cpp
	* \date 26/08/2019 13:10
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#include "pre.h"
#include "XRADDicomTools.h"

//#include <XRADDicom/DCMTKAccess/dcmtkElementsTools.h> //чтобы вытащить инструменты для общего использования

XRAD_BEGIN


namespace Dicom
{

// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_XRADDicomTools() {}

} //end namespace Dicom


XRAD_END