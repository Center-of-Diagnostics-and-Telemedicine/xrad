/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "GraphStyleSet.h"



// для памяти кладу здесь описание цветов Qt
// Qt::white	3	White (#ffffff)
// Qt::black	2	Black (#000000)
// Qt::red	7	Red (#ff0000)
// Qt::darkRed	13	Dark red (#800000)
// Qt::green	8	Green (#00ff00)
// Qt::darkGreen	14	Dark green (#008000)
// Qt::blue	9	Blue (#0000ff)
// Qt::darkBlue	15	Dark blue (#000080)
// Qt::cyan	10	Cyan (#00ffff)
// Qt::darkCyan	16	Dark cyan (#008080)
// Qt::magenta	11	Magenta (#ff00ff)
// Qt::darkMagenta	17	Dark magenta (#800080)
// Qt::yellow	12	Yellow (#ffff00)
// Qt::darkYellow	18	Dark yellow (#808000)
// Qt::gray	5	Gray (#a0a0a4)
// Qt::darkGray	4	Dark gray (#808080)
// Qt::lightGray	6	Light gray (#c0c0c0)
// Qt::transparent	19	a transparent black value (i.e., QColor(0, 0, 0, 0))
// Qt::color0	0	0 pixel value (for bitmaps)
// Qt::color1	1	1 pixel value (for bitmaps)
//
//
// enum PenStyle
// Qt::NoPen	0	no line at all. For example, QPainter::drawRect() fills but does not draw any boundary line.
// Qt::SolidLine	1	A plain line.
// Qt::DashLine	2	Dashes separated by a few pixels.
// Qt::DotLine	3	Dots separated by a few pixels.
// Qt::DashDotLine	4	Alternate dots and dashes.
// Qt::DashDotDotLine	5	One dash, two dots, one dash, two dots.
// Qt::CustomDashLine	6	A custom pattern defined using QPainterPathStroker::setDashPattern().

// Qt::SolidLine

//     enum BrushStyle { // brush pen_style
//         NoBrush,
//         SolidPattern,
//         Dense1Pattern,
//         Dense2Pattern,
//         Dense3Pattern,
//         Dense4Pattern,
//         Dense5Pattern,
//         Dense6Pattern,
//         Dense7Pattern,
//         HorPattern,
//         VerPattern,
//         CrossPattern,
//         BDiagPattern,
//         FDiagPattern,
//         DiagCrossPattern,
//         LinearGradientPattern,
//         RadialGradientPattern,
//         ConicalGradientPattern,
//         TexturePattern = 24
//     };

//--------------------------------------------------------------
//
//	класс GraphStyles
//
//	вспомогательный класс, определяющий стили рисования кривых.
//	пока задает только цвета. в перспективе следует задать возможность
//	изменения также штриховки. особенно важно при выводе векторного
//	файла для вставки в ч/б печатный документ
//

namespace XRAD_GUI
{



const int	GraphStyles::n_graph_colors = 12;
std::vector<QColor> GraphStyles::graph_colors;

const int	GraphStyles::n_graph_brushes = 14;
std::vector<BrushStyle> GraphStyles::graph_brushes;

const int	GraphStyles::n_graph_pens = 5;
std::vector<Qt::PenStyle> GraphStyles::graph_pens;

void	GraphStyles::InitGraphStyles()
	{
	do_once
		{
		graph_colors.push_back(Qt::green);
		graph_colors.push_back(Qt::blue);
		graph_colors.push_back(Qt::red);
		graph_colors.push_back(Qt::cyan);
		graph_colors.push_back(Qt::magenta);
		graph_colors.push_back(Qt::yellow);

		graph_colors.push_back(Qt::darkGreen);
		graph_colors.push_back(Qt::darkBlue);
		graph_colors.push_back(Qt::darkRed);
		graph_colors.push_back(Qt::darkCyan);
		graph_colors.push_back(Qt::darkMagenta);
		graph_colors.push_back(Qt::darkYellow);

		if(graph_colors.size() != n_graph_colors)
			{
			ForceDebugBreak();
			}
		graph_pens.push_back(Qt::SolidLine);
		graph_pens.push_back(Qt::DashLine);
		graph_pens.push_back(Qt::DotLine);
		graph_pens.push_back(Qt::DashDotLine);
		graph_pens.push_back(Qt::DashDotDotLine);

		if(graph_pens.size() != n_graph_pens)
			{
			ForceDebugBreak();
			}

		graph_brushes.push_back(SolidPattern);

		graph_brushes.push_back(Dense4Pattern);
		graph_brushes.push_back(Dense5Pattern);
		graph_brushes.push_back(Dense6Pattern);
		graph_brushes.push_back(Dense7Pattern);

		graph_brushes.push_back(Dense3Pattern);
		graph_brushes.push_back(Dense2Pattern);
		graph_brushes.push_back(Dense1Pattern);

		graph_brushes.push_back(CrossPattern);
		graph_brushes.push_back(DiagCrossPattern);
		graph_brushes.push_back(HorPattern);
		graph_brushes.push_back(VerPattern);
		graph_brushes.push_back(BDiagPattern);
		graph_brushes.push_back(FDiagPattern);

		if(graph_brushes.size() != n_graph_brushes)
			{
			ForceDebugBreak();
			}
		}
	}

QPen GraphStyles::GetGraphStyle(graph_line_style gss, double lw, int graph_no )
	{
	InitGraphStyles();

	qreal	line_width = lw;
	QColor	pen_color;
	PenStyle pen_style;
	BrushStyle	brush_style;

	switch(gss)
		{
		default:
		case	solid_color_lines:
			{
			pen_color = graph_colors[graph_no%n_graph_colors];
			pen_style = SolidLine;
			brush_style = SolidPattern;
			break;
			}
		case	dashed_black_lines:
			{
			pen_color = Qt::black;
			pen_style = graph_pens[graph_no%n_graph_pens];
			brush_style = SolidPattern;
			break;
			}

		case	dashed_color_lines:
			pen_color = graph_colors[graph_no%n_graph_colors];
			pen_style = graph_pens[graph_no%n_graph_pens];
			brush_style = SolidPattern;
			break;

		case textured_black_lines:
			pen_color = Qt::black;
			pen_style = SolidLine;
			brush_style = graph_brushes[graph_no%n_graph_brushes];
			break;

		case textured_color_lines:
			pen_color = graph_colors[graph_no%n_graph_colors];
			pen_style = SolidLine;
			brush_style = graph_brushes[graph_no%n_graph_brushes];
			break;
		};
	QBrush	brush(pen_color, brush_style);

	return QPen (brush,
		line_width,
		pen_style,
//		Qt::SquareCap,
//		Qt::FlatCap,
		Qt::RoundCap,
		Qt::RoundJoin);
//		Qt::MiterJoin);
//		Qt::BevelJoin);
	}



}//namespace XRAD_GUI
