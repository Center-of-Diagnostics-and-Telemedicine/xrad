#ifndef __MathFunctionGUIMD_h
#define __MathFunctionGUIMD_h

//--------------------------------------------------------------
//
//	created:	2014/10/10
//	created:	10.10.2014   14:04
//	author:		kns
//
//--------------------------------------------------------------

#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADBasic/Sources/ScanConverter/ScanConverterOptions.h>
#include "MathFunctionGUI2D.h"

XRAD_BEGIN

template<class A3DT>
void	DisplayMathFunction3D(const DataArrayMD<A3DT> &array_md, const wstring &title,
		ScanConverterOptions sco = ScanConverterOptions());

template<class A3DT>
void	DisplayMathFunction3D(const DataArrayMD<A3DT> &array_md, const string &title,
		ScanConverterOptions sco = ScanConverterOptions());

XRAD_END

#include <MathFunctionGUIMD.hh>

#endif //__MathFunctionGUIMD_h