﻿#ifndef __MathFunctionGUI2D_h
#define __MathFunctionGUI2D_h

//--------------------------------------------------------------
//
//	created:	2014/05/16
//	created:	16.5.2014   17:02
//	author:		kns
//
//--------------------------------------------------------------

#include <XRADBasic/MathFunctionTypes2D.h>

XRAD_BEGIN

template <class AT>
void DisplayMathFunction2D(const DataArray2D<AT> &image, const string &title,
		const ScanConverterOptions &sco = ScanConverterOptions());

template <class AT>
void DisplayMathFunction2D(const DataArray2D<AT> &image, const wstring &title,
		const ScanConverterOptions &sco = ScanConverterOptions());

XRAD_END

#include "MathFunctionGUI2D.hh"

#endif //__MathFunctionGUI2D_h