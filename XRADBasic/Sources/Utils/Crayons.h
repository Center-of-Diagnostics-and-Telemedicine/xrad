/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_crayons_h
#define XRAD__File_crayons_h

#include <XRADBasic/Sources/SampleTypes/ColorSample.h>

/*
здесь определяется набор констант, определяющих ряд
наиболее употребимых цветов в формате ColorSampleF64.
любой из этих цветов может:

- складываться с другим (смешение)
crayons::red+crayons::green === crayons::yellow

- умножаться на число
crayons::white * 0.5 === crayons::gray_50

- менять насыщенность
crayons::red % 0 === crayons::white
crayons::red % 0.5 === crayons::red/2 + crayons::gray_50
*/

XRAD_BEGIN

namespace	crayons
{
	inline ColorSampleF64	black(){return ColorSampleF64(0,0,0);}
	inline ColorSampleF64	white(){ return ColorSampleF64(255, 255, 255); }
	inline ColorSampleF64	gray_25(){ return ColorSampleF64(64, 64, 64); }
	inline ColorSampleF64	gray_50(){ return ColorSampleF64(128, 128, 128); }
	inline ColorSampleF64	gray_75(){ return ColorSampleF64(192, 192, 192); }

	inline ColorSampleF64	red(){ return ColorSampleF64(255, 0, 0); }
	inline ColorSampleF64	green(){ return ColorSampleF64(0,255,0); }
	inline ColorSampleF64	blue(){ return ColorSampleF64(0,0,255); }
	inline ColorSampleF64	magenta(){ return ColorSampleF64(255,0,255); }
	inline ColorSampleF64	yellow(){ return ColorSampleF64(255,255,0); }
	inline ColorSampleF64	cyan(){ return ColorSampleF64(0,255,255); }

	inline ColorSampleF64	violet(){ return ColorSampleF64(128, 0, 256); }
	inline ColorSampleF64	orange(){ return ColorSampleF64(255, 128, 0); }
	inline ColorSampleF64	turquoise(){ return ColorSampleF64(0, 255, 180); }// бирюзовый цвет
	inline ColorSampleF64	brown(){ return ColorSampleF64(140, 80, 0); }

	inline ColorSampleF64	dark_red(){ return ColorSampleF64(100,0,0); }
	inline ColorSampleF64	dark_green(){ return ColorSampleF64(0,100,0); }
	inline ColorSampleF64	dark_blue(){ return ColorSampleF64(0,0,100); }
	inline ColorSampleF64	dark_magenta(){ return ColorSampleF64(100,0,100); }
	inline ColorSampleF64	dark_yellow(){ return ColorSampleF64(100,100,0); }
	inline ColorSampleF64	dark_cyan(){ return ColorSampleF64(0,100,100); }
};



XRAD_END

#endif //XRAD__File_crayons_h
