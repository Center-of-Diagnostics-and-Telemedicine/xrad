#include "pre.h"
#include "RasterImageSet.h"

#include <XRADGUIAPI.h>

/********************************************************************
	created:	2016/02/05
	created:	5:2:2016   15:31
	author:		kns
*********************************************************************/

XRAD_BEGIN

using namespace XRAD_GUI;



RasterImageSet::RasterImageSet(const wstring &title, size_t vs, size_t hs) : m_vsize(vs), m_hsize(hs)
{
	window.reset(new ImageWindowContainer(api_CreateRasterImageSet(title, vs, hs)));
}

bool RasterImageSet::AddFrames(size_t n_frames)
{
	return api_AddImageFrames(image_container(), n_frames);
}


bool RasterImageSet::SetAxesScales(double in_z0, double in_dz, double in_y0, double in_dy, double in_x0, double in_dx)
{
return api_SetImageAxesScales(image_container(), in_z0, in_dz, in_y0, in_dy, in_x0, in_dx);
}

bool RasterImageSet::SetDefaultBrightness(double in_black, double in_white, double in_gamma)
{
return api_SetImageDefaultBrightness(image_container(), in_black, in_white, in_gamma);
}


bool	RasterImageSet::SetupFrame(int in_frame_no, const void* data, display_sample_type pt)
{
	return api_SetupImageFrame(image_container(), in_frame_no, data, pt);
}
bool RasterImageSet::InsertFrame(int after_frame_no, const void* data, display_sample_type pt)
{
	return api_InsertImageFrame(image_container(), after_frame_no, data, pt);
}


bool RasterImageSet::SetLabels(const wstring &in_title, const wstring &in_z_label, const wstring &in_y_label, const wstring &in_x_label, const wstring &in_value_label)
{
	return api_SetImageLabels(image_container(), in_title, in_z_label, in_y_label, in_x_label, in_value_label);
}


XRAD_END