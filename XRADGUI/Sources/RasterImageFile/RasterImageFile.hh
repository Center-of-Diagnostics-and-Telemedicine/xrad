
#ifndef __RasterImageFile_hh
//#error This file must be included from "RasterImageFile.h" only
#endif



XRAD_BEGIN



//template <typename IMG_T, typename ST>
template <typename IMG_T>
void RasterImageFile::GetRGB(IMG_T& arg)
{
	arg.resize(m_sizes[0], m_sizes[1]);

	ColorImageF64	buffer = GetRGBInternal();

	double	f = scalefactor_calculator<typename IMG_T::value_type>::get(m_bits_per_channel);

	for (size_t i = 0; i < m_sizes[0]; i++)
	{
		for (size_t j = 0; j < m_sizes[1]; j++)
		{
		ColorSampleF64	&p = buffer.at(int(i), int(j));
		arg.at(i, j) = typename IMG_T::value_type(p.red()*f, p.green()*f, p.blue()*f);
		}
	}
};

template <typename ROW_T>
void RasterImageFile::GetChannel(color_type c, MathFunction2D<ROW_T>& arg)
{

	XRAD_ASSERT_THROW_M(numeric_limits<typename ROW_T::value_type>::max() > 360, invalid_argument, ssprintf("datatype is to small for hue"));

	arg.resize(m_sizes[0], m_sizes[1]);

	RealFunction2D_F64	buffer = GetChannelInternal(c);
	double f, offset;
	if (c == color_type::e_H)
	{
		f = 180.;
		offset = -1.;
	}
	else
	{
		f = sf_calculator<typename ROW_T::value_type>::get(m_bits_per_channel);
		offset = 0.;
	}

	for (size_t i = 0; i < m_sizes[0]; i++)
	{
		for (size_t j = 0; j < m_sizes[1]; j++)
		{
			arg.at(i, j) = typename ROW_T::value_type((buffer.at(i, j) - offset)*f);
		}
	}
}

XRAD_END
