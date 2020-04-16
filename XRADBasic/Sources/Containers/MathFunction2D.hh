//	file MathFunction2D.hh
//--------------------------------------------------------------
#ifndef __math_function_2d_h
#error This file must be included from "MathFunction2D.h" only
#endif

#include "SpaceCoordinates.h"
#include "UniversalInterpolation2D.h"

XRAD_BEGIN



//TODO: Две следующие функции из DataArray2D (родителя) не подхватились, выяснить причину. То же в MD и в complex2d.
template<class FT>
MathFunction2D<FT>	zero_value(const MathFunction2D<FT> &datum)
{
	return MathFunction2D<FT>(datum.vsize(), datum.hsize(), zero_value(datum.at(0, 0)));
}

template<class FT>
void	make_zero(MathFunction2D<FT> &datum)
{
	make_zero(datum.at(0, 0));
	datum.fill(datum.at(0, 0));
}



//--------------------------------------------------------------
//
//	Прокрутка
//
//--------------------------------------------------------------

//	На половину размера
template<class FT>
void MathFunction2D<FT>::roll_half(bool forward)
{
	for(size_t i = 0; i < vsize(); ++i) row(i).roll_half(forward);
	for(size_t i = 0; i < hsize(); ++i) col(i).roll_half(forward);
}

//	На заданное кол-во отсчетов по вертикали и горизонтали
template<class FT>
void MathFunction2D<FT>::roll(ptrdiff_t v, ptrdiff_t h)
{
	for(size_t i = 0; i < vsize(); ++i) row(i).roll(h);
	for(size_t i = 0; i < hsize(); ++i) col(i).roll(v);
}



//--------------------------------------------------------------
//
//	Фильтрация
//
//--------------------------------------------------------------



template<class FT>
template<class FIR_FILTER_T>
void	MathFunction2D<FT>::Filter(const FIR_FILTER_T &filter)
{
	self Buffer(*this);

	for(size_t i = 0; i < vsize(); i++)
	{
		typename row_type::iterator	it = row(i).begin(), ie = row(i).end();
		size_t j = 0;
		for(; it < ie; ++it, ++j)
		{
			*it = filter.Apply(Buffer, i, j);
		}
	}
}



//--------------------------------------------------------------
// Двумерный фильтр.
// Обход по каждой из 2 осей по рядам
//--------------------------------------------------------------

template <class FT, class FILTER_T1, class FILTER_T2>
void FilterArray2DSeparate(DataArray2D<FT> &slice, const FILTER_T1 &filter_y, const FILTER_T2 &filter_x)
{
	if(slice.n_dimensions() !=2)
	{
		ForceDebugBreak();
		throw invalid_argument("FilterArray2DSeparate(void FilterArray2DSeparate(DataArray2D<FT> &slice, const FILTER_T1 &filter_x, const FILTER_T2 &filter_y), invalid number of dimensions");
	}

	if(filter_x.size()>1)
	{
		for(size_t i = 0; i < slice.vsize(); ++i)
		{
			slice.row(i).Filter(filter_x);
		}
	}

	if(filter_y.size()>1)
	{
		for(size_t i = 0; i < slice.hsize(); ++i)
		{
			slice.col(i).Filter(filter_y);
		}
	}
}

template <class FT, class FILTER_T, class ST>
void FilterArray2DSeparate(DataArray2D<FT> &slice, const point_2<FILTER_T, ST, typename FILTER_T::field_tag> &filters)
{
	FilterArray2DSeparate(slice, filters.y(), filters.x());
}

template<class FT>
void	MathFunction2D<FT>::FilterGaussSeparate(double v_dispersion, double h_dispersion, double value_at_edge, extrapolation::method ex)
{
	if(v_dispersion)
	{
		FilterKernelReal	v_filter;
		InitFIRFilterGaussian(v_filter, v_dispersion, value_at_edge);
		v_filter.SetExtrapolationMethod(ex);

		for(size_t i = 0; i < hsize(); i++)
		{
// 			col(i).FilterGauss(v_dispersion, value_at_edge, ex);
			col(i).Filter(v_filter);
		}
	}
	if(h_dispersion)
	{
		FilterKernelReal	h_filter;
		InitFIRFilterGaussian(h_filter, h_dispersion, value_at_edge);
		h_filter.SetExtrapolationMethod(ex);

		for(size_t i = 0; i < vsize(); i++)
		{
// 			row(i).FilterGauss(h_dispersion, value_at_edge, ex);
			row(i).Filter(h_filter);
		}
	}
}



template<class FT>
template<class B>
void	MathFunction2D<FT>::Filter(FIRFilterKernel2DMask<B> &filter)
{
	self	Buffer(*this);

	for(size_t i = 0; i < vsize(); i++)
	{
		typename row_type::iterator	it = row(i).begin(), ie = row(i).end();
		for(size_t j = 0; it < ie; ++it, ++j)
		{
			*it = filter.Apply(Buffer, i, j);//вызываемую функцию перевести на size_t аргументы нельзя
		}
	}
}



//--------------------------------------------------------------
//
// Интерполяция и дифференцирование
//
//--------------------------------------------------------------

template<class FT>
floating32_type<typename MathFunction2D<FT>::value_type> MathFunction2D<FT>::in(double v, double h) const
{
	return in(v, h, &interpolators2D::bessel1_isotropic);
}



template<class FT>
template<class INTERPOLATOR_T>
floating32_type<typename MathFunction2D<FT>::value_type> MathFunction2D<FT>::in(double v, double h, const INTERPOLATOR_T *interpolator) const
{
	interpolator->ApplyOffsetCorrection(v, h);
	size_t	vi = size_t(v);
	size_t	hi = size_t(h);
	const typename INTERPOLATOR_T::filter_type	*filter = interpolator->GetNeededFilter(v, h);

	return filter->Apply((*this), vi, hi);
}



template<class FT>
typename MathFunction2D<FT>::value_type MathFunction2D<FT>::d_dx(double v, double h) const
{
	return in(v, h, &interpolators2D::bessel_ddx);
}

template<class FT>
typename MathFunction2D<FT>::value_type MathFunction2D<FT>::d_dy(double v, double h) const
{
	return in(v, h, &interpolators2D::bessel_ddy);
}



//--------------------------------------------------------------
//
//	Функция SmoothEdges применяет к данным в таблице.
//	окно вида (cos+1)/2, но только на расстоянии edgeWidth от края
//	и только с заданных сторон. в прочих местах
//	функция остается неизменной. используется мною далее
//	при обработке перекрывающихся участков, но может быть полезной
//	еще где-нибудь
//
//--------------------------------------------------------------



template<class FT>
void	SmoothEdges(MathFunction2D<FT> &x, size_t edgeWidth, array_2D_sides side){
	size_t	vs = x.vsize();
	size_t	hs = x.hsize();
	size_t	os = edgeWidth;

	DataArray<double> column_factor(vs);
	DataArray<double> row_factors(hs);
	column_factor.fill(1);
	column_factor.fill(1);

	for(size_t i = 0; i < vs; i++)
	{
		if((side & array_2D_top) && (i < os)) column_factor[i] = (1. - cos(pi()*double(i)/os))/2;
		if((side & array_2D_bottom) && (i > vs-os)) column_factor[i] = (1. - cos(pi()*double(i-vs)/os))/2;
	}
	for(size_t j = 0; j < hs; j++)
	{
		if((side & array_2D_left) && (j < os)) column_factor[j] = (1. - cos(pi()*double(j)/os))/2;
		if((side & array_2D_right) && (j > hs-os)) column_factor[j] = (1. - cos(pi()*double(j-hs)/os))/2;
	}

	for(size_t i = 0; i < vs; i++)
	{
		for(size_t j = 0; j < hs; j++)
		{
			x.at(i,j) *= (column_factor[j]*column_factor[i]);
		}
	}
}



//--------------------------------------------------------------
//
//	ниже идут функции, которые обрабатывают данные
//	посегментно, с перекрытием и без оного. сделано "по случаю", но кажется, может оказаться
//	полезным. вызов такой:
//
//	class	someImageProcessor : GrayImage{...}; // класс, ответственный за спец. обработку
//
//	GrayImage	somePicture(256,256);
//
//
//	void	func(someImageProcessor *x){x->Process();}
//
//	ProcessTiles(somePicture, 32, func); // применяет обработку к каждой клетке размером 32х32 без перекрытия
//	ProcessTiles(somePicture, 32, func, "Processing"); // то же с прогрессом
//	ProcessOverlaps(somePicture, 32, 4, func); // то же с перекрытием клеток на 4 отсчета
//
//
//
//--------------------------------------------------------------



//! \todo Передать в качестве параметра прогресс вместо message.
template <class ARR2D, class ARR2D_PROC>
void	ProcessTiles(ARR2D &theImage, size_t tileSize, void(*theFun)(ARR2D_PROC*), char *message)
{
	size_t	vs = theImage.vsize();
	size_t	hs = theImage.hsize();

	ARR2D	result(vs, hs);
	ARR2D	seg(tileSize, tileSize);

	size_t	v = ceil(double(vs)/tileSize);
	size_t	h = ceil(double(hs)/tileSize);

	size_t	dv = vs - (v-1)*tileSize;
	size_t	dh = hs - (h-1)*tileSize;

	ARR2D	lastSegH(tileSize, dh);
	ARR2D	lastSegV(dv, tileSize);
	ARR2D	lastSegVH(dv, dh);

	size_t	top, left;

	//if(message) Start_Progress(message, v*h);

	for(size_t i = 0; i < v-1; i++)
	{
		top = i*tileSize;
		for(size_t j = 0; j < h-1; j++)
		{
			left = j*tileSize;
			seg.GetDataSegment(theImage, top, left);
			//извлекается сегмент
			theFun(&seg);
			//вызывается обработчик, который может быть наследником ARR2D
			seg.PutDataSegment(result, top, left);
			//сегмент вставляется на место

			//if(message) Next_Progress();
		}
		{// обработка последнего сегмента, который может иметь другую ширину
			left = (h-1)*tileSize;
			lastSegH.GetDataSegment(theImage, top, left);
			theFun(&lastSegH);
			lastSegH.PutDataSegment(result, top, left);

			//if(message) Next_Progress();
		}
	}

	{// то же для последних сегментов, который имеет другую высоту
		top = (v-1)*tileSize;
		for(size_t j = 0; j < h-1; j++)
		{
			left = j*tileSize;
			lastSegV.GetDataSegment(theImage, top, left);
			theFun(&lastSegV);
			lastSegV.PutDataSegment(result, top, left);

			//if(message) Next_Progress();
		}
		{
			left = (h-1)*tileSize;
			lastSegVH.GetDataSegment(theImage, top, left);
			theFun(&lastSegVH);
			lastSegVH.PutDataSegment(result, top, left);

			//if(message) Next_Progress();
		}
	}

	//if(message) End_Progress();
	theImage.CopyData(result);
}



//! \todo Передать в качестве параметра прогресс, убрать ifdef.
template <class ARR2D, class ARR2D_PROC>
void	ProcessOverlaps(ARR2D &theImage, size_t tileSize, size_t overlapSize, void(*theFun)(ARR2D_PROC*), char *message)
{
	size_t	os = overlapSize;
	size_t	vs = theImage.vsize();
	size_t	hs = theImage.hsize();
	if(!(os%2)) os++;// только нечетное перекрытие
	//size_t	os2 = os/2;
	size_t	step = tileSize - overlapSize;

	ARR2D	result(vs, hs);

	size_t	v = ceil(double(vs-os)/step);
	size_t	h = ceil(double(hs-os)/step);

	size_t	dv = vs - (v-1)*step;
	size_t	dh = hs - (h-1)*step;

	ARR2D	seg(tileSize, tileSize);
	ARR2D	lastSegH(tileSize, dh);
	ARR2D	lastSegV(dv, tileSize);
	ARR2D	lastSegVH(dv, dh);

	//size_t	i,j;
	size_t	top, left;

	result.fill(0);
	array_2D_sides	side;

#ifdef __XRAD_INTERFACE_FUNCTIONS
	if(message) Start_Progress(message, v*h);
#endif

	for(size_t i = 0; i < v-1; i++)
	{
		top = i*step;
		for(size_t j = 0; j < h-1; j++)
		{
			left = j*step;
			seg.GetDataSegment(theImage, top, left);
			//извлекается сегмент
			theFun(&seg);
			//вызывается обработчик, который может быть наследником ARR2D
			side = array_2D_right|array_2D_bottom;
			if(i) side |= array_2D_top;
			if(j) side |= array_2D_left;
			SmoothEdges(seg, os, side);
			seg.PutDataSegment(result, top, left, 1);
			//сегмент вставляется на место

#ifdef __XRAD_INTERFACE_FUNCTIONS
			if(message) Next_Progress();
#endif
		}
		{// обработка последнего сегмента, который может иметь другую ширину
			left = (h-1)*step;
			lastSegH.GetDataSegment(theImage, top, left);
			theFun(&lastSegH);
			side = array_2D_bottom;
			if(left) side |= array_2D_left;
			if(i) side |= array_2D_top;
			SmoothEdges(lastSegH, os, side);
			lastSegH.PutDataSegment(result, top, left, 1);
#ifdef __XRAD_INTERFACE_FUNCTIONS
			if(message) Next_Progress();
#endif
		}
	}
	{// то же для последних сегментов, который имеет другую высоту
		top = (v-1)*step;
		for(size_t j = 0; j < h-1; j++)
		{
			left = j*step;
			lastSegV.GetDataSegment(theImage, top, left);
			theFun(&lastSegV);
			side = array_2D_right;
			if(top) side |= array_2D_top;
			if(j) side |= array_2D_left;
			SmoothEdges(lastSegV, os, side);
			lastSegV.PutDataSegment(result, top, left, 1);
#ifdef __XRAD_INTERFACE_FUNCTIONS
			if(message) Next_Progress();
#endif
		}
		{
			left = (h-1)*step;
			lastSegVH.GetDataSegment(theImage, top, left);
			theFun(&lastSegVH);
			side = array_2D_no_side;
			if(top) side |= array_2D_top;
			if(left) side |= array_2D_left;
			SmoothEdges(lastSegVH, os, side);
			lastSegVH.PutDataSegment(result, top, left, 1);
#ifdef __XRAD_INTERFACE_FUNCTIONS
			if(message) Next_Progress();
#endif
		}
	}

#ifdef __XRAD_INTERFACE_FUNCTIONS
	if(message) End_Progress();
#endif
	theImage.CopyData(result);
}



XRAD_END
