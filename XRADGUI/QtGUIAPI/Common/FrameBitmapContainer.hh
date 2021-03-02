/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
namespace XRAD_GUI
{



inline const void* FrameBitmapContainer::GetBitmapFile() const
	{
	switch(current_sample_type)
		{
		case rgba_sample_ui8:
			return rgb.GetBitmapFile();
			break;
		case gray_sample_ui8:
		case indexed_color_sample_ui8:
			return indexed.GetBitmapFile();
			break;
		default:
			throw invalid_argument(ssprintf("FrameBitmapContainer::GetBitmapFile(), invalid pixel type = %d", int(current_sample_type)));
		};
	}

inline size_t FrameBitmapContainer::GetBitmapFileSize() const
	{
	switch(current_sample_type)
		{
		case rgba_sample_ui8:
			return rgb.GetBitmapFileSize();
			break;
		case gray_sample_ui8:
		case indexed_color_sample_ui8:
			return indexed.GetBitmapFileSize();
			break;
		default:
			throw invalid_argument(ssprintf("FrameBitmapContainer::GetBitmapFileSize(), invalid pixel type = %d", int(current_sample_type)));
		};
	}

// для скалярных данных шаблон
template<class T, class FUNCTOR>
void FrameBitmapContainer::SetFrameData(const xrad::DataArray2D<xrad::DataArray<T> >& frame, bool transpose, const FUNCTOR &f)
	{
	// переворачиваем картину, т.к. в структуре bmp развертка идет снизу вверх
	if(transpose)
		{
		SetFrameSizes(frame.hsize(),frame.vsize());
		for(size_t i = 0; i < frame.hsize(); ++i)
			{
			const DataArray<T> &col(frame.col(frame.hsize()-i-1));
			CopyData(indexed.row(i), col, f);
			}
		}
	else
		{
		SetFrameSizes(frame.vsize(),frame.hsize());
		for(size_t i = 0; i < frame.vsize(); ++i)
			{
			const DataArray<T> &row(frame.row(frame.vsize()-i-1));
			CopyData(indexed.row(i), row, f);
			}
		}
	current_sample_type = gray_sample_ui8;
	}

// для цветного растра отдельная функция
template<class T, class FUNCTOR>
void FrameBitmapContainer::SetFrameData(const xrad::DataArray2D<xrad::DataArray<RGBColorSample<T> > >& frame, bool transpose, const FUNCTOR &f)
	{
	// переворачиваем картину, т.к. в структуре bmp развертка идет снизу вверх
	if(transpose)
		{
		SetFrameSizes(frame.hsize(),frame.vsize());
		for(size_t i = 0; i < frame.hsize(); ++i)
			{
		//	const DataArray<RGBColorSample<T> > &col(frame.col(frame.hsize()-i-1));
			CopyData(rgb.row(i), frame.col(frame.hsize()-i-1), f);
			}
		}
	else
		{
		SetFrameSizes(frame.vsize(),frame.hsize());
		for(size_t i = 0; i < frame.vsize(); ++i)
			{
		//	const DataArray<ColorPixel> &row(frame.row(frame.vsize()-i-1));
			CopyData(rgb.row(i), frame.row(frame.vsize()-i-1), f);
			}
		}
	current_sample_type = rgba_sample_ui8;
	}



}//namespace
