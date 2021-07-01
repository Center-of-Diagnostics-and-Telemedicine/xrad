/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "GradientPalette.h"
#include "Crayons.h"

XRAD_BEGIN

GradientPalette::GradientPalette(const std::initializer_list<double> &in_positions, const std::initializer_list<ColorSampleF64> &in_colors)
{
	XRAD_ASSERT_THROW(in_positions.size()==in_colors.size());
	XRAD_ASSERT_THROW(in_positions.size() > 0);
	auto	p = in_positions.begin();
	auto	c = in_colors.begin();

	double	previous_color_position = *p;
	for(;p<in_positions.end(); ++p, ++c)
	{
		XRAD_ASSERT_THROW(*p>=previous_color_position); // индекс цвета не должен убывать
		colors.push_back(make_pair(*p, *c));
		previous_color_position = *p;
	}
}

GradientPalette::GradientPalette(const std::initializer_list<base_color_t> in_colors)
{
	auto	c = in_colors.begin();

	double	previous_color_position = c->first;
	for(;c<in_colors.end(); ++c)
	{
		XRAD_ASSERT_THROW(c->first >= previous_color_position); // индекс цвета не должен убывать
		colors.push_back(*c);
		previous_color_position = c->first;
	}
}


GradientPalette::GradientPalette(size_t n_colors) : colors(n_colors, make_pair(0, crayons::black()))
{
}


void	GradientPalette::SetColor(size_t color_no, const base_color_t &new_color)
{
	XRAD_ASSERT_THROW(in_range(color_no, 0, colors.size()-1));
	colors[color_no] = new_color;
}

void	GradientPalette::MoveColor(size_t color_no, double new_color_position)
{
	XRAD_ASSERT_THROW(in_range(color_no, 0, colors.size()-1));
	if(colors.size() > 1)
	{
		if(color_no == 0)
		{
			XRAD_ASSERT_THROW(new_color_position<=colors[1].first);
		}
		else if(color_no == colors.size()-1)
		{
			XRAD_ASSERT_THROW(new_color_position>=colors[colors.size()-2].first);
		}
		else
		{
			XRAD_ASSERT_THROW(in_range(new_color_position, colors[color_no-1].first, colors[color_no+1].first));
		}
	}
	colors[color_no].first = new_color_position;
}


ColorSampleF64 GradientPalette :: operator()(double color_position)
{
	ColorSampleF64	result(0, 0, 0);
	if(!colors.size()) return result;
	if(color_position <= colors[0].first)
	{
		return colors[0].second;
	}
	if(color_position >= colors[colors.size()-1].first)
	{
		return colors[colors.size()-1].second;
	}

	size_t	n1 = 0;
	size_t	n2 = colors.size()-1;

	while(color_position > colors[n1].first && n1 < colors.size()-1) ++n1;
	while(color_position < colors[n2].first && n2 > 0) --n2;

	double	w0 = colors[n1].first - colors[n2].first;
	if(w0 <= 0) return colors[n1].second;

	double	w1 = (color_position - colors[n2].first)/w0;
	double	w2 = (colors[n1].first - color_position)/w0;

	return (colors[n1].second*w1) + (colors[n2].second*w2);
}

XRAD_END
