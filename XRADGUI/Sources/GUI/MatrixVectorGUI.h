#ifndef MatrixVectorGUI_h__
#define MatrixVectorGUI_h__

/********************************************************************
	created:	2016/04/07
	created:	7:4:2016   10:49
	author:		kns
*********************************************************************/

#include "MathFunctionGUI2D.h"
#include "TextDisplayer.h"
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/LinearVectorTypes.h>

XRAD_BEGIN

namespace MatrixVectorGUIAuxiliaries
{

template<class T, class ST>
string	VectorShowText(const DataArray<ComplexSample<T, ST> > &v, double factor)
{
	string	result = "[";
	for (size_t i = 0; i < v.size(); ++i)
	{
		result += ssprintf("(%.2f,%.2f)", double(v[i].re) / factor, double(v[i].im) / factor);
		if (i<v.size() - 1) result += "\t";
	}
	result += "]";
	return result;
}

template<class T>
string	VectorShowText(const DataArray<T> &v, double factor)
	{
	string	result = "[";
	for(size_t i = 0; i < v.size(); ++i)
		{
		result += ssprintf("%.2f", double(v[i])/factor);
		if(i<v.size()-1) result += "\t";
		}
	result += "]";
	return result;
	}


inline string	OrderFactorString(int exponent)
	{
	return ssprintf(" * 10^%d", exponent);
	}
}//namespace MatrixVectorGUIAuxiliaries



template<class VECTOR_T>
void	ShowVector(string title, const VECTOR_T &v, bool is_stopped = true)
	{
	DataArray<typename VECTOR_T::value_type_variable> buffer(v.size());
	std::copy(v.begin(), v.end(), buffer.begin());

	if(v.size() > 10)
		{
//		DataArray<typename VECTOR_T::value_type> buffer(v);
		DisplayMathFunction(buffer, 0, 1, title);
		}
	else
		{
		string	result;
		int	exponent = floor(log10(MaxAbsoluteValue(buffer)));
		if(abs(exponent)<3) exponent = 0;
		double	factor = pow(10., exponent);

		result += MatrixVectorGUIAuxiliaries::VectorShowText(buffer, factor);

		if(exponent) result += MatrixVectorGUIAuxiliaries::OrderFactorString(exponent);

//		ShowText(title, result);
		TextDisplayer	td(title);
		td.SetText(result);
		td.SetFixedWidth(true);
		td.Display(is_stopped);
		}
	}

template<class MATRIX_T>
void	ShowMatrix(string title, const MATRIX_T &m, bool is_stopped = true)
{
		size_t	decision(0);

	while(decision != 2)
	{
		decision = Decide("Matrix display mode", /*3,*/ {"Text", "Graphics", "Exit display"});
		switch(decision)
		{
			case 1:
			{
				DataArray2D<typename MATRIX_T::row_type> buffer(m);
				DisplayMathFunction2D(m, title);
			}
				break;
			case 0:
			{
				string	result;
				int	exponent = floor(log10(MaxAbsoluteValue(m)));
				if(abs(exponent)<3) exponent = 0;
				double	factor = pow(10., exponent);

				for(size_t i = 0; i < m.vsize(); ++i)
				{
					result += MatrixVectorGUIAuxiliaries::VectorShowText(m.row(i), factor);
					if(exponent && i==m.vsize()/2) result += MatrixVectorGUIAuxiliaries::OrderFactorString(exponent);
					result += ssprintf("\n");
				}

//				ShowText(title, result);

				TextDisplayer	td(title);
				td.SetText(result);
				td.SetFixedWidth(true);
				td.Display(is_stopped);
			}
				break;
		}
	}
}

XRAD_END

#endif // MatrixVectorGUI_h__