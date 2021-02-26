/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FourierDefs.h
//--------------------------------------------------------------
#ifndef XRAD__File_FourierDefs_h
#define XRAD__File_FourierDefs_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Направление Фурье-преобразования
enum ftDirection
{
	ftForward = 1,
	ftReverse = !ftForward
};

//--------------------------------------------------------------

//! \brief Задание комбинированной обработки (Фурье преобразование и сдвиг)
enum	ft_flags
{
	fftNone = 0,
	fftFwd = 0x01,
	fftRev = 0x02,
	fftDirectionMask = 0x03,
	fftRollBefore = 0x04,
	fftRollAfter = 0x08,

	fftRollBoth = fftRollBefore | fftRollAfter,

	fftFwdRollAfter = fftFwd | fftRollAfter,
	fftFwdRollBefore = fftFwd | fftRollBefore,
	fftRevRollAfter = fftRev | fftRollAfter,
	fftRevRollBefore = fftRev | fftRollBefore,

	fftFwdRollBoth = fftFwd | fftRollBoth,
	fftRevRollBoth = fftRev | fftRollBoth
};

inline ft_flags operator | (ft_flags f1, ft_flags f2)
{
	return static_cast<ft_flags>(int(f1)|int(f2));
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FourierDefs_h
