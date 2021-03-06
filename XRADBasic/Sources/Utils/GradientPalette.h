﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_gradient_palette_h
#define XRAD__File_gradient_palette_h

#include <XRADBasic/Sources/SampleTypes/ColorSample.h>
#include <vector>

XRAD_BEGIN

/*!
	\brief GradientPalette генератор градиентной палитры

	Содержит список цветов, привязанных к определенным позициям в палитре.
	Оператор () возвращает интерполированное значение цвета в координатах положений исходных цветов.

	Задание сразу:
	GradientPalette	gp({0,200,500}, {crayons::green, crayons::blue, crayons::yellow});
	gp.MoveColor(0, -100);

	Задание палитры по частям с последующим изменением
	GradientPalette	gp(3, 0, 500);
	gp.SetColor(0, make_pair(0, crayons::green));
	gp.SetColor(1, make_pair(200, crayons::blue));
	gp.SetColor(2, make_pair(500, crayons::red));

	смещение одного из цветов
	gp.MoveColor(1, 400);

	ColorFunction	f(500);
	for(size_t i = 0; i < 500; ++i) palette[i] = gp(i);

*/


class	GradientPalette
{
public:
	typedef  pair<double, ColorSampleF64> base_color_t;

	GradientPalette(const std::initializer_list<double>& in_positions, const std::initializer_list<ColorSampleF64>& in_colors);
	GradientPalette(const std::initializer_list<base_color_t> in_colors);
	GradientPalette(size_t n_colors);

	void	SetColor(size_t color_no, const base_color_t& new_color);
	void	MoveColor(size_t color_no, double new_color_position);
	ColorSampleF64	operator()(double color_position);

	size_t	size() const { return colors.size(); }

private:
	std::vector<base_color_t>	colors;
};

XRAD_END



#endif // XRAD__File_gradient_palette_h
