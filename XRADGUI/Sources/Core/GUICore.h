/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef GUICore_h__
#define GUICore_h__
/*!
	\file
	\date 2017/06/07 14:05
	\author kulberg

	\brief определения типов, которые будут общими между GUI и GUIApi
*/

#include <XRADBasic/Core.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

XRAD_BEGIN



//--------------------------------------------------------------
//
//	basic graphics
//
//--------------------------------------------------------------

enum out_of_range_control
{
	out_of_range_allowed = true,
	out_of_range_prohibited = false
};

struct axis_legend
{
	double	min_value, step;
	wstring	label;

	axis_legend(double in_v0, double in_dv, wstring in_label);
	axis_legend(double in_v0, double in_dv, string in_label);

// 	axis_legend &operator=(const axis_legend &original){min_value=original.min_value;step=original.step;label=original.label; return *this;}
};

struct value_legend
{
	range1_F64	display_range;
	double	gamma;
	wstring	label;

	value_legend(double in_min, double in_max, double in_gamma, wstring in_label);
	value_legend(double in_min, double in_max, double in_gamma, string in_label);

	value_legend(const range1_F64 &in_display_range, double in_gamma, wstring in_label);
	value_legend(const range1_F64 &in_display_range, double in_gamma, string in_label);
};

// объект, который идентифицирует виджет с графиками, изображениями и текстом.
// пользуясь им,можно изменять ранее нарисованные графики, добавлять
// и удалять кривые и т.п.
// в заголовке намеренно нет информации об устройстве этого класса
// то же для набора растровых картинок

struct	DataWindowContainer
{
	void *window_ptr;
	DataWindowContainer() : window_ptr(nullptr){}
};


struct	TextWindowContainer : public DataWindowContainer
{
};


inline size_t graph_set_new_graph(){ return size_t(-1); }//временно вспомогательная функция, потом сделать что-то общее для графиков и изображений

struct	GraphWindowContainer : public DataWindowContainer
{
};

struct	ImageWindowContainer : public DataWindowContainer
{
};

enum graph_value_transforms
{
	e_transform_none,
	e_uniform_scale,
	e_log_compress,
	e_1st_derivative,
	e_2nd_derivative,
	e_integral,
	e_sum
};

enum graph_line_style
{
	solid_color_lines,
	dashed_black_lines,
	dashed_color_lines,
	textured_black_lines,
	textured_color_lines
};

enum
{
	n_graph_line_styles = textured_color_lines + 1
};



XRAD_END

#endif // GUICore_h__
