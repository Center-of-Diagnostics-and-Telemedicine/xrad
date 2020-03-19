XRAD_BEGIN


//--------------------------------------------------------------
//
//	FIRFilterKernel Apply
//
//--------------------------------------------------------------

template<XRAD__FilterKernel_template>
template<class ORIGINAL_IT, class FIT>
floating64_type<typename ORIGINAL_IT::value_type>	FIRFilterKernel<XRAD__FilterKernel_template_args>::ApplyUsingIterators(ORIGINAL_IT data_it, FIT fit, FIT fie) const
{
	floating64_type<typename ORIGINAL_IT::value_type>	result(*data_it);// если отсчет исходных данных представляет собой динамический массив, то здесь будет зааллокирован результат
	xrad::multiply(result, *data_it, *fit);
	++fit, ++data_it;
	for(;fit<fie; ++fit, ++data_it)
	{
		xrad::add_multiply(result, *data_it, *fit);
	}
	return result /= divisor;
}


//! \todo Этот метод нужно переписать, вместо вызовов at() использовать итератор.
template<XRAD__FilterKernel_template>
template<class ARR2>
floating64_type<typename ARR2::value_type> FIRFilterKernel<XRAD__FilterKernel_template_args>::Apply(const ARR2 &data, ptrdiff_t x) const
{
	if (!size())
		return floating64_type<typename ARR2::value_type>();
	size_t	fs2 = size()/2;
		// ранее было (size()-1)/2, но это приводило к ошибке в фильтре 2 порядка,
		// который используется при линейной интерполяции. для нечетных
		// порядков это на результат не влияет, а четные лучше так.
	size_t	data_size = data.size();

	if(!(size()%2)) ++x;
		// четный фильтр всегда дает асимметричный результат. по умолчанию
		// фильтрованная функция смещалась на 1 отсчет вправо () точнее,
		// центр тяжести смещается на полотсчета. это приводит
		// к смещению результатов интерполяции. эта поправка обеспечивает
		// сдвиг влево на четных фильтрах.

		// возможно, это не лучший вариант
		// исправления ошибки: правильнее было бы внести поправки в формулы
		// для величин i0, i1 и data_i в последующих вычислениях. но это
		// привело бы к необходимости заново отлаживать весь последующий код

	//TODO расчет этих границ улучшен. перенести то же в двумерный фильтр
	ptrdiff_t	x0 = x-ptrdiff_t(fs2);
	ptrdiff_t	x1 = x0+size();//было x+s2
	ptrdiff_t	i0 = (x0 > 0) ? 0 : ptrdiff_t(fs2)-x;
	ptrdiff_t	i1 = (x1 < ptrdiff_t(data_size)) ? ptrdiff_t(size()) : ptrdiff_t(size()) - (x1 - ptrdiff_t(data_size));

	// в расчетах диапазонов i0, i1 присутствовала ошибка, приводящая к выходу за границы
	// массива интерполятора (не данных). тот же алгоритм и в двумерных интерполяторах,
	// исправление:
	if(i0 > ptrdiff_t(size())) i0 = size();
	if(i1 < 0) i1 = 0;

#if 1
	auto	filter_it = cbegin();
	auto	filter_ie = cend();
#else
	//TODO на пробу повысить быстродействие. в фильтрах не должен встречаться шаг более 1
	// почему-то не сработала data() вместо &at(0)
	auto	*filter_it = &at(0);
	auto	*filter_ie = filter_it+size();
#endif
	typename ARR2::const_iterator data_it = data.begin(), data_ie = data.end();

	floating64_type<typename ARR2::value_type_variable>	result(*data_it);
	make_zero(result);
	// если массив простых типов, присваивает ноль.
	// если массив контейнеров, копирует в контейнер результата нулевое содержимое.
	// описать подробнее

	if(i0)
	{
	// для элементов фильтра от 0 до i0 данных нет. требуется экстраполяция
		switch(m_extrapolation_method)
		{
			case extrapolation::none:
				ForceDebugBreak();
				throw out_of_range("FilterKernel::Apply, extrapolation is not allowed");

			case extrapolation::by_zero:
				// экстраполяция нулями. учитываем только те элементы фильтра, для которых есть отсчеты данных
				filter_it += i0;
				break;

			case extrapolation::cyclic:
				// экстраполируется циклически. результат точно совпадает с результатом свертки посредством бпф
				for(ptrdiff_t i = 0; i < i0; ++i, ++filter_it)
				{
					ptrdiff_t	residue = (x+i+data_size-ptrdiff_t(fs2)) % data_size;//TODO эта формула нуждается в проверке. То же ниже
					ptrdiff_t	data_i = residue>=0 ? residue : residue+data_size;
					xrad::add_multiply(result, data[data_i], *filter_it);
				}
				break;

			case extrapolation::by_last_value:
				// экстраполяция последним известным значением. первый цикл суммирует элементы фильтра, помноженные на нулевой элемент данных
				for(ptrdiff_t i = 0; i < i0; ++i, ++filter_it)
				{
					xrad::add_multiply(result, *data_it, *filter_it);
				}
				break;

			default:
				ForceDebugBreak();
				throw invalid_argument(typeid(self).name() + string("::Apply(") + typeid(ARR2).name() +
						ssprintf(", int).\nUnknown extrapolation method = 0x%X.", m_extrapolation_method));
		}
	}

	// ядро фильтра пройдено целиком, данные накоплены, дальнешие действия излишни
	if(filter_it >= filter_ie) return result /= divisor;

	if((x+i0-ptrdiff_t(fs2)) < ptrdiff_t(data.size()))
	{
		// если первый требуемый фильтром элемент данных находится в допустимом диапазоне, выполняем
		// основной цикл фильтра: вычисляется свертка в полном объеме там, где есть данные
		data_it += (x+i0-ptrdiff_t(fs2));
		for(ptrdiff_t i = i0; i < i1; ++i, ++filter_it, ++data_it)
		{
			xrad::add_multiply(result, *data_it, *filter_it);
		}
	}

	if(i1<ptrdiff_t(size()))
	{
	// для элементов фильтра от i1 до size() данных нет. требуется экстраполяция
		switch(m_extrapolation_method)
		{
			case extrapolation::none:
				ForceDebugBreak();
				throw out_of_range("FilterKernel::Apply, extrapolation is not allowed");

			case extrapolation::by_zero:
				// ничего более не требуется
				break;

			case extrapolation::cyclic:
				// экстраполируется циклически. результат точно совпадает с результатом свертки посредством бпф
				for(ptrdiff_t i = i1; i < ptrdiff_t(size()); ++i, ++filter_it)
				{
					ptrdiff_t	residue = (x+i+data_size-ptrdiff_t(fs2)) % data_size;
					ptrdiff_t	data_i = residue>=0 ? residue : residue+data_size;
					xrad::add_multiply(result, data[data_i], *filter_it);
				}
				break;

			case extrapolation::by_last_value:
				// экстраполяция последним известным значением. суммируем элементы фильтра, помноженные на последний элемент данных.
				// итератор данных возвращаем влево на 1
				data_it = data_ie-1;
				for(ptrdiff_t i = i1; i < ptrdiff_t(size()); ++i, ++filter_it)
				{
					xrad::add_multiply(result, *data_it, *filter_it);
				}
				break;

			default:
				ForceDebugBreak();
				throw invalid_argument(typeid(self).name() + string("::Apply(") + typeid(ARR2).name() +
						ssprintf(", ptrdiff_t).\nUnknown extrapolation method = 0x%X.", m_extrapolation_method));
		}
	}
	return result /= divisor;
}



XRAD_END
