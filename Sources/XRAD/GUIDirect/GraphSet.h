#ifndef __graph_set_h
#define __graph_set_h

#include <XRAD/GUICore.h>
#include <GraphScale.h>
#include <XRADBasic/MathFunctionTypes.h>
#include <DataDisplayer.h>
// #include <XRADGUI.h>

XRAD_BEGIN


class	GraphSet : public DataDisplayer
{
	GraphWindowContainer	&graph_container(){ return static_cast<GraphWindowContainer&>(*window); }
	const GraphWindowContainer	&graph_container() const { return static_cast<GraphWindowContainer&>(*window); }

public:

	GraphSet(const wstring &title, const wstring &y_label, const wstring &x_label);
	GraphSet(const string &title, const string &y_label, const string &x_label);

	~GraphSet() = default;


	bool	ChangeLabels(const wstring &new_title, const wstring &new_y_label, const wstring &new_x_label);
	bool	ChangeLabels(const string &new_title, const string &new_y_label, const string &new_x_label);

	template<class T>
	bool	AddGraphUniform(const DataArray<T> &data_y, double x0, double dx, const wstring &graph_name);
	template<class T>
	bool	AddGraphUniform(const DataArray<T> &data_y, double x0, double dx, const string &graph_name){ return AddGraphUniform(data_y, x0, dx, convert_to_wstring(graph_name)); }

	template<class T, class T2>
	bool	AddGraphParametric(const DataArray<T> &data_y, const DataArray<T2>& data_x, const wstring &graph_name);
	template<class T, class T2>
	bool	AddGraphParametric(const DataArray<T> &data_y, const DataArray<T2>& data_x, const string &graph_name){ return AddGraphParametric(data_y, data_x, convert_to_wstring(graph_name)); }


	template<class T>
	bool	ChangeGraphUniform(size_t graph_no, const DataArray<T> &data_y, double x0, double dx, const wstring &graph_name);
	template<class T>
	bool	ChangeGraphUniform(size_t graph_no, const DataArray<T> &data_y, double x0, double dx, const string &graph_name){ return ChangeGraphUniform(graph_no, data_y, x0, dx, convert_to_wstring(graph_name)); }
	bool	ChangeGraphUniform(size_t graph_no, const DataArray<double> &data_y, double x0, double dx, const wstring &graph_name);

	template<class T, class T2>
	bool	ChangeGraphParametric(size_t graph_no, const DataArray<T> &data_y, const DataArray<T2> &data_x, const wstring &graph_name);
	template<class T, class T2>
	bool	ChangeGraphParametric(size_t graph_no, const DataArray<T> &data_y, const DataArray<T2> &data_x, const string &graph_name){ return ChangeGraphParametric(graph_no, data_y, data_x, convert_to_wstring(graph_name)); }

	bool	ChangeGraphParametric(size_t graph_no, const DataArray<double> &data_y, const DataArray<double> &data_x, const wstring &graph_name);

	bool	DeleteGraph(size_t graph_no);

	bool	GetScale(GraphScale &);
	bool	SetScale(const GraphScale&);
	bool	SetGraphStyle(graph_line_style style_set, double in_line_width);

	bool	SavePicture(const wstring &filename_with_extension);
};

template<class T>
bool	GraphSet::AddGraphUniform(const DataArray<T> &data_y, double x0, double dx, const wstring &graph_name)
{
	return ChangeGraphUniform(graph_set_new_graph(), data_y, x0, dx, graph_name);
}

template<class T, class T2>
bool	GraphSet::AddGraphParametric(const DataArray<T> &data_y, const DataArray<T2> &data_x, const wstring &graph_name)
{
	return ChangeGraphParametric(graph_set_new_graph(), data_y, data_x, graph_name);
}



template<class T>
bool	GraphSet::ChangeGraphUniform(size_t graph_no, const DataArray<T> &data_y, double x0, double dx, const wstring &graph_name)
{
	RealFunctionF64	yy(data_y);
	return ChangeGraphUniform(graph_no, yy, x0, dx, graph_name);
}

template<class T, class T2>
bool	GraphSet::ChangeGraphParametric(size_t graph_no, const DataArray<T> &data_y, const DataArray<T2> &data_x, const wstring &graph_name)
{
	RealFunctionF64	yy(data_y);
	RealFunctionF64	xx(data_x);

	return ChangeGraphParametric(graph_no, yy, xx, graph_name);
}




XRAD_END

#endif // __graph_set_h
