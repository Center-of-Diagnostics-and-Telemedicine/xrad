/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_MathFunctionGUIMD_h
#define XRAD__File_MathFunctionGUIMD_h

//--------------------------------------------------------------
//
//	created:	2014/10/10
//	created:	10.10.2014   14:04
//	author:		kns
//
//--------------------------------------------------------------

#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADBasic/Sources/ScanConverter/ScanConverterOptions.h>

XRAD_BEGIN

template<class A3DT>
void	DisplayMathFunction3D(const DataArrayMD<A3DT> &array_md, const wstring &title,
		ScanConverterOptions sco = ScanConverterOptions());

template<class A3DT>
void	DisplayMathFunction3D(const DataArrayMD<A3DT> &array_md, const string &title,
		ScanConverterOptions sco = ScanConverterOptions());

XRAD_END

#include "MathFunctionGUIMD.hh"

#endif //XRAD__File_MathFunctionGUIMD_h