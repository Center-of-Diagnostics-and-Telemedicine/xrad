
#ifndef __RasterImageFile_hh
//#error This file must be included from "RasterImageFile.h" only
#endif



XRAD_BEGIN



template<class RGB_IMAGE_T> 
RGB_IMAGE_T file::raster_image::rgb()
{
	using value_type = typename RGB_IMAGE_T::value_type;
	ColorImageF64	buffer = rgb_internal();
	double	scalefactor = scalefactor_calculator<value_type>::get(m_bits_per_channel);
	
	RGB_IMAGE_T result;
	MakeCopy(result, buffer, [scalefactor](value_type &y, const auto &x){return y = x*scalefactor;});
	return result;
}

template <typename IMAGE_T> IMAGE_T file::raster_image::channel(color_channel in_channel)
{
	using value_type = typename IMAGE_T::value_type;
	RealFunction2D_F64	buffer = channel_internal(in_channel);
	double scalefactor, offset;
	if(in_channel == color_channel::hue)
	{
		XRAD_ASSERT_THROW_M(numeric_limits<value_type>::max() > 360, invalid_argument, ssprintf("datatype is to small for hue"));
		scalefactor = 180.;
		offset = -1.;
	}
	else
	{
		scalefactor = value_scalefactor_calculator<value_type>::get(m_bits_per_channel);
		offset = 0.;
	}

	IMAGE_T	result;
	result.MakeCopy(buffer, [scalefactor, offset](value_type &y, const auto &x){return y = (x-offset)*scalefactor;});

	return result;
}

template <typename IMAGE_T>
IMAGE_T file::raster_image::lightness()
{
	return channel(color_channel::lightness);
}


XRAD_END
