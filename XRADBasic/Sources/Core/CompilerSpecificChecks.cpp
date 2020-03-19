// file CompilerSpecificChecks.cpp
//--------------------------------------------------------------
#include "pre.h"
/*!
	\addtogroup gr_CompilerSpecific
	@{

	\file
	\brief Проверка условий компиляции, которые можно проверить универсальным способом

	Здесь проверяются условия компиляции, которые можно проверить универсальным,
	не зависящим от компилятора способом. Эти проверки не требуется выносить
	в заголовочный файл.
*/

#include <climits>

//--------------------------------------------------------------

/*!
	\file
	\par Контроль разрядности
	Проверяется, что разрядность целевой платформы соответствует требованиям библиотеки.
*/
#if CHAR_BIT != 8
	#error The library requires 1 byte = 8 bits
	// Библиотека требует разрядность 1 байт = 8 битов.
#endif

static_assert(sizeof(int) == 4, "The library supports int size 4 only.");
static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8, "The library supports size_t sizes 4 and 8 only.");

namespace xrad
{
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_CompilerSpecificChecks() {}
} // namespace xrad

//--------------------------------------------------------------
//! @} <!-- ^group gr_CompilerSpecific -->
