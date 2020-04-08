#include "pre.h"
#include "GraphSet.h"
#include <XRADGUIApi.h>

XRAD_BEGIN

using namespace XRAD_GUI;

bool	GraphSet::DeleteGraph(size_t graph_no)
{
	return api_SetupGraphCurve(graph_container(), graph_no, DataArray<double>(), DataArray<double>(), L"");
}

bool GraphSet::ChangeLabels(const wstring &new_title, const wstring &new_y_label, const wstring &new_x_label)
{
	return api_SetGraphLabels(graph_container(), new_title, new_y_label, new_x_label);
}

bool GraphSet::ChangeLabels(const string &new_title, const string &new_y_label, const string &new_x_label)
{
	return ChangeLabels(convert_to_wstring(new_title), convert_to_wstring(new_y_label), convert_to_wstring(new_x_label));
}

GraphSet::GraphSet(const wstring &title, const wstring &y_label, const wstring &x_label)
{
	window.reset(new GraphWindowContainer(api_CreateGraph(title, y_label, x_label)));
}

GraphSet::GraphSet(const string &title, const string &y_label, const string &x_label)
{
	window.reset(new GraphWindowContainer(api_CreateGraph(convert_to_wstring(title), convert_to_wstring(y_label), convert_to_wstring(x_label))));
}


bool GraphSet::GetScale(GraphScale &sc)
{
	return api_GetGraphScale(graph_container(), sc);
}

bool	GraphSet::SetGraphStyle(graph_line_style style, double in_line_width)
{
	return api_SetGraphStyle(graph_container(), style, in_line_width);
}


bool	GraphSet::SetScale(const GraphScale &sc)
{
	return api_SetGraphScale(graph_container(), sc);
}

bool	GraphSet::ChangeGraphUniform(size_t graph_no, const DataArray<double> &data_y, double x0, double dx, const wstring &graph_name)
{
	RealFunctionF64	xx(data_y.size());
	RealFunctionF64::iterator it = xx.begin();
	for(size_t i = 0; i < xx.size(); ++i, ++it) *it = x0+i*dx;
	return api_SetupGraphCurve(graph_container(), graph_no, data_y, xx, graph_name);
}

bool	GraphSet::ChangeGraphParametric(size_t graph_no, const DataArray<double> &data_y, const DataArray<double> &data_x, const wstring &graph_name)
{
	if(data_x.size()!=data_y.size())
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GraphSet::ChangeGraph(), different arrays sizes (%d, %d)", data_x.size(), data_y.size()));
	}
	if(data_x.empty())
	{
		DeleteGraph(graph_no);
		return true;
	}

	return api_SetupGraphCurve(graph_container(), graph_no, data_y, data_x, graph_name);
}

bool	GraphSet::SavePicture(const wstring &filename_with_extension)
{
	return api_SaveGraphPicture(graph_container(), filename_with_extension);
}


XRAD_END
