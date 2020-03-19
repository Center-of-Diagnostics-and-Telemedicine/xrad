//	file MathFunction.hh
//--------------------------------------------------------------
// File "pre.h" should not be included

#ifndef __mathfunction_cc
#define __mathfunction_cc

// TODO: Разорвать эту зависимость от посторонних типов данных.
#include "FIRFilterKernelFunctions.h"

XRAD_BEGIN



//--------------------------------------------------------------
//
//	Filtering functions
//
//--------------------------------------------------------------



template<XRAD__MathFunction_template>
template<class FILTER_KERNEL_T>
void	MathFunction<XRAD__MathFunction_template_args>::Filter(const FILTER_KERNEL_T &filter)
{
	filtering_algorithm fa = filter.FilteringAlgorithm();
	bool	do_bufferization = (step()>1) &&
		(complexity_e(T()) <= number_complexity_e::array) &&
		(fa != fir_scan_filter) &&
		(fa != fir_built_in);

	// Буферизация оправдана только в тех случаях, когда:
	// 1) элементы фильтруемого массива в памяти идут не подряд;
	// 2) элементы не являются динамическими контейнерами;
	// 3) когда не используются алгоритмы fir_scan_filter и filter_built_in,
	// поскольку в нем обращения к исходным данным и так сведены к минимуму.

	if(step()>1 && !do_bufferization)
	{
		// Если обрабатывается массив контейнеров, буферизация неуместна.
		// Но есть алгоритмы, которые от нее зависят.
		if(fa==fir_scan_data)
		{
			// В этом случае потерь производительности нет, просто подменяем алгоритм.
			fa=fir_scan_filter;
		}
		if(fa==iir_forward || fa==iir_reverse || fa==iir_bidirectional)
		{
			// Эта ситуация недопустима. Следует переделать алгоритм уровнем выше.
			ForceDebugBreak();
			throw invalid_argument("MathFunction::Filter(), invalid filter option, see comments in code");
		}
	}

	self	original;
	self	result;

	// Решаем вопрос с выделением буферов и созданием копии оригинала, если это нужно.
	switch(fa)
	{
		case fir_scan_data:
		case fir_built_in:
		case fir_scan_filter:
			original.MakeCopy(*this);
			if(do_bufferization)
			{
				result.realloc(size(), zero_value(at(0)));
			}
			else
			{
				result.UseData(*this);
				make_zero(*this);
			}
			break;

		case iir_forward:
		case iir_reverse:
		case iir_bidirectional:
			// Здесь копия оригинала не нужна.
			if(do_bufferization)
			{
				result.MakeCopy(*this);
			}
			else
			{
				result.UseData(*this);
			}
			break;
	};


	// Выполняем фильтрацию.
	switch(filter.FilteringAlgorithm())
	{
		case fir_scan_data:
			result.FilterScanData(original, filter);
			break;

		case fir_scan_filter:
			result.FilterScanFilter(original, filter);
			break;

		case fir_built_in:
			result.FilterBuiltIn(original, filter);
			break;

		case iir_forward:
			result.FilterRecursive(filter, iir_filter_forward);
			break;

		case iir_reverse:
			result.FilterRecursive(filter, iir_filter_reverse);
			break;

		case iir_bidirectional:
			result.FilterRecursiveBidirectional(filter);
			break;
	}

	if(do_bufferization)
	{
		CopyData(result);
	}
}



template<XRAD__MathFunction_template>
template<class FILTER_KERNEL_T>
void	MathFunction<XRAD__MathFunction_template_args>::FilterBuiltIn(
	const MathFunction<XRAD__MathFunction_template_args> &original,
	const FILTER_KERNEL_T &filter)
{
	//	Этот алгоритм может работать без буферизации. Проверка 	if(step()>1) не нужна.
	iterator result_it = begin();

	for(size_t i = 0; i < size(); ++result_it, ++i)
	{
		//floating64_type<value_type> result_buffer;
		//filter.Apply(result_buffer, original, i);
		//*result_it = result_buffer;
		*result_it = filter.Apply(original, i);
	}
}



template<XRAD__MathFunction_template>
template<class FILTER_KERNEL_T>
void	MathFunction<XRAD__MathFunction_template_args>::FilterScanFilter(
	const MathFunction<XRAD__MathFunction_template_args> &original,
	const FILTER_KERNEL_T &filter)
{
	//Этот алгоритм может работать без буферизации. Проверка if(this->step()>1) не нужна. Однако оригинал должен быть без разрывов.
	if(original.step()>1)
	{
		// Буферизация определена уровнем выше, эта функция должна получать сплошной массив без разрывов.
		// Она объявлена private, может быть вызвана только через селектор, в котором это должно быть предусмотрено.
		ForceDebugBreak();
		throw invalid_argument("void	MathFunction<T,ST>::FilterScanFilter(const FILTER_KERNEL_T &filter, fir_filter_direction direction), invalid step");
	}

	size_t filter_offset = (filter.size()%2) ? 0:1;
	ptrdiff_t filter_start_position = filter.size()/2 - filter_offset;
	ptrdiff_t	filter_remainder = filter.size() - filter_start_position - 1;

	//floating64_type<value_type>	result_buffer(at(0));
	// Конструктор от элемента нужен на тот случай, если value_type представляет собой динамический массив.
	// В этом случае выделяется буфер соответствующего размера и далее уже не переаллокируется.

	typename FILTER_KERNEL_T::const_iterator	fit = filter.begin(), fie = filter.end();

	iterator	result_it = begin();
	iterator	result_ie = end() - filter_remainder;
	const_iterator	extrapolation_it = original.begin();
	const_iterator	extrapolation_ie = original.end()-1;
	const_iterator	original_it = original.begin(), original_ie = original.end();

	extrapolation_it.freeze();
	extrapolation_ie.freeze();

	for(ptrdiff_t i = 0; i < filter_start_position; ++i, ++result_it)
	{
		// Экстраполяция влево
		switch(filter.ExtrapolationMethod())
		{
			case extrapolation::none:
				ForceDebugBreak();
				throw out_of_range("MathFunction::FilterScanFilter, extrapolation is not allowed");

			case extrapolation::by_zero:
				break;

			case extrapolation::by_last_value:
				*result_it += filter.ApplyUsingIterators(extrapolation_it, fit, fit+filter_start_position-i);
				break;
			case extrapolation::cyclic:
				*result_it += filter.ApplyUsingIterators(original.end()-(filter_start_position-i), fit, fit+filter_start_position-i);
				break;
		}
		*result_it += filter.ApplyUsingIterators(original.begin(), fit+filter_start_position-i, fie);
		//*result_it += result_buffer;
	}

	for(; result_it<result_ie; ++result_it, ++original_it)
	{
		*result_it += filter.ApplyUsingIterators(original_it, fit, fie);
	}

	for(ptrdiff_t i = 1; i <= filter_remainder; ++i, ++result_it, ++original_it)
	{
		// Экстраполяция вправо
		typename FILTER_KERNEL_T::const_iterator intermediate_fie = fie-i;
		*result_it = filter.ApplyUsingIterators(original_it, fit, intermediate_fie);
		switch(filter.ExtrapolationMethod())
		{
			case extrapolation::none:
				ForceDebugBreak();
				throw out_of_range("MathFunction::FilterScanFilter, extrapolation is not allowed");

			case extrapolation::by_zero:
				break;

			case extrapolation::by_last_value:
				*result_it += filter.ApplyUsingIterators(extrapolation_ie, intermediate_fie, fie);
				break;

			case extrapolation::cyclic:
				*result_it += filter.ApplyUsingIterators(original.begin(), intermediate_fie, fie);
				break;
		}
		//*result_it += result_buffer;
	}
	(*this) /= filter.GetNormalizer();
}



template<XRAD__MathFunction_template>
template<class FILTER_KERNEL_T>
void	MathFunction<XRAD__MathFunction_template_args>::FilterScanData(
	const MathFunction<XRAD__MathFunction_template_args> &original,
	const FILTER_KERNEL_T &filter)
{
	// Предполагается, что обход фильтруемой функции будет более эффективен, чем обход фильтра.
	if(step()>1 || original.step()>1)
	{
		// Буферизация определена уровнем выше, эта функция должна получать сплошные массивы без разрывов.
		// Она объявлена private, может быть вызвана только через селектор, в котором это должно быть предусмотрено.
		ForceDebugBreak();
		throw invalid_argument("void	MathFunction<T,ST>::FilterScanData(const MathFunction<T,ST> &original, const FILTER_KERNEL_T &filter), invalid step");
	}

	size_t filter_start_position = filter.size()/2;
	size_t filter_offset = (filter.size()%2) ? 0:1;
	// Четный фильтр всегда дает асимметричный результат. По умолчанию
	// фильтрованная функция смещалась на 1 отсчет вправо (точнее,
	// центр тяжести смещается на полотсчета). Это приводит
	// к смещению результатов интерполяции. Эта поправка обеспечивает
	// сдвиг влево на четных фильтрах.

	// Результат уже обнулен в процедуре-селекторе
	// make_zero(*this);

	const value_type	left_edge = original.at(0);
	const value_type	right_edge = original.at(size()-1);
	typename FILTER_KERNEL_T::const_iterator	filter_value = filter.begin();

	for(size_t filter_position = 0; filter_position < filter.size(); ++filter_position, ++filter_value)
	{
		ptrdiff_t	data_start_position = ptrdiff_t(filter_position) + filter_offset - filter_start_position;
		ptrdiff_t	data_end_position = data_start_position + size();

		iterator result_it = begin(), result_ie = end();
		const_iterator original_it = original.begin(), original_ie = original.end();

		if(data_start_position < 0)
		{
			// Требуются данные слева от нулевого элемента. Экстраполяция
			switch(filter.ExtrapolationMethod())
			{
				case extrapolation::none:
					ForceDebugBreak();
					throw out_of_range("MathFunction::FilterScanData, extrapolation is not allowed");

				case extrapolation::by_zero:
					// Просто перематываем вправо итератор результата
					result_it -= max(data_start_position, result_it-result_ie);
					break;

				case extrapolation::by_last_value:
					for(ptrdiff_t i = data_start_position; i < 0 && result_it<result_ie; ++i, ++result_it)
					{
						xrad::add_multiply(*result_it, left_edge, *filter_value);
					}
					break;

				case extrapolation::cyclic:
					for(ptrdiff_t i = data_start_position; i < 0 && result_it<result_ie; ++i, ++result_it)
					{
						ptrdiff_t	residue = i%size();
						ptrdiff_t	data_index = (residue>=0) ? residue : (residue + size());
							//Это нужно здесь, т.к. возможна вырожденная ситуация, когда фильтр намного длиннее, чем данные.
						xrad::add_multiply(*result_it, original[data_index], *filter_value);
					}
					break;
				default:
					ForceDebugBreak();
					throw invalid_argument(typeid(self).name() + string("::Filter(") + typeid(FILTER_KERNEL_T).name() +
							ssprintf(").\nUnknown extrapolation method = 0x%X.", filter.ExtrapolationMethod()));
			}
			original_ie += max(data_start_position, original_it-original_ie);
		}
		else
		{
			original_it += min(data_start_position, original_ie-original_it);
		}

		for(; original_it<original_ie; ++result_it, ++original_it)
		{
			// Основной цикл фильтрации
			xrad::add_multiply(*result_it, *original_it, *filter_value);
		}

		if(data_end_position>=ptrdiff_t(size()))
		{
			switch(filter.ExtrapolationMethod())
			{
				case extrapolation::none:
					ForceDebugBreak();
					throw out_of_range("MathFunction::FilterScanData, extrapolation is not allowed");

				case extrapolation::by_zero:
					break;

				case extrapolation::by_last_value:
					--original_it;
					for(; result_it<result_ie;++result_it)
					{
						xrad::add_multiply(*result_it, right_edge, *filter_value);
					}
					break;

				case extrapolation::cyclic:
					for(ptrdiff_t i = size(); result_it < result_ie; ++i, ++result_it)
					{
						ptrdiff_t	residue = i%size();
						ptrdiff_t	data_index = (residue>=0) ? residue : (residue + size());
							//Это нужно здесь, т.к. возможна вырожденная ситуация, когда фильтр намного длиннее, чем данные.
						xrad::add_multiply(*result_it, original[data_index], *filter_value);
					}
					break;

				default:
					ForceDebugBreak();
					throw invalid_argument(typeid(self).name() + string("::Filter(") + typeid(FILTER_KERNEL_T).name() +
							ssprintf(").\nUnknown extrapolation method = 0x%X.", filter.ExtrapolationMethod()));
			}
		}
	}
	(*this) /= filter.GetNormalizer();
}



template<XRAD__MathFunction_template>
template<class FILTER_KERNEL_T>
void	MathFunction<XRAD__MathFunction_template_args>::FilterRecursive(const FILTER_KERNEL_T &filter, fir_filter_direction direction)
{
	if(filter.size()==2)
	{
		// Оптимизированная версия под конкретный случай
		FilterRecursive2(filter, direction);
		return;
	}

	if(step()>1)
	{
		// Буферизация определена уровнем выше, эта функция должна получать сплошной массив без разрывов.
		// Она объявлена private, может быть вызвана только через селектор, в котором это должно быть предусмотрено.
		ForceDebugBreak();
		throw invalid_argument("void	MathFunction<T,ST>::FilterRecursive(const FILTER_KERNEL_T &filter, fir_filter_direction direction), invalid step");
	}

	//floating64_type<value_type>	result_buffer(at(0));
	// Конструктор от элемента нужен на тот случай, если value_type представляет собой динамический массив.
	// В этом случае выделяется буфер соответствующего размера и далее уже не переаллокируется.

	typename FILTER_KERNEL_T::const_iterator	fit = filter.begin(), fie = filter.end();

	if(direction == iir_filter_forward)
	{
		iterator	result_it = begin(), result_ie = end();
		iterator	extrapolation_it = begin();
		iterator	data_start_it = begin();

		extrapolation_it.freeze();

		size_t	s0 = filter.size();

		for(size_t i = 1; i < s0; ++i, ++result_it)
		{
			// Экстраполяция
			switch(filter.ExtrapolationMethod())
			{
				case extrapolation::none:
					ForceDebugBreak();
					throw out_of_range("MathFunction::FilterRecursive, extrapolation is not allowed");

				case extrapolation::by_zero:
					break;

				case extrapolation::by_last_value:
					*result_it += filter.ApplyUsingIterators(extrapolation_it, fit, fie-i);
					break;

				case extrapolation::cyclic:
					*result_it += filter.ApplyUsingIterators(end()-i, fit, fie-i);
					break;
			}
			*result_it += filter.ApplyUsingIterators(data_start_it, fie-i, fie);
			//*result_it += result_buffer;
		}

		for(; result_it<result_ie; ++result_it, ++data_start_it)
		{
			*result_it += filter.ApplyUsingIterators(data_start_it, fit, fie);
			//*result_it += result_buffer;
		}
	}
	else
	{
		reverse_iterator	result_it = rbegin(), result_ie = rend();
		reverse_iterator	extrapolation_it = rbegin();
		reverse_iterator	data_start_it = rbegin();

		extrapolation_it.base().freeze();

		size_t	s0 = filter.size();

		for(size_t i = 1; i < s0; ++i, ++result_it)
		{
			// Экстраполяция
			switch(filter.ExtrapolationMethod())
			{
				case extrapolation::none:
					ForceDebugBreak();
					throw out_of_range("MathFunction::FilterRecursive, extrapolation is not allowed");

				case extrapolation::by_zero:
					break;

				case extrapolation::by_last_value:
					*result_it += filter.ApplyUsingIterators(extrapolation_it, fit, fie-i);
					break;

				case extrapolation::cyclic:
					*result_it += filter.ApplyUsingIterators(end()-i, fit, fie-i);
					break;
			}
			*result_it += filter.ApplyUsingIterators(data_start_it, fie-i, fie);
		}

		for(; result_it<result_ie; ++result_it, ++data_start_it)
		{
			*result_it += filter.ApplyUsingIterators(data_start_it, fit, fie);
		}
	}
}



template<XRAD__MathFunction_template>
template<class FILTER_KERNEL_T>
void	MathFunction<XRAD__MathFunction_template_args>::FilterRecursive2(const FILTER_KERNEL_T &filter, fir_filter_direction direction)
{
	if(step()>1)
	{
		// Буферизация определена уровнем выше, эта функция должна получать сплошной массив без разрывов.
		// Она объявлена private, может быть вызвана только через селектор, в котором это должно быть предусмотрено.
		ForceDebugBreak();
		throw invalid_argument("void	MathFunction<T,ST>::FilterRecursive2(const FILTER_KERNEL_T &filter, fir_filter_direction direction), invalid step");
	}

	//typename FILTER_KERNEL_T::value_type	a = filter[0], b = filter[1];
	//Вышестоящая строчка вредно сказывалась на производительности. Лучше как ниже.
	typename FILTER_KERNEL_T::const_iterator a = filter.begin(), b(a+1);

	if(direction == iir_filter_forward)
	{
		iterator	it1 = begin(), ie = end();
		iterator	it2 = it1+1;

		xrad::add_multiply(*it1, *it1, *b);
		switch(filter.ExtrapolationMethod())
		{
			case extrapolation::none:
				ForceDebugBreak();
				throw out_of_range("MathFunction::FilterRecursive2, extrapolation is not allowed");

			case extrapolation::by_zero:
				break;

			case extrapolation::by_last_value:
				xrad::add_multiply(*it1, *it1, *a/(*b+1));// Здесь включена поправка на ранее сделанное умножение на b.
				break;

			case extrapolation::cyclic:
				xrad::add_multiply(*it1, at(size()-1), *a);
				break;
		}
		for(; it2<ie; ++it1, ++it2)
		{
			xrad::add_multiply(*it2, *it2, *b);
			xrad::add_multiply(*it2, *it1, *a);
		}
	}
	else
	{
		reverse_iterator	it1 = rbegin(), ie = rend();
		reverse_iterator	it2 = it1+1;

		xrad::add_multiply(*it1, *it1, *b);
		switch(filter.ExtrapolationMethod())
		{
			case extrapolation::none:
				ForceDebugBreak();
				throw out_of_range("MathFunction::FilterRecursive2, extrapolation is not allowed");

			case extrapolation::by_zero:
				break;

			case extrapolation::by_last_value:
				xrad::add_multiply(*it1, *it1, *a/(*b+1));// Здесь включена поправка на ранее сделанное умножение на b.
				break;

			case extrapolation::cyclic:
				xrad::add_multiply(*it1, at(size()-1), *a);
				break;
		}

		for(; it2<ie; ++it1, ++it2)
		{
			xrad::add_multiply(*it2, *it2, *b);
			xrad::add_multiply(*it2, *it1, *a);
		}
	}
}

template<XRAD__MathFunction_template>
template<class FILTER_KERNEL_T>
void	MathFunction<XRAD__MathFunction_template_args>::FilterRecursiveBidirectional(const FILTER_KERNEL_T &filter)
{
	FilterRecursive(filter, iir_filter_forward);
	FilterRecursive(filter, iir_filter_reverse);
}



template<XRAD__MathFunction_template>
void	MathFunction<XRAD__MathFunction_template_args>::FilterMedian(size_t filter_size)
{
	if(!size() || !filter_size) return;

	if(!(filter_size%2)) ++filter_size; // Должен быть нечетным
	size_t	fs2 = filter_size/2;

	const self	unfiltered_buffer(*this);
	self	filter(filter_size);


	//unfiltered_buffer.MakeCopy(*this);
	//filter.realloc(filter_size);

	//auto filter_begin = &filter.at(0);
	//auto f2 = &filter.at(0) + fs2;
	//auto filter_end = &filter.at(0) + filter_size;
	auto filter_begin = filter.begin();
	auto f2 = filter.begin() + fs2;
	auto filter_end = filter.begin() + filter_size;
	// Два буфера (копия данных и фильтр) аллокируются прямо здесь;
	// следовательно, шаг step() у них равен единице.
	// Поэтому вместо итератора, заданного в классе, используем
	// ниже указатели на элементы массива. Это для скорости.

	iterator data_it = begin();
	// А для массива данных это невозможно, так как шаг в нем
	// произвольный.

	for(size_t i = 0; i < size(); ++i, ++data_it)
	{
		//auto	b1 = &unfiltered_buffer.at(range(i-fs2, 0, size()-filter_size-1));
		auto	b1 = unfiltered_buffer.begin() + range(i-fs2, 0, size()-filter_size-1);
		copy(b1, b1+filter_size, filter_begin);

		nth_element(filter_begin, f2, filter_end);
		*data_it = *f2;
	}
}



//--------------------------------------------------------------
//
//	MathFunction::FilterGauss
//	Выполняет свертку с гауссоидой заданной ширины.
//

template<XRAD__MathFunction_template>
void	MathFunction<XRAD__MathFunction_template_args>::FilterGauss(double dispersion, double value_at_edge, extrapolation::method ex)
{
	FIRFilterKernel<double, double, AlgebraicStructures::FieldTagScalar>	filter;

	InitFIRFilterGaussian(filter, dispersion, value_at_edge);

	filter.SetExtrapolationMethod(ex);
	Filter(filter);
}



//--------------------------------------------------------------

XRAD_END

#endif //__mathfunction_cc
