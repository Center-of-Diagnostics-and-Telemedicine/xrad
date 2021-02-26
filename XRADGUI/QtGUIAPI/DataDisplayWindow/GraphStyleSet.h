#ifndef XRAD__File_GraphStyleSet_h
#define XRAD__File_GraphStyleSet_h
//--------------------------------------------------------------
//
//	created:	2014/03/28
//	created:	28.3.2014   13:21
//	author:		KNS
//
//	purpose:	набор стилей рисования кривых графиков.
//				цветной для анализа на экране, штихпунктирный для печати.
//				возможно, будет еще что-то другое
//
//--------------------------------------------------------------

#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <XRADGUI/Sources/Core/GUICore.h>
#include <vector>

namespace XRAD_GUI
{

XRAD_USING
using namespace Qt;
using namespace std;

class	GraphStyles
{
		const static	int n_graph_pens;
		const static	int	n_graph_colors;
		const static	int n_graph_brushes;

		static	vector<QColor> graph_colors;
		static	vector<PenStyle> graph_pens;
		static	vector<BrushStyle> graph_brushes;

		static void	InitGraphStyles();

	public:
		static QPen	GetGraphStyle(graph_line_style gss, double lw, int graph_no);
};

}//namespace XRAD_GUI

#endif //XRAD__File_GraphStyleSet_h
