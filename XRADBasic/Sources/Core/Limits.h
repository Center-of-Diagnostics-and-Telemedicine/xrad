// file XRAD/Core/Limits.h
//--------------------------------------------------------------
#ifndef __XRAD_Core_Limits_h
#define __XRAD_Core_Limits_h
//--------------------------------------------------------------

//	Внутренний файл библиотеки.

#include "Config.h"
#include "BasicMacros.h"
#include <limits>

XRAD_BEGIN

//--------------------------------------------------------------
//	Константы диапазона значений
//--------------------------------------------------------------

inline double	max_double() {return std::numeric_limits<double>::max();}
inline float	max_float() {return std::numeric_limits<float>::max();}

inline size_t		max_size_t() {return std::numeric_limits<size_t>::max();}
inline ptrdiff_t	max_ptrdiff_t() {return std::numeric_limits<ptrdiff_t>::max();}

inline long		max_long() {	return std::numeric_limits<long>::max();}
inline int		max_int() {return std::numeric_limits<int>::max();}
inline short	max_short() {return std::numeric_limits<short>::max();}
inline short	max_char() {return std::numeric_limits<char>::max();}

inline unsigned long	max_ulong() {return std::numeric_limits<unsigned long>::max();}
inline unsigned int		max_uint() {return std::numeric_limits<unsigned int>::max();}
inline unsigned short	max_ushort() {return std::numeric_limits<unsigned short>::max();}
inline unsigned char	max_uchar() {return std::numeric_limits<unsigned char>::max();}

XRAD_END

//--------------------------------------------------------------
#endif // __XRAD_Core_Limits_h
