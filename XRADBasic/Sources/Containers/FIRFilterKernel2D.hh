/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
XRAD_BEGIN



//--------------------------------------------------------------
//
//	FIRFilter2DMask
//
//--------------------------------------------------------------


template<class BOOLEAN>
inline void	FIRFilterKernel2DMask<BOOLEAN>::InitFilter(FIRFilter2DType filter_type, double fp)
{
	size_t	s, s2;
	switch(filter_type)
	{
		case square3:
		case cross3:
		case diamond3:
			s=3;
			break;

		case diamond5:
		case square5:
		case cross5:
			s=5;
			break;

		case square7:
		case cross7:
		case diamond7:
			s=7;
			break;

		case square9:
		case cross9:
		case diamond9:
			s = 9;
			break;


		default:
			ForceDebugBreak();
			throw invalid_argument(typeid(self).name() + ssprintf("::InitFilter(FIRFilter2DType) -- Unknown filter type (%d)", int(filter_type)));
	};

	s2 = s/2;

	switch(filter_type)
	{
		case cross3:
		case cross5:
		case cross7:
		case cross9:
			realloc(s, s, false);
			row(s2).fill(true);
			col(s2).fill(true);
			break;


		case square3:
		case square5:
		case square7:
		case square9:
			realloc(s, s, true);
			break;

		case diamond3:
		case diamond5:
		case diamond7:
		case diamond9:
			realloc(s, s, false);
			for(ptrdiff_t i = 0; i < ptrdiff_t(s2+1); ++i)
			{
				for(ptrdiff_t j = -ptrdiff_t(i); j < i+1; ++j)
				{
					at(i, j+s2) = true;
					at(s-i-1, j+s2) = true;
				}
			}
			break;

		default:
			ForceDebugBreak();
			throw invalid_argument(typeid(self).name() + ssprintf("::InitFilter(FIRFilter2DType) -- Unknown filter type (%d)", int(filter_type)));
	};
	InitBuffer(fp);
}

template<class B>
void FIRFilterKernel2DMask<B>::CountOrderStatistics()
{
	n_order_statistics = 0;
	for(size_t i = 0; i < vsize(); ++i)
	{
		for(size_t j = 0; j < hsize(); ++j)
		{
			if(at(i, j)==true) ++n_order_statistics;
		}
	}
}


template<class B>
void FIRFilterKernel2DMask<B>::SetFractile(double p)
{
	fractile_no = iround_n<size_t>((n_order_statistics-1)*p);
}


template<class B>
template<class A2D2>
typename A2D2::value_type	FIRFilterKernel2DMask<B>::Apply(const A2D2 &data, ptrdiff_t v, ptrdiff_t h)
{
	if(!ready() || !order_buffer)
	{
		ForceDebugBreak();
		throw invalid_argument(typeid(self).name() + string("::Apply(") + typeid(A2D2).name() +
				ssprintf(", ptrdiff_t, ptrdiff_t).\nFilter not initialized"));
	}
	if (data.empty())
	{
		throw invalid_argument(ssprintf("%s::Apply(data, v, h): empty data.",
				EnsureType<const char*>(typeid(self).name())));
	}

	typedef typename A2D2::value_type filtered_value_type;
	order_buffer->realloc(n_order_statistics*sizeof(filtered_value_type));

	filtered_value_type	*order_start = reinterpret_cast<filtered_value_type*>(order_buffer->data());
	filtered_value_type	*order_end = order_start;

	ptrdiff_t	data_i0 = v - ptrdiff_t(vsize()/2);

	ptrdiff_t	data_j0 = h - ptrdiff_t(hsize()/2);
	ptrdiff_t data_j1 = data_j0 + ptrdiff_t(hsize());
	size_t c0 = 0, c1 = 0;
	if (data_j0 < 0)
	{
		if (data_j1 <= 0)
		{
			data_j1 = 0;
			c0 = hsize();
		}
		else
		{
			c0 = size_t(-data_j0);
		}
		data_j0 = 0;
	}
	if (data_j1 > ptrdiff_t(data.hsize()))
	{
		if (data_j0 >= ptrdiff_t(data.hsize()))
		{
			data_j0 = ptrdiff_t(data.hsize() - 1);
			data_j1 = data_j0;
			c1 = hsize();
		}
		else
		{
			c1 = size_t(data_j1) - (data.hsize() - 1);
			data_j1 = ptrdiff_t(data.hsize() - 1);
		}
	}

	for(size_t i = 0; i < vsize(); ++i)
	{
		ptrdiff_t	data_i = range(data_i0 + static_cast<ptrdiff_t>(i),
				0, static_cast<ptrdiff_t>(data.vsize() - 1));

		typename A2D2::row_type::const_iterator data_it = data.row(data_i).begin() + data_j0;
		typename row_type::const_iterator	filter_it = row(i).begin();

		for (size_t j = 0; j < c0; ++j, ++filter_it)
		{
			if(*filter_it==true)
			{
				*order_end = *data_it;
				++order_end;
			}
		}
		for(ptrdiff_t j = data_j0; j < data_j1; ++j, ++filter_it, ++data_it)
		{
			if(*filter_it==true)
			{
				*order_end = *data_it;
				++order_end;
			}
		}
		for (size_t j = 0; j < c1; ++j, ++filter_it)
		{
			if(*filter_it==true)
			{
				*order_end = *data_it;
				++order_end;
			}
		}
	}

	nth_element(order_start, order_start + fractile_no, order_end);
	return order_start[fractile_no];
}

//--------------------------------------------------------------
//
//	FIRFilter2DConvolve
//
//--------------------------------------------------------------



template<class FF1D>
void	FIRFilterKernel2DConvolve<FF1D>::InitFilter(FIRFilter2DType filter_type)
{
	FIRFilterKernel2DMask<bool> buffer(filter_type, 0);
	xrad::MakeCopy(*this, buffer.GetMask(), auxiliary_import_boolean<bool>());
	value_type	normalizer(0);

	for(size_t i = 0; i < vsize(); ++i)
	{
		for(size_t j = 0; j < hsize(); ++j)
		{
			normalizer+=at(i, j);
		}
	}

	*this /= normalizer;
}




template<class FF1D>
void	FIRFilterKernel2DConvolve<FF1D>::IFilterGaussian(double sigma, double value_at_edge)
{
// sigma в отсчетах
	if(sigma <= 0 || value_at_edge >= 1 || value_at_edge <= 0)
	{
		ForceDebugBreak();
		throw invalid_argument(typeid(self).name() + string("::IFilterGaussian(") +
				ssprintf(", double, double).\nInvalid arguments(%g,%g)", sigma, value_at_edge));
	}

// расчет размера фильтра исходя из заданного минимального значения
// на краю
	ptrdiff_t	s = 1 + ptrdiff_t(2.*sigma*sqrt(-2.*log(value_at_edge)));
	if(s < 3) s = 3;
	if(!(s%2)) ++s;
	size_t	s2 = (s-1)/2;
	realloc(s, s);

	double normalizer = 0;

	for(size_t i = 0; i < vsize(); ++i)
	{
		double	argv = (double(i)-s2)/sigma;
		for(size_t j = 0; j < vsize(); ++j)
		{
			double	argh = (double(j)-s2)/sigma;
			double	arg = (argv*argv + argh*argh)/2;

			if(std::hypot(double(i)-s2, double(j)-s2) < s2)
			{
			// делаем гауссоиду изотропной, срезаем углы
				double	value = value_type(exp(-arg));
				at(i, j) = value;
				normalizer += value;
			}
			else at(i, j) = value_type(0);
		}
	}
	*this /= normalizer;
}

template<class FF1D>
void	FIRFilterKernel2DConvolve<FF1D>::IFilterUnsharp(double radius, double strength)
{
	IFilterGaussian(radius);
	*this *= -strength;
	at(vsize()/2, hsize()/2) += 1.+strength;
}



template<class FF1D>
template<class A2D2>
floating64_type<typename A2D2::value_type>	FIRFilterKernel2DConvolve<FF1D>::Apply(const A2D2 &data, ptrdiff_t v, ptrdiff_t h) const
{
	if (!vsize() || !hsize())
		return floating64_type<typename A2D2::value_type>();
	ptrdiff_t	vs2 = vsize()/2;
		// ранее было (vsize()-1)/2, но это приводило к ошибке в фильтре 2 порядка,
		// который используется при линейной двумерной интерполяции. для нечетных
		// порядков это на результат не влияет, а четные лучше так. то же и для hs2
	ptrdiff_t	hs2 = hsize()/2;

	if(!(vsize()%2)) ++v;
	if(!(hsize()%2)) ++h;

	ptrdiff_t	v_data_size = data.vsize();
	ptrdiff_t	h_data_size = data.hsize();

	// i0, i1, j0, j1 это границы, по которым коэффициенты фильтра пересекаются с массивом данных.
	// в большинстве случаев они равны 0 и (размер_фильтра-1) соответственно.
	// на краях данных одна из величин урезается, по ней отрабатывается экстраполяция.
	// возможен вырожденный случай, если данные меньше фильтра, тогда урезание с обеих сторон.
	// следует это переписать более внятно.
	ptrdiff_t	i0 = (v > vs2) ? 0 : vs2-v;
	ptrdiff_t	j0 = (h > hs2) ? 0 : hs2-h;

	ptrdiff_t	i1 = (v < v_data_size-vs2) ? ptrdiff_t(vsize()) : (ptrdiff_t(vsize()) - (v - (v_data_size - vs2)) - 1);
	ptrdiff_t	j1 = (h < h_data_size-hs2) ? ptrdiff_t(hsize()) : (ptrdiff_t(hsize()) - (h - (h_data_size - hs2)) - 1);

	if(i0 > ptrdiff_t(vsize())) i0 = ptrdiff_t(vsize());
	if(i1 < 0)  i1 = 0;
	//if(vsize() < v - (v_data_size - vs2) + 1) i1 = 0;

	if(j0 > ptrdiff_t(hsize())) j0 = ptrdiff_t(hsize());
	if(j1 < 0) j1 = 0;
	//if(hsize() < h - (h_data_size - hs2) + 1) j1 = 0;


	floating64_type<typename A2D2::value_type>	result(0);
	make_zero(result);

	switch(m_extrapolation_method)
	{
		case extrapolation::none:
			ForceDebugBreak();
			throw out_of_range("FilterKernel2D::Apply, extrapolation is not allowed");

		case extrapolation::by_zero:
			{
			// за пределами допустимых индексов
			// фильтруемая функция экстраполируется нулями
				for(ptrdiff_t i = i0; i < i1; ++i)
				{
					ptrdiff_t	data_i = v+i-vs2;
					for(ptrdiff_t j = j0; j < j1; ++j)
					{
						ptrdiff_t	data_j = h+j-hs2;
						result += data.at(data_i, data_j)*at(i, j);
						// 2016_09_01 KNS
						// по всему документу строки вида result += at(i,j)*data.at(data_i, data_j);
						// заменены на result += data.at(data_i, data_j)*at(i,j);
						//
						// большинство типов чисел можно умножать на скаляр справа. обратный порядок возможен далеко не всегда.
						// скалярные фильтры в принципе должны быть применимы для всех этих чисел.
					}
				}
			}
			break;
		case extrapolation::cyclic:
			{
			// за пределами допустимых индексов
			// фильтруемая функция экстраполируется циклически
				for(size_t i = 0; i < vsize(); ++i)
				{
					ptrdiff_t	data_i = (v+i-vs2+v_data_size) % v_data_size;
					for(size_t j = 0; j < hsize(); ++j)
					{
						ptrdiff_t	data_j = (h+j-hs2+h_data_size) % h_data_size;
						result += data.at(data_i, data_j) * at(i, j);
					}
				}
			}
			break;
		case extrapolation::by_last_value:
// 		case extrapolate_by_default:
			{
				// этот вариант предполагает, что фильтруемая функция
				// экстраполируется последним известным значением
				for(ptrdiff_t i = 0; i < i0; ++i)
				{
					ptrdiff_t	data_i = 0;

					for(ptrdiff_t j = 0; j < j0; ++j)
					{
						ptrdiff_t	data_j = 0;
						result += data.at(data_i, data_j)*at(i, j);
					}
					for(ptrdiff_t j = j0; j < j1; ++j)
					{
						ptrdiff_t	data_j = h+j-hs2;
						result += data.at(data_i, data_j)*at(i, j);
					}
					for(ptrdiff_t j = j1; j < ptrdiff_t(hsize()); ++j)
					{
						ptrdiff_t	data_j = data.hsize()-1;
						result += data.at(data_i, data_j)*at(i, j);
					}
				}

				for(ptrdiff_t i = i0; i < i1; ++i)
				{
					ptrdiff_t	data_i = v+i-vs2;

					for(ptrdiff_t j = 0; j < j0; ++j)
					{
						ptrdiff_t	data_j = 0;
						result += data.at(data_i, data_j)*at(i, j);
					}
					for(ptrdiff_t j = j0; j < j1; ++j)
					{
						ptrdiff_t	data_j = h+j-hs2;
						result += data.at(data_i, data_j)*at(i, j);
					}
					for(ptrdiff_t j = j1; j < ptrdiff_t(hsize()); ++j)
					{
						ptrdiff_t	data_j = data.hsize()-1;
						result += data.at(data_i, data_j)*at(i, j);
					}
				}

				for(ptrdiff_t i = i1; i < ptrdiff_t(vsize()); ++i)
				{
					ptrdiff_t	data_i = data.vsize()-1;

					for(ptrdiff_t j = 0; j < j0; ++j)
					{
						ptrdiff_t	data_j = 0;
						result += data.at(data_i, data_j)*at(i, j);
					}
					for(ptrdiff_t j = j0; j < j1; ++j)
					{
						ptrdiff_t	data_j = h+j-hs2;
						result += data.at(data_i, data_j)*at(i, j);
					}
					for(ptrdiff_t j = j1; j < ptrdiff_t(hsize()); ++j)
					{
						ptrdiff_t	data_j = data.hsize()-1;
						result += data.at(data_i, data_j)*at(i, j);
					}
				}
			}
			break;
		default:
			ForceDebugBreak();
			throw invalid_argument(typeid(self).name() + string("::Apply(") + typeid(A2D2).name() +
					ssprintf(", size_t, size_t).\nUnknown extrapolation method = 0x%X.", m_extrapolation_method));
	}; // extrapolation_method
	return result;
}

XRAD_END
