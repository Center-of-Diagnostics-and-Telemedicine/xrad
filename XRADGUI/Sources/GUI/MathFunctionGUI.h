#ifndef __math_functions_interface_h
#define __math_functions_interface_h

// #include <DataArray.h>
#include <XRADBasic/MathFunctionTypes.h>
#include <XRADGUI.h>
#include <GraphSet.h>
#include <ShowComplex.h>
#include <XRADBasic/Sources/SampleTypes/ColorSample.h>

XRAD_BEGIN

struct DisplayTagScalar {};
struct DisplayTagComplex {};
struct DisplayTagRGB {};

template<class T>
typename std::enable_if<std::is_arithmetic<T>::value, DisplayTagScalar>::type GetDisplayTag(T *);

template<class T, class ST>
DisplayTagComplex GetDisplayTag(ComplexSample<T, ST> *);

template<class T>
DisplayTagRGB GetDisplayTag(RGBColorSample<T> *);

template<class T>
auto GetDisplayTag(const T *) -> decltype(GetDisplayTag(declval<T*>()));

namespace NS_DisplayMathFunctionHelpers
{
XRAD_USING;



struct	get_display_value
	{
	double &operator()(double &x, double y) const { return x=double(y); }
//	float &operator()(float &x, double y) const {return x=float(y);}

	template<class T, BooleanComplexity C>
		double &operator()(double &x, const BooleanGeneric<T,C> &y) const { return x=y.value(); }
	template<class T, BooleanComplexity C>
		float &operator()(float &x, const BooleanGeneric<T,C> &y) const { return x=y.value(); }
	};



void	grafreal(const RealFunctionF64 &data, const wstring &title,
		const value_legend &vlegend,
		const axis_legend & xlegend,
		bool bIsStopped = true);

void	grafc(const ComplexFunctionF64 &data, const wstring &title,
		const value_legend &vlegend,
		const axis_legend & xlegend,
		bool bIsStopped = true);

void	grafrgb(const ColorFunctionF64 &data, const wstring &title,
		const value_legend &vlegend,
		const axis_legend & xlegend,
		bool bIsStopped = true);



template<class DISPLAY_TAG>
struct DisplayMathFunctionHelper;

//--------------------------------------------------------------
//
//	шаблон для действительной функции
//

template<>
struct DisplayMathFunctionHelper<DisplayTagScalar>
{
	// Внимание: T может быть const.

// 	template <class T>
// 	static void Display(const DataArray<physical_unit<T>> &mf, double x0, double dx, const wstring &title, const wstring &y_label, const wstring &x_label, bool b_is_stopped = true)

	template <class T>
	static void Display(const DataArray<T> &mf, double x0, double dx, const wstring &title, const wstring &y_label, const wstring &x_label, bool b_is_stopped = true)
	{
		switch(mf.size())
		{
			case 0:
				break;

			case 1:
			{
				double	display_value;
				ShowFloating("Real function contains the only sample:", get_display_value()(display_value, mf.at(0)));
				break;
			}
			default:
			{
				RealFunctionF64 buffer(mf);
				grafreal(buffer, title, value_legend(0, 0, 1, y_label), axis_legend(x0, dx, x_label), b_is_stopped);
			}
		}
	}
};


//--------------------------------------------------------------
//
//	шаблон для функции цветности
//


template<>
struct DisplayMathFunctionHelper<DisplayTagRGB>
{
	// Внимание: T может быть const.
	template <class T>
	static void Display(const DataArray<T> &mf, double x0, double dx, const wstring &title, const wstring &y_label, const wstring &x_label, bool b_is_stopped = true)
	{
		switch(mf.size())
		{
			case 0:
				break;

			case 1:
			{
				ShowString("Color function contains the only sample:", ssprintf("red = %g, green = %g, blue = %g", double(mf.at(0).red()), double(mf.at(0).green()), double(mf.at(0).blue())));
				break;
			}
			default:
			{
				ColorFunctionF64 buffer(mf);

				grafrgb(buffer, title, value_legend(0,0,1,y_label), axis_legend(x0, dx, x_label), b_is_stopped);
			}
		}
	}
};

//--------------------------------------------------------------
//
//	шаблон для комплексной функции
//
//



template<>
struct DisplayMathFunctionHelper<DisplayTagComplex>
{
	// Внимание: T может быть const.
	template <class T>
	static void Display(const DataArray<T> &mf, double x0, double dx, const wstring &title, const wstring &y_label, const wstring &x_label, bool b_is_stopped = true)
	{
		switch(mf.size())
		{
			case 0:
				break;

			case 1:
				ShowComplex("Complex function contains the only sample:", mf.at(0));
				break;

			default:
			{
				ComplexFunctionF64 buffer(mf);
				grafc(buffer, title, value_legend(0,0,1,y_label), axis_legend(x0, dx, x_label), b_is_stopped);
			}
		}
	}
};
}

//--------------------------------------------------------------
//
//	упрощенный универсальный шаблон (меньшее количество аргументов)
//

template <class ARR_T>
void DisplayMathFunction(const ARR_T &mf, double x0, double dx, const wstring &title, const wstring &y_label, const wstring &x_label, bool b_is_stopped = true)
{
	NS_DisplayMathFunctionHelpers::DisplayMathFunctionHelper<decltype(NS_DisplayMathFunctionHelpers::GetDisplayTag(declval<typename ARR_T::value_type*>()))>::Display(
			mf, x0, dx, title, y_label, x_label, b_is_stopped);
}

template <class ARR_T>
void DisplayMathFunction(const ARR_T &mf, double x0, double dx, const string &title, const string &y_label, const string &x_label, bool b_is_stopped = true)
{
	NS_DisplayMathFunctionHelpers::DisplayMathFunctionHelper<decltype(NS_DisplayMathFunctionHelpers::GetDisplayTag(declval<typename ARR_T::value_type*>()))>::Display(
			mf, x0, dx, convert_to_wstring(title), convert_to_wstring(y_label), convert_to_wstring(x_label), b_is_stopped);
}


template <class ARR_T>
void	DisplayMathFunction(const ARR_T &mf, double x0, double dx, const string &title, bool b_is_stopped = true)
{
	DisplayMathFunction(mf, x0, dx, title, "", "", b_is_stopped);
}

template <class ARR_T>
void	DisplayMathFunction(const ARR_T &mf, double x0, double dx, const wstring &title, bool b_is_stopped = true)
{
	DisplayMathFunction(mf, x0, dx, title, L"", L"", b_is_stopped);
}



XRAD_END

#endif //__math_functions_interface_h
