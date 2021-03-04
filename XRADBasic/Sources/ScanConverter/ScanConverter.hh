/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_scan_converter_h
#error "This file should be included through ScanConverter.h"
#endif

#include <XRADBasic/Sources/Utils/TimeProfiler.h>
#include <omp.h>

XRAD_BEGIN

template <class IM_T, class CS_T>
ScanConverter<IM_T,CS_T> :: ScanConverter(size_t nr, size_t ns):IM_T(nr,ns)
	{
// 	image_title = "Cartesian image";

	background_color = converted_sample_type(0);
	// по ошибке строчка инициализации background_color была закомментирована.
	// компилятор не дает предупреждения о неинициализированных членах класса,
	// так как к ним всегда применяется конструктор по умолчанию. но когда в
	// качестве sample_t выступает float, никакого конструктора не вызывается.
	// в результате отображаемый растр оказывается наполовину заполнен мусором
	// вместо положенного фонового цвета. дело осложняется еще тем, что довольно
	// часто по различным причинам область памяти, в которой создается объект,
	// уже проинициализирована нулями, так что созданный объект оказывается
	// вполне работоспособным. из-за этого ошибка проявляется редко, опознать
	// ее оказалось сложно. кнс 17.12.09


	grid_color = converted_sample_type(255);//ColorPixel(0, 127, 0);
	inited = false;
	flip = false;
	}


template <class IM_T, class CS_T>
ScanConverter<IM_T,CS_T> :: ~ScanConverter()
	{
	}


// template <class IM_T, class CS_T>
// void	ScanConverter<IM_T,CS_T> :: SetImageTitle(const string &new_title)
// 	{
// 	image_title = new_title;
// 	}

//--------------------------------------------------------------
//
//	инициализация
//
//--------------------------------------------------------------

template <class IM_T, class CS_T>
void	ScanConverter<IM_T,CS_T> :: SetFlip(bool fl)
	{
	if(flip == fl) return;
	if(!inited) flip = fl;
	else
		{
		flip = fl;
		InitScanConverter(n_rows, n_cols);
		}
	}

template <class IM_T, class CS_T>
void	ScanConverter<IM_T,CS_T> :: SetBackground(typename ScanConverter<IM_T,CS_T>::converted_sample_type c)
	{
	if(background_color == c) return;
	if(!inited) background_color = converted_sample_type(c);
	else
		{
		background_color = converted_sample_type(c);
		InitScanConverter(n_rows, n_cols);
		}
	}


template <class IM_T, class CS_T>
void	ScanConverter<IM_T,CS_T> :: InitScanConverter(size_t nr, size_t nc)
	{

	if(!d_angle().radians() && !scanning_trajectory_length().cm() && !r_max().cm())
		{
		// если область отображения пустая, то делаем ее равной (в пикселях)
		// исходному массиву
		physical_length w = cm(hsize())/pixels_per_cm;
		physical_length h = cm(hsize())/pixels_per_cm;
		SetFrameRectangle(w, h);
		}

	physical_angle	max_angle = max(absolute_value(end_angle()), absolute_value(start_angle()));

	last_sample = ptrdiff_t(r_max()/dr());
	first_sample = ptrdiff_t(r_min()/dr());

	physical_length	first_row_heigth;
	physical_length	image_heigth;

	if(d_angle().radians())
		{
		first_row_heigth = r_min()*cosine(max_angle);
		image_heigth = r_max() - first_row_heigth;
		}
	else
		{
		first_row_heigth = cm(0);
		image_heigth = r_max() - r_min();
		}

	if(!nr)
		{
		// высота изображения по умолчанию
		//n_rows = size_t(r_max()*pixels_per_cm);
		n_rows = size_t(image_heigth.cm()*pixels_per_cm);
		}
	else
		{
		// высота изображения в пикселах равна nr
		n_rows = nr;
		pixels_per_cm = n_rows/image_heigth.cm();
		}

	if(d_angle().radians())
		{
		// секторный датчик
//		if(!nc) n_cols = 2*n_rows*sine(max_angle);
		if(!nc) n_cols = size_t(2*r_max().cm()*pixels_per_cm*sine(max_angle));
		else n_cols = nc;
		first_row = ptrdiff_t(first_row_heigth.cm() * pixels_per_cm);
		}
	else if(scanning_trajectory_length().cm())
		{
		// линейный датчик
		if(!nc) n_cols = size_t(scanning_trajectory_length().cm()*pixels_per_cm);
		else n_cols = nc;
		first_row = 0;
		}
	else
		{
		ForceDebugBreak();
		throw invalid_argument(typeid(self).name() + string("::InitScanConverter -- invalid arguments."));
		}


	middle_col = n_cols/2;

	if(d_angle().radians())
		{
		if(start_angle().radians() < 0 ) start_angle_ctg = 1./tangent(start_angle());
		else	start_angle_ctg = 0;
		if(end_angle().radians() > 0 ) end_angle_ctg = 1./tangent(end_angle());
		else end_angle_ctg = 0;
		first_col = ptrdiff_t(middle_col + (n_rows + first_row)*sine(start_angle()));
		last_col = ptrdiff_t(middle_col + (n_rows + first_row)*sine(end_angle()));
		}
	else
		{
		start_angle_ctg = 0;
		end_angle_ctg = 0;
		first_col = ptrdiff_t(middle_col - pixels_per_cm*scanning_trajectory_length().cm()/2);
		last_col = ptrdiff_t(middle_col + pixels_per_cm*scanning_trajectory_length().cm()/2);
		}

	first_col = range(first_col, 0, middle_col);
	last_col = range(last_col, middle_col, ptrdiff_t(n_cols));


	converted_image.realloc(n_rows, n_cols);
	converted_image.fill(background_color);

	interpolator.realloc(n_rows, n_cols);
	interpolator.fill(interpolator_t(background_color));

	inited = true;
	// все зааллокировано, осталось заполнить таблицу

	for(ptrdiff_t col = first_col; col < last_col; col ++)
		{
		ptrdiff_t	min_row;
		if(col < middle_col) min_row = ptrdiff_t((col - middle_col)*start_angle_ctg - first_row);
		else min_row = ptrdiff_t((col - middle_col)*end_angle_ctg - first_row);

		min_row = range(min_row, 0, n_rows);

		for(ptrdiff_t row = min_row; row < ptrdiff_t(n_rows); row ++)
			{
			ray_sample_coord	rs = GetRaySampleCoords(row, col);
			if(rs.ray_f > 0 && rs.ray_f < n_rays()-1)
				{
				if(rs.sample_f <= 0)interpolator.at(row,col) = interpolator_t(converted_sample_type(0));// black
				else if(rs.sample_f < n_samples()-1)
					{
					interpolator.at(row,col) = interpolator_t(rs.ray_f,rs.sample_f, *this);
					}
				}
			}
		}
	}


//--------------------------------------------------------------
//
//	преобразования координат
//
//--------------------------------------------------------------


template <class IM_T, class CS_T>
ray_sample_coord ScanConverter<IM_T,CS_T> :: GetRaySampleCoords(double row_f, double col_f) const
	{
	double	ray_f, sample_f;
	double	row_abs = row_f + first_row;

	if(flip) col_f = n_cols - col_f - 1; // flip
	double	col_abs = col_f - middle_col;


	if(!inited)
		{
		ray_f = sample_f = 0;
		//return ray_sample_coord(0,0);
		}
	else if(d_angle().radians())
		{
		double	radius = sqrt(col_abs*col_abs + row_abs*row_abs);
		physical_angle	angle = radians(radius ? std::atan2(col_abs, row_abs) : 0);

		sample_f = last_sample*radius/(n_rows + first_row) - first_sample;
		ray_f = (angle - start_angle())/d_angle();
		}
	else if(scanning_trajectory_length().cm())
		{
		sample_f = last_sample*row_abs/(n_rows + first_row) - first_sample;
		ray_f = n_rays()/2 + col_abs/(dx().cm()*pixels_per_cm);
		}
	else
		{
		ForceDebugBreak();
		throw invalid_argument(typeid(self).name() + string("::GetRaySampleCoords -- invalid arguments."));
		}
	return ray_sample_coord(ray_f, sample_f);
	}

template <class IM_T, class CS_T>
ray_sample_coord ScanConverter<IM_T,CS_T> :: GetRaySampleCoords(row_col_coord rc) const
	{
	return GetRaySampleCoords(rc.row_f, rc.col_f);
	}


template <class IM_T, class CS_T>
row_col_coord ScanConverter<IM_T,CS_T> :: GetRowColCoords(double ray_f, double sample_f) const
	{
	double	row_abs, col_abs;

	if(!inited)
		{
		return row_col_coord(0,0);
		}
	//+++
	else if(d_angle().radians())
		{
		double	radius = (sample_f + first_sample)*double(n_rows + first_row)/last_sample;
		physical_angle	angle = ray_f*d_angle() + start_angle();

		row_abs = radius * cosine(angle);
		col_abs = radius * sine(angle);

//		обращение формул:
//		double	radius = sqrt(col_abs*col_abs + row_abs*row_abs);
//		double	angle = radius ? atan2(col_abs, row_abs) : 0;
//		sample_f = last_sample*radius/(n_rows + first_row) - first_sample;
//		ray_f = (angle - start_angle)/d_angle();
		}
	else if(scanning_trajectory_length().cm())
		{
		row_abs = double(sample_f + first_sample)*double(n_rows + first_row)/last_sample;
		col_abs = (ray_f - n_rays()/2) * (dx().cm()*pixels_per_cm);
//		обращение формул:
//		sample_f = last_sample*row_abs/(n_rows + first_row) - first_sample;
//		ray_f = n_rays()/2 + col_abs/(dx()*pixels_per_cm);
		}
	else
		{
		ForceDebugBreak();
		throw invalid_argument(typeid(self).name() + string("::GetRowColCoords -- invalid arguments."));
		}


	double	row_f = row_abs - first_row;
	double	col_f = col_abs + middle_col;
	if(flip) col_f = n_cols - col_f - 1; // flip


	return row_col_coord(row_f, col_f);
	}

template <class IM_T, class CS_T>
row_col_coord ScanConverter<IM_T,CS_T> :: GetRowColCoords(ray_sample_coord rs) const
	{
	return GetRowColCoords(rs.ray_f, rs.sample_f);
	}

template <class IM_T, class CS_T>
void	ScanConverter<IM_T,CS_T> :: GetRasterDimensions(physical_length &vmin, physical_length &vmax, physical_length &hmin, physical_length &hmax) const
	{
	physical_length	first_row_heigth;
	physical_length	image_heigth;
	physical_angle	max_angle = max(absolute_value(end_angle()), absolute_value(start_angle()));

	if(d_angle().radians())
		{
		vmax = r_max() - r_min();
		}
	else
		{
		vmax = r_max() - r_min()*cosine(max_angle);
		}
	vmin = cm(0);

	if(d_angle().radians())
		{
		// секторный датчик
		hmax = r_max() * sine(max_angle);
		}
	else if(scanning_trajectory_length().cm())
		{
		// линейный датчик
		hmax = scanning_trajectory_length()/2;
		}
	hmin = -hmax;
	}


//--------------------------------------------------------------
//
//	утилиты
//
//--------------------------------------------------------------



template <class IM_T, class CS_T>
void	ScanConverter<IM_T,CS_T> :: DrawPalette()
	{
	if(!inited)
		{
		ForceDebugBreak();
		throw logic_error(typeid(self).name() + string("::DrawPalette -- Scan converter not initialized"));
		}
	for(uint32_t col = 5; col < 20; col ++)
		{
		for(uint32_t row = 0; row < 256; row ++)
			{
				if(row < n_rows-53) converted_image.at(row+53,col) = static_cast<converted_sample_type>(row);
			}
		}
	}


template <class IM_T, class CS_T>
void	ScanConverter<IM_T,CS_T> :: DrawGrid()
	{
	if(!inited)
		{
		ForceDebugBreak();
		throw logic_error(typeid(self).name() + string("::DrawGrid -- Scan converter not initialized"));
		}
	size_t	grid_step_pixels = size_t(grid_step.cm()*pixels_per_cm);
	for(size_t col = 0; col < n_cols; col += grid_step_pixels)
		{
		for(size_t row = 0; row < n_rows; row += 2) converted_image.at(row,col) = grid_color;
		}
	for(size_t col = 0; col < n_cols; col += 2)
		{
		for(size_t row = 0; row < n_rows; row += grid_step_pixels) converted_image.at(row,col) = grid_color;
		}
	}


template <class IM_T, class CS_T>
void	ScanConverter<IM_T,CS_T> :: BuildConvertedImage()
	{
	if(!inited)
		{
		ForceDebugBreak();
		throw logic_error(typeid(self).name() + string("::BuildConvertedImage -- Scan converter not initialized"));
		}

	//TODO здесь на пробу включаю испльзование omp. посмотреть, как скажется на работе в целом.
	// 2015_09_02 после долгого использования попробовал отключить. стало хуже. вернул назад. продолжать наблюдение
	ThreadErrorCollector ec("ScanConverter::BuildConvertedImage");
	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t row = 0; row < ptrdiff_t(n_rows); row ++)
		{
		if (ec.HasErrors())
			{
#ifdef XRAD_COMPILER_MSC
			break;
#else
			continue;
#endif
			}
		ThreadSetup ts; (void)ts;
		try
			{
			typename converted_image_type::row_type::iterator raster_it = converted_image.row(row).begin();
			typename converted_image_type::row_type::iterator raster_end = converted_image.row(row).end();
			typename DataArray<interpolator_t>::iterator interpolator_it = interpolator.row(row).begin();

			for(; raster_it < raster_end; ++raster_it, ++interpolator_it)
				{
				*raster_it = interpolator_it->interpolate();
				}
			}
		catch (...)
			{
				ec.CatchException();
			}
		}
	ec.ThrowIfErrors();

	if(draw_grid) DrawGrid();
	if(add_palette) DrawPalette();
	}





XRAD_END
