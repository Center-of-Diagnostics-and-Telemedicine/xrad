#include "pre_GUI.h"

#include <XRADBasic/MathFunctionTypes.h>
#include <MathFunctionGUI.h>
#include <XRADGUI.h>

#include <GraphSet.h>
#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>
#include <XRADBasic/FFT1D.h>

XRAD_BEGIN

namespace NS_DisplayMathFunctionHelpers
{
XRAD_USING

// вспомогательные процедуры получения режимов отображения

complex_function_oscillation AskOscillation()
{
	return Decide(L"Choose oscillation type",
			{
				MakeButton(L"None", no_oscillation),
				MakeButton(L"Positive", positive_oscillation),
				MakeButton(L"Negative", negative_oscillation)
			});
}

window_function_e AskWindowFunction()
{
	auto window_types =
			{
				e_constant_window,
				e_triangular_window,
				e_cos2_window,
				e_hamming_window,
				e_nuttall_window,
				e_blackman_harris_window,
				e_blackman_nuttall_window,
				e_flat_top_window
			};
	vector<Button<window_function_e>> buttons;
	for (auto type: window_types)
	{
		buttons.push_back(MakeButton(GetWindowFunctionName(type), type));
	}
	buttons.push_back(MakeButton(L"View window function", n_window_functions));
	auto choice = GetButtonDecision(L"Choose window function", buttons);
	if (choice != n_window_functions)
		return choice;
	vector<Button<window_function_e>> view_buttons;
	for (auto type: window_types)
	{
		view_buttons.push_back(MakeButton(GetWindowFunctionName(type), type));
	}
	for (;;)
	{
		auto view_choice = Decide(L"View window function", view_buttons);
		SafeExecute([&]()
			{
				auto window_generator = GetWindowFunctionByEnum(view_choice);
				RealFunctionF32 window_function(1024+1);
				for (size_t i = 0; i < window_function.size(); ++i)
				{
					window_function[i] = (*window_generator)(double(i) / window_function.size());
				}
				DisplayMathFunction(window_function, 0, 1,
						ssprintf("Window function: %s",
								EnsureType<const char*>(GetWindowFunctionName(view_choice).c_str())));
			});
		enum { c_use, c_another, c_cancel };
		auto func_choice = GetButtonDecision(L"Window function",
				{
					MakeButton(L"Use this function", c_use),
					MakeButton(L"View another function", c_another),
					MakeButton(L"Cancel", c_cancel)
				});
		switch (func_choice)
		{
			case c_cancel:
				throw canceled_operation("Window function selection canceled.");
			case c_use:
				return view_choice;
		}
	}
}


// вспомогательные процедуры получения компонентов комплексной функции




ComplexFunctionF64 Derivative(const ComplexFunctionF64 &f, complex_function_oscillation osc)
{
	ComplexFunctionF64	result(f.size());
	for(size_t i=0; i<f.size(); ++i) result[i] = f.d_dx(i, osc);
	return	result;
}




RealFunctionF64	PhaseDerivative(const ComplexFunctionF64 &f, complex_function_oscillation osc)
{
	size_t	s = f.size();
	size_t	interpolation_factor = 128;
	RealFunctionF64	buffer(interpolation_factor);

	complexF64 previous = f.in(0, osc);

	RealFunctionF64 result(s);

	for(size_t i = 0; i < s; ++i)
	{
		for(size_t j = 0; j < interpolation_factor; ++j)
		{
			double	x = i + double(j)/interpolation_factor;
			complexF64 current = f.in(x, osc);
			buffer[j] = arg(current%previous);
			previous=current;
		}
		result[i] = AverageValue(buffer)*interpolation_factor;
	}

	return	result;
}




RealFunctionF64 LinearMagnitude(const ComplexFunctionF64 &f)
{
	RealFunctionF64 result(f.size());
	CopyData(result, f, Functors::assign_f1(Functors::absolute_value()));
	return	result;
}



RealFunctionF64 LogMagnitude(const ComplexFunctionF64 &f, double minDB)
{
	RealFunctionF64 result(f.size());

	CopyData(result, f, [](double &r, const complexF64 &v) { r = cabs2(v); });
//	double	max_value = MaxValue(result);

	auto	it = result.begin();
	for(size_t i=0; i<f.size(); ++it, ++i) // normalized dB's
	{
		double	value = *it;
		if(value)
		{
// делалась нормировка, максимальное значение устанавливалось в 0 дБ. неинформативно.
// отменено, показывается логарифм как есть. 2016_04_11 KNS
//			*it = max(10.0*log10(value/max_value), -minDB);
			*it = max(10.0*log10(value), -minDB);
		}
		else
		{
			*it = -minDB;
		}
	}
	return	result;
}


// RealFunctionF64 LogMagnitudeAbs(const ComplexFunctionF64 &f, double dynamic_range_min, double dynamic_range_max)
// {
// 	RealFunctionF64 result(f.size());
// 	CopyData(result, f, cabs_functor<double, complexF64>());
//
// 	for(size_t i=0; i<f.size(); i++)
// 	{
// 		if(!result[i])
// 			result[i] = dynamic_range_min;
// 		else
// 		{
// 			result[i] = range(20.0*log10(result[i]), dynamic_range_min, dynamic_range_max);
// 		}
// 	}
// 	return	result;
// }




RealFunctionF64 PhaseContinued(const ComplexFunctionF64 &f, complex_function_oscillation osc)
{
	RealFunctionF64 result(PhaseDerivative(f, osc));

	result[0] = arg(f[0]);
	for(size_t i=1; i<f.size(); i++)
	{
		result[i] += result[i-1];
	}
	return	result;
}


RealFunctionF64 PhaseCut(const ComplexFunctionF64 &f)
{
	RealFunctionF64 result(f.size());
	CopyData(result, f, [](double &r, const complexF64 &v) { r = arg(v); });
	return	result;
}

void	grafreal(const RealFunctionF64 &mf, const wstring &title,
		const value_legend &vlegend,
		const axis_legend & xlegend,
		bool b_is_stopped)
{
	DataArray<double> buffer;
	MakeCopy(buffer, mf, get_display_value());
	GraphSet	gs(title, vlegend.label, xlegend.label);
	gs.AddGraphUniform(buffer, xlegend.min_value, xlegend.step, title);
	gs.Display(b_is_stopped);

}


void	grafc(const ComplexFunctionF64 &data, const wstring &data_title,
		const value_legend &vlegend,
		const axis_legend &xlegend,
		bool)
{
	bool	inverse_ft_direction = false;
	bool	roll_data_before_ft = false;
	bool	roll_spectrum_after_ft = false;
	bool	upsample_fragment = false;

	bool	phase_radians = true;
	bool	phase_continuous = true;

	bool	log_magnitude = true;
	static const double	log_magnitude_dynamic_range = 2000;

	bool	derivative_per_samples = false;

	wstring option_title = ssprintf(L"Display '%s'", data_title.c_str());

	for (;;)
	{
		enum	display_options
		{
			linear_magnitude_option,
			real_option,
			log_magnitude_option,
			phase_cut_option,

			phase_deriv_option,
			roll_option,
			cancel_phase_option,
			spectrum_option,

			polar_option,

			real_imag_option,
			imag_option,
			magnitude_phase_option,
			phase_continued_option,

			derivative_option,
			flip_phase_option,
			cancel_magnitude_option,
			fragment_option,

			cancel_option
		};
		auto option = GetButtonDecision(option_title,
				{
					MakeButton(L"Linear magnitude", linear_magnitude_option),
					MakeButton(L"Real part", real_option),
					MakeButton(L"Log. magnitude", log_magnitude_option),
					MakeButton(L"Phase cut", phase_cut_option),

					MakeButton(L"Phase derivative", phase_deriv_option),
					MakeButton(L"Roll data ->", roll_option),
					MakeButton(L"Cancel phase ->", cancel_phase_option),
					MakeButton(L"Spectrum ->", spectrum_option),

					MakeButton(L"Real/Imaginary (polar)", polar_option),

					MakeButton(L"Real/Imaginary", real_imag_option),
					MakeButton(L"Imaginary part", imag_option),
					MakeButton(L"Magnitude/Phase", magnitude_phase_option),
					MakeButton(L"Phase continued", phase_continued_option),

					MakeButton(L"Derivative ->", derivative_option),
					MakeButton(L"Flip phase ->", flip_phase_option),
					MakeButton(L"Cancel magnitude ->", cancel_magnitude_option),
					MakeButton(L"Fragment ->", fragment_option),

					MakeButton(L"Exit display", cancel_option)
				});
		if (option == cancel_option)
			break;

		SafeExecute([&]()
			{
				complex_function_oscillation phase_oscillation = no_oscillation;

				switch(option)
				{
					case log_magnitude_option:
						log_magnitude = true;
						break;
					case linear_magnitude_option:
						log_magnitude = false;
						break;
					case magnitude_phase_option:
					case cancel_phase_option:
						log_magnitude = Decide2("Magnitude display mode", "Linear", "Log", log_magnitude? 1: 0) == 1;
						break;
				}


				if(option == magnitude_phase_option || option == phase_cut_option ||
						option == phase_continued_option || option == cancel_magnitude_option || option == phase_deriv_option)
				{
					switch(option)
					{
						case magnitude_phase_option:
						case cancel_magnitude_option:
							phase_continuous = Decide2("Phase compute algorithm", "Cut", "Continuous", phase_continuous? 1: 0) == 1;
							break;

						case phase_cut_option:
							phase_continuous = false;
							break;

						case phase_continued_option:
							phase_continuous = true;
							break;

						case phase_deriv_option:
							derivative_per_samples = Decide2("Derivative argument", "Natural", "Sample", derivative_per_samples? 1: 0) == 1;
							phase_continuous = true;
							break;
					};
					phase_oscillation = phase_continuous ? AskOscillation() : no_oscillation;
					phase_radians = Decide2("Phase units", "Degrees", "Radians", phase_radians? 1: 0) == 1;
				}

				if(option==derivative_option)
				{
					phase_oscillation = AskOscillation();
					derivative_per_samples = Decide2("Derivative argument", "Natural", "Sample", derivative_per_samples? 1: 0) == 1;
				}

				wstring	derivative_divisor_label = derivative_per_samples ? L"/sample" : wstring(L"/") + (xlegend.label.size() ? xlegend.label:L"x unit");
				wstring	phase_unit_label = phase_radians ? L"radians" : L"degrees";
				double	phase_factor = phase_radians ? 1. : 180./pi();
				wstring	magnitude_unit_label = log_magnitude ? L"dB" : vlegend.label;

				RealFunctionF64 phase = (phase_continuous ? PhaseContinued(data, phase_oscillation) : PhaseCut(data)) * phase_factor;
				RealFunctionF64	magnitude = log_magnitude ? LogMagnitude(data, log_magnitude_dynamic_range) : LinearMagnitude(data);

				switch(option)
				{
					case magnitude_phase_option:
					{
						GraphSet	gs(data_title, L"", xlegend.label);
						gs.AddGraphUniform(phase, xlegend.min_value, xlegend.step, ssprintf(L"phase(%s)", phase_unit_label.c_str()));
						gs.AddGraphUniform(magnitude, xlegend.min_value, xlegend.step, (magnitude_unit_label.size() ? ssprintf(L"magnitude(%s)", magnitude_unit_label.c_str()) : L"magnitude(linear)"));
						gs.Display();
					}
					break;

					case real_option:
					{
						DisplayMathFunction(real(data), xlegend.min_value, xlegend.step, ssprintf(L"%s <Real part>", data_title.c_str()), vlegend.label, xlegend.label);
					}
					break;

					case imag_option:
					{
						DisplayMathFunction(imag(data), xlegend.min_value, xlegend.step, ssprintf(L"%s <Imaginary part>", data_title.c_str()), vlegend.label, xlegend.label);
					}
					break;

					case linear_magnitude_option:
					case log_magnitude_option:
					{
						wstring	magnitude_title = log_magnitude ? ssprintf(L"%s <Log. magnitude>", data_title.c_str()) : ssprintf(L"%s <Linear magnitude>", data_title.c_str());
						DisplayMathFunction(magnitude, xlegend.min_value, xlegend.step, magnitude_title, magnitude_unit_label, xlegend.label);
					}
					break;

					case phase_continued_option:
					case phase_cut_option:
					{
						DisplayMathFunction(phase, xlegend.min_value, xlegend.step, ssprintf(L"%s <Phase>", data_title.c_str()), phase_unit_label, xlegend.label);
					}
					break;

					case phase_deriv_option:
					{
						RealFunctionF64 phase_derivative = PhaseDerivative(data, phase_oscillation) * phase_factor;
						if(!derivative_per_samples)
						{
							phase_derivative /= xlegend.step;
						}
						DisplayMathFunction(phase_derivative, xlegend.min_value, xlegend.step, ssprintf(L"%s <Phase derivative>", data_title.c_str()), phase_unit_label + derivative_divisor_label, xlegend.label);
					}
					break;

					case roll_option:
					{
						ComplexFunctionF64	rolled_data(data);
						rolled_data.roll_half(true);

						DisplayMathFunction(rolled_data, xlegend.min_value-xlegend.step*data.size()/2., xlegend.step, ssprintf(L"%s <Rolled>", data_title.c_str()), vlegend.label, xlegend.label);
					}
					break;

					case spectrum_option:
					{
						if(GetCheckboxDecision("Choose FT options",
						{"Inverse FT", "Roll before FT", "Roll after FT"},
						{&inverse_ft_direction, &roll_data_before_ft, &roll_spectrum_after_ft}))
						{
							ComplexFunctionF64	spectrum(data);

							window_function_e wfe = AskWindowFunction();

							ApplyWindowFunction(spectrum, wfe);
							if(roll_data_before_ft)
							{
								spectrum.roll_half(true);
							}
							if(!inverse_ft_direction) FT(spectrum, ftForward);
							else FT(spectrum, ftReverse);

							if(roll_spectrum_after_ft)
							{
								spectrum.roll_half(true);
							}

							double	x0 = roll_spectrum_after_ft ? -pi()/xlegend.step : 0;
							wstring	frequency_label(L"");

							if(xlegend.label.size())
							{
								frequency_label = ssprintf(L"2*pi/(%s)", xlegend.label.c_str());
							}

							DisplayMathFunction(spectrum, x0, 2.*pi()/(xlegend.step*data.size()), ssprintf(L"%s <Spectrum>", data_title.c_str()), vlegend.label, frequency_label); // 2*PI
						}
					}
					break;

					case polar_option:
					{
						GraphSet	gs(data_title, vlegend.label + L"<imag>", vlegend.label + L" <real>");
						gs.AddGraphParametric(imag(data), real(data), ssprintf(L"%s <Polar>", data_title.c_str()));
						gs.Display();
					}
					break;

					case real_imag_option:
					{
						GraphSet	gs(data_title, vlegend.label, xlegend.label);
						gs.AddGraphUniform(real(data), xlegend.min_value, xlegend.step, "Real part");
						gs.AddGraphUniform(imag(data), xlegend.min_value, xlegend.step, "Imag part");
						gs.Display();
					}
					break;

					case flip_phase_option:
					{
						ComplexFunctionF64 flipped_phase(data.size());
						for(size_t i=0; i<data.size(); ++i)
						{
							flipped_phase[i] = i%2 ? -data[i] : data[i];
						}
						DisplayMathFunction(flipped_phase, xlegend.min_value, xlegend.step, ssprintf(L"%s <Flip phase>", data_title.c_str()), vlegend.label, xlegend.label);
					}
					break;

					case cancel_phase_option:
					{
						ComplexFunctionF64 magnitude_c(magnitude);
						DisplayMathFunction(magnitude_c, xlegend.min_value, xlegend.step, ssprintf(L"%s <Magnitude>", data_title.c_str()), vlegend.label, xlegend.label);
					}
					break;

					case cancel_magnitude_option:
					{
						ComplexFunctionF64 phase_c(data.size());
						for(size_t i=0; i<data.size(); i++)
						{
							phase_c[i] = polar(1., phase[i]);
						}
						DisplayMathFunction(phase_c, xlegend.min_value, xlegend.step, ssprintf(L"%s <Phase only>", data_title.c_str()), L"", xlegend.label);
					}
					break;

					case derivative_option:
					{
						ComplexFunctionF64 derivative = Derivative(data, phase_oscillation);
						if(!derivative_per_samples)
						{
							derivative /= xlegend.step;
						}
						wstring	y_label_derivative = vlegend.label.size() ? vlegend.label + derivative_divisor_label : L"y unit" + derivative_divisor_label;
						DisplayMathFunction(derivative, xlegend.min_value, xlegend.step, ssprintf(L"%s <Derivative>", data_title.c_str()), y_label_derivative, xlegend.label);
					}
					break;

					case fragment_option:
					{
						double	x_max = xlegend.min_value + xlegend.step*data.size();
						double	x_min = GetFloating("Fragment start", xlegend.min_value, xlegend.min_value, x_max);
						x_max = GetFloating("Fragment end", x_max, x_min, x_max);

						upsample_fragment = YesOrNo("Upsample fragment?", false);

						size_t	fragment_size = (x_max - x_min)/xlegend.step;
						size_t	fragment_start = (x_min - xlegend.min_value)/xlegend.step;
						if(fragment_start + fragment_size > data.size()-1) fragment_size = data.size() - fragment_start;
						if(fragment_size < 2) break;
						printf("\n fragment_start = %du, fragEnd = %du, size = %du", int(fragment_start), int(fragment_start + fragment_size), int(data.size()));

						ComplexFunctionF64 fragment;
						size_t	upsample_ratio;

						if(upsample_fragment)
						{
							upsample_ratio = GetUnsigned("Upsample ratio", 2, 1, 1024);
							complex_function_oscillation oscillation = AskOscillation();

							fragment.realloc(fragment_size*upsample_ratio);
							for(size_t i = 0; i < fragment_size*upsample_ratio; i++)
							{
								fragment[i] = data.in(fragment_start + double(i)/upsample_ratio, oscillation);
							}
						}
						else
						{
							upsample_ratio = 1;
							fragment.realloc(fragment_size);
							std::copy(data.begin() + fragment_start, data.begin() + fragment_start+fragment_size, fragment.begin());
						}

						DisplayMathFunction(fragment, x_min, xlegend.step/upsample_ratio, ssprintf(L"%s <Fragment %g--%g>", data_title.c_str(), x_min, x_max), vlegend.label, xlegend.label);
					}
					break;

					default:
						break;
				}
			});
	}
}


void	grafrgb(const ColorFunctionF64 &data, const wstring &data_title,
				const value_legend &vlegend,
				const axis_legend &xlegend,
				bool stop)
{
	wstring option_title = ssprintf(L"Display '%s'",
			EnsureType<const wchar_t*>(data_title.c_str()));
	for (;;)
	{
		enum	display_options
		{
			rgb,
			red,
			green,
			blue,
			hue,
			lightness,
			saturation,

			cancel_option
		};
		auto answer = GetButtonDecision(convert_to_string(option_title),
				{
					MakeButton(L"RGB graph", rgb),
					MakeButton(L"Red graph", red),
					MakeButton(L"Green graph", green),
					MakeButton(L"Blue graph", blue),
					MakeButton(L"Hue graph", hue),
					MakeButton(L"Lightness graph", lightness),
					MakeButton(L"Saturation graph", saturation),

					MakeButton(L"Exit display", cancel_option)
				});
		if (answer == cancel_option)
			break;
		switch(answer)
		{
			case rgb:
			{
				GraphSet	gs(data_title, vlegend.label, xlegend.label);

				gs.AddGraphUniform(data.green(), xlegend.min_value, xlegend.step, "green");
				gs.AddGraphUniform(data.blue(), xlegend.min_value, xlegend.step, "blue");
				gs.AddGraphUniform(data.red(), xlegend.min_value, xlegend.step, "red");

				gs.Display(stop);
			}
			break;
			case red:
				DisplayMathFunction(data.red(), xlegend.min_value, xlegend.step, data_title + L" red", vlegend.label, xlegend.label, stop);
				break;
			case green:
				DisplayMathFunction(data.green(), xlegend.min_value, xlegend.step, data_title + L" green", vlegend.label, xlegend.label, stop);
				break;
			case blue:
				DisplayMathFunction(data.blue(), xlegend.min_value, xlegend.step, data_title + L" blue", vlegend.label, xlegend.label, stop);
				break;
			case hue:
			{
				RealFunctionF32	H;
				H.MakeCopy(data, [](float &x, const HLSColorSample &s){return x=s.H;});
				DisplayMathFunction(H, xlegend.min_value, xlegend.step, data_title + L" 'hue'", vlegend.label, xlegend.label, stop);
			}
			break;
			case lightness:
			{
				RealFunctionF32	L;
				L.MakeCopy(data, [](float &x, const HLSColorSample &s){return x=s.L;});
				DisplayMathFunction(L, xlegend.min_value, xlegend.step, data_title + L" 'lightness'", vlegend.label, xlegend.label, stop);
			}
			break;
			case saturation:
			{
				RealFunctionF32	S;
				S.MakeCopy(data, [](float &x, const HLSColorSample &s){return x=s.S;});
				DisplayMathFunction(S, xlegend.min_value, xlegend.step, data_title + L" 'saturation'", vlegend.label, xlegend.label, stop);
			}
			break;
		}
	}
}
}//namespace __DisplayMathFunctionHelpers

XRAD_END
