#ifndef XRAD__File_hls_color_sample_h
#define XRAD__File_hls_color_sample_h

#include "ColorSample.h"

XRAD_BEGIN

class	HLSColorSample
{
	void	ToRGB(float &r, float &g, float &b) const;
	void	FromRGB(float r, float g, float b);
public:
	float	H, L, S;

	HLSColorSample(){H=L=S=0;};
	HLSColorSample(const ColorSampleF64 &);
	HLSColorSample(const ColorPixel &);

//	operator ColorPixel() const;
	operator ColorSampleF64() const;
};

XRAD_END

#endif //XRAD__File_hls_color_sample_h
