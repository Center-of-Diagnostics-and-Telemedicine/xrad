#ifndef __MathFunctionGUI2D_h
#define __MathFunctionGUI2D_h

//--------------------------------------------------------------
//
//	created:	2014/05/16
//	created:	16.5.2014   17:02
//	author:		kns
//
//--------------------------------------------------------------

#include <XRADBasic/MathFunctionTypes2D.h>
#include <MathFunctionGUI.h>
#include <XRADBasic/Sources/ScanConverter/ScanConverter.h>
#include <GraphSet.h>

#include <MathFunctionGUI2D.hh>



XRAD_BEGIN

template <class AT>
void DisplayMathFunction2D(const DataArray2D<AT> &image, const string &title, const ScanConverterOptions &sco = ScanConverterOptions())
	{
	MF2DInterfaceAuxiliaries::DisplayMathFunction2DTypeForkHelper(image, convert_to_wstring(title), sco);
	}

template <class AT>
void DisplayMathFunction2D(const DataArray2D<AT> &image, const wstring &title, const ScanConverterOptions &sco = ScanConverterOptions())
	{
	MF2DInterfaceAuxiliaries::DisplayMathFunction2DTypeForkHelper(image, title, sco);
	}


XRAD_END


#endif //__MathFunctionGUI2D_h