#ifndef RasterImageSet_h__
#define RasterImageSet_h__

/********************************************************************
	created:	2016/02/05
	created:	5:2:2016   15:31
	author:		kns
*********************************************************************/

#include "DataDisplayer.h"
#include "DisplaySampleType.h"
#include <XRADBasic/MathFunctionTypes2D.h>

XRAD_BEGIN

//--------------------------------------------------------------

class	RasterImageSet : public DataDisplayer
{
	ImageWindowContainer	&image_container(){ return static_cast<ImageWindowContainer&>(*window); }
	const ImageWindowContainer	&image_container() const{ return static_cast<ImageWindowContainer&>(*window); }


	bool	SetupFrame(int in_frame_no, const void* frame_data, display_sample_type pt);
	bool	InsertFrame(int after_frame_no, const void* frame_data, display_sample_type pt);

	const size_t	m_vsize, m_hsize;

public:
	RasterImageSet(const wstring &title, size_t vs, size_t hs);
	~RasterImageSet() = default;


	size_t	vsize() const { return m_vsize; }
	size_t	hsize() const { return m_hsize; }

	bool	SetAxesScales(double in_z0, double in_dz, double in_y0, double in_dy, double in_x0, double in_dx);
	bool	SetDefaultBrightness(double in_black, double in_white, double in_gamma);
	bool	SetLabels(const wstring &in_title, const wstring &in_z_label, const wstring &in_y_label, const wstring &in_x_label, const wstring &in_value_label);

	bool	AddFrames(size_t n_frames);

	template<class ROW_T>
	bool SetupFrame(int in_frame_no, const DataArray2D<ROW_T> &frame);

	template<class ROW_T>
	bool InsertFrame(int after_frame_no, const DataArray2D<ROW_T> &frame);
	bool DeleteFrame(int in_frame_no);
};



template<class ROW_T>
bool RasterImageSet::InsertFrame(int after_frame_no, const DataArray2D<ROW_T> &frame)
{
	display_sample_type pt = DisplaySampleType<typename ROW_T::value_type>();
	if(frame.row_step() == 1 && frame.column_step() == ptrdiff_t(frame.hsize()) && frame.vsize()==vsize() && frame.hsize()==hsize())
	{
		return InsertFrame(after_frame_no, frame.data(), pt);
	}
	else
	{
		DataArray2D<DataArray<typename ROW_T::value_type_variable>> buffer(vsize(), hsize());
		buffer.CopyData(frame);
		return InsertFrame(after_frame_no, buffer.data(), pt);
	}
}



template<class ROW_T>
bool RasterImageSet::SetupFrame(int in_frame_no, const DataArray2D<ROW_T> &frame)
{
	display_sample_type pt = DisplaySampleType<typename ROW_T::value_type>();

	if(frame.row_step() == 1 && frame.column_step() == ptrdiff_t(frame.hsize()) && frame.vsize()==vsize() && frame.hsize()==hsize())
	{
		return SetupFrame(in_frame_no, frame.data(), pt);
	}
	else
	{
		DataArray2D<DataArray<typename ROW_T::value_type_variable>> buffer(vsize(), hsize());
		buffer.CopyData(frame);
		return SetupFrame(in_frame_no, buffer.data(), pt);
	}
}

//--------------------------------------------------------------

XRAD_END

#endif // RasterImageSet_h__