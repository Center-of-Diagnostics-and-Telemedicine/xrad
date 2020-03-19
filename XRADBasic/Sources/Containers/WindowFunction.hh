#ifndef __window_function_cc
#define __window_function_cc

XRAD_BEGIN



//--------------------------------------------------------------
//
//	Window function
//
//--------------------------------------------------------------

template <XRAD__MathFunction_template>
void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, const window_function &w_left, const window_function &w_right, size_t s0, size_t s1)
{
	if(!s1) s1 = function.size();
	if(s1<=s0)
	{
		function.fill(zero_value(function[0]));
		return;
	}
	size_t	s = s1-s0;
	if(s < 3) return;

	auto it = function.begin();
	auto window_start = it + s0;
	auto window_middle = window_start + s/2;
	auto window_end = window_start + s;
	auto it_end = function.end();


	for(; it < window_start; ++it)
	{
		*it = T(0);
	}

	size_t	i = 0;
	for(; it < window_middle; ++it,++i)
	{
		*it *= w_left(i, s);
	}

	for(; it < window_end; ++it,++i)
	{
		*it *= w_right(i, s);
	}

	for(; it < it_end; ++it)
	{
		*it = T(0);
	}
}

template <XRAD__MathFunction_template>
void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, const window_function &win, size_t s0, size_t s1)
{
	ApplyWindowFunction(function, win, win, s0, s1);
}

template <XRAD__MathFunction_template>
void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, const window_function &w_left, const window_function &w_right, size_t s0, size_t s1)
{
	function.fill(T(1));
	ApplyWindowFunction(function, w_left, w_right, s0, s1);
}

template <XRAD__MathFunction_template>
void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, const window_function &win, size_t s0, size_t s1)
{
	function.fill(T(1));
	ApplyWindowFunction(function, win, s0, s1);
}



//--------------------------------------------------------------



template <XRAD__MathFunction_template>
void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, window_function_e ew_left, window_function_e ew_right, size_t s0, size_t s1)
{
	shared_ptr<window_function>	w_left = GetWindowFunctionByEnum(ew_left);
	shared_ptr<window_function>	w_right = GetWindowFunctionByEnum(ew_right);

	ApplyWindowFunction(function, *w_left, *w_right, s0, s1);
}

template <XRAD__MathFunction_template>
void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, window_function_e ewin, size_t s0, size_t s1)
{
	shared_ptr<window_function>	win = GetWindowFunctionByEnum(ewin);
	ApplyWindowFunction(function, *win, s0, s1);
}

template <XRAD__MathFunction_template>
void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, window_function_e ew_left, window_function_e ew_right, size_t s0, size_t s1)
{
	shared_ptr<window_function>	w_left = GetWindowFunctionByEnum(ew_left);
	shared_ptr<window_function>	w_right = GetWindowFunctionByEnum(ew_right);

	CreateWindowFunction(function, *w_left, *w_right, s0, s1);
}

template <XRAD__MathFunction_template>
void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &function, window_function_e ewin, size_t s0, size_t s1)
{
	shared_ptr<window_function>	win = GetWindowFunctionByEnum(ewin);
	CreateWindowFunction(function, *win, s0, s1);
}



XRAD_END

#endif //__window_function_cc
