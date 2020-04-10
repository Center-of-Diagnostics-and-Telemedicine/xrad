#include "pre.h"
#include "GraphWindow.h"

#include "ThreadGUI.h"
#include "GUIController.h"
#include "FileSaveUtils.h"
#include <XRADGUI/Sources/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>
#include <XRADSystem/TextFile.h>

namespace XRAD_GUI
{

XRAD_USING



//--------------------------------------------------------------
//
//	несколько вспомогательных процедур, нужных только здесь
//

namespace
{

int	listbox_index_from_line_style(graph_line_style gs)
{
	return gs;
}

graph_line_style line_style_from_listbox_index(int n)
{
	switch(n)
	{
		case 0:
		default:
			return solid_color_lines;
			break;
		case 1:
			return dashed_black_lines;
			break;
		case 2:
			return dashed_color_lines;
			break;
		case 3:
			return textured_black_lines;
			break;
		case 4:
			return textured_color_lines;
			break;
	};
}

double	MFVectorMinValue(const std::vector<xrad::RealFunctionF64> &f)
{
	double	result(0);
	bool	found = false;
	size_t i = 0;

	while(i<f.size())
	{
		if(f[i].empty()) ++i;
		else
		{
			result = MinValue(f[i]);
			found = true;
			break;
		}
	}

	for(; i < f.size(); ++i)
	{
		if(!f[i].empty())
		{
			result = min(result, MinValue(f[i]));
		}
	}
	if(found) return result;
	else throw invalid_argument("Empty graph set");
}

double	MFVectorMaxValue(const std::vector<xrad::RealFunctionF64> &f)
{
	double	result(0);
	bool	found = false;
	size_t i = 0;

	while(i<f.size())
	{
		if(f[i].empty()) ++i;
		else
		{
			result = MaxValue(f[i]);
			found = true;
			break;
		}
	}

	for(; i < f.size(); ++i)
	{
		if(!f[i].empty())
		{
			result = max(result, MaxValue(f[i]));
		}
	}
	if(found) return result;
	else throw invalid_argument("Empty graph set");

// 	double	result = MaxValue(f[0]);
// 	for(size_t i = 1; i < f.size(); ++i)
// 	{
// 		result = max(result, MaxValue(f[i]));
// 	}
// 	return result;
}

class	not_found{};

int	FindIndex(const xrad::RealFunctionF64 &data_x_local, const xrad::RealFunctionF64 &data_y_local, double x, double y)
{
	std::vector<int> found;

	if(data_x_local.empty() || data_y_local.empty()) throw not_found();

	xrad::RealFunctionF64::const_iterator it = data_x_local.begin();
	xrad::RealFunctionF64::const_iterator it1 = ++data_x_local.begin();

	for(size_t i = 0; i < data_x_local.size()-1; ++i, ++it, ++it1)
	{
		double mid = 0.5 * (*it + *it1);
		if (*it <= *it1)
		{
			if (in_range(x, *it, mid))
			{
				if (found.empty() || found.back() != i)
					found.push_back(int(i));
			}
			else if (in_range(x, mid, *it1))
				found.push_back(int(i+1));
		}
		else
		{
			if (in_range(x, mid, *it))
			{
				if (found.empty() || found.back() != i)
					found.push_back(int(i));
			}
			else if (in_range(x, *it1, mid))
				found.push_back(int(i+1));
		}
	}

	if(found.size()==1) return found[0];//монотонное возрастание или убывание x, самый простой случай
	else if(!found.size())// x за границами значений, возвращаем -1, далее по этому значению данные игнорируются
	{
		if(x <= MinValue(data_x_local)) return 0;
		else if(x>=MaxValue(data_x_local)) return int(data_x_local.size())-1;
		else throw not_found();
		//return -1;
		//if(x<=MinValue(data_x)) return 0;
		//else return data_x.size()-1;
	}
	else
	{
		int	index = found[0];
		double diff = fabs(data_y_local[index]-y);
		vector<int>::iterator	it = found.begin()+1;
		for(size_t i = 1; i < found.size(); ++i, ++it)
		{
			double	newdiff = fabs(data_y_local[*it]-y);
			if(newdiff<diff) diff = newdiff, index = *it;
		}
		return index;
	}
}

}//namespace

//
//--------------------------------------------------------------



GraphWindow::GraphWindow(const QString &title, const QString &in_y_label, const QString &in_x_label, GUIController &gc)
	:
	DataDisplayWindow(gc),
	VisualizationPlot(plot),
	graph_value_drawer(NULL),
	n_curves(0),
	legend(NULL),
	line_style(solid_color_lines),
	line_width(1.5),
	axes_drawer(NULL),
	n_incomplete_curves(0)
{
	try
	{
		setupUi(this);

		// задаем положение окна
		auto corner = GetCornerPosition();
		setGeometry(QRect(QPoint(corner.x(), corner.y()), WindowGeometry::default_graph_window_size()));

		InitVisualizationPlot();

		// делаем белый фон
		SetBackground(Qt::white, Qt::SolidPattern);
//		uniform_y_scale = cbIgnoreScale->checkState();

		// устанавливаем наименование осей координат
		SetGraphLabels(title, in_y_label, in_x_label);

		// инициализируем строку отображения текущих значений
		graph_values_label->setText("");

		sbLineWidth->setValue(line_width);

		cbLineStyle->addItem("Colour solid lines");
		cbLineStyle->addItem("Black dashed lines");
		cbLineStyle->addItem("Colour dashed lines");

		cbLineStyle->addItem("Black textured lines");
		cbLineStyle->addItem("Colour textured lines");

		cbLineStyle->setCurrentIndex(0);

		cbTransform->addItem("None");
		cbTransform->addItem("Uniform scale");
		cbTransform->addItem("Log. compress");
		cbTransform->addItem("1st derivative");
		cbTransform->addItem("2nd derivative");
		cbTransform->addItem("Integral");
		cbTransform->setCurrentIndex(transform=0);

		// устанавливаем обработчик событий
		plot->installEventFilter(this);

		QObject::connect(cbShowSymbol, SIGNAL(toggled(bool)), this, SLOT(slotSetSymbol(bool)));
//		QObject::connect(cbIgnoreScale, SIGNAL(toggled(bool)), this, SLOT(slotSetTransform(bool)));
		QObject::connect(cb_grid, SIGNAL(toggled(bool)), this, SLOT(SetGridVisibility(bool)));

		QObject::connect(btSavePicture, SIGNAL(clicked()), this, SLOT(slotSavePicture()));
		QObject::connect(btSaveRawData, SIGNAL(clicked()), this, SLOT(slotSaveRawData()));

		QObject::connect(cbTransform, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetTransform(int)));
		QObject::connect(cbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGraphStyle()));
		QObject::connect(sbLineWidth, SIGNAL(valueChanged(double)), this, SLOT(slotGraphStyle()));

		// приводим вид графиков в соответствие элементам управления
		slotGraphStyle();

		// на Esc вызываем закрытие формы (по умолчанию Escape на QDialog вызывает reject() и только скрывает ее)
		// QObject::connect(this, SIGNAL(rejected()),SLOT(close()) );

		QObject::connect(main_zoom, SIGNAL(ScaleChanged()), this, SLOT(UpdateAxesDrawer()), Qt::QueuedConnection);

		SetAxesVisibility(true);

		axes_drawer = new QwtPlotShapeItem;
		axes_drawer->setPen(Qt::black, 0.1);
		axes_drawer->setBrush(Qt::black);
		axes_drawer->attach(plot);

		//добавляем объект в массив диалогов
		gui_controller.AddWidget(this);

		setAttribute(Qt::WA_DeleteOnClose);

		installEventFilter(this);
	}
	catch(...)
	{
	}
}

GraphWindow::~GraphWindow()
{
	//удаляем объект из массива диалогов
	gui_controller.RemoveWidget(this);

	SetLegend(false);
	for(auto curve_ptr: curves) DestroyObject(curve_ptr);

	DestroyObject(axes_drawer);
}

void GraphWindow::SetLineStyle(graph_line_style gss, double in_line_width)
{
	if (in_line_width > 0)
	{
		line_width = in_line_width;
		sbLineWidth->setValue(line_width);
	}
	int	needed_listbox_position = listbox_index_from_line_style(gss);
	if(needed_listbox_position != cbLineStyle->currentIndex())
	{
		cbLineStyle->setCurrentIndex(needed_listbox_position);
		return;//в следующий раз вернется сюда по событию и тогда уже все поменяет
	}
	line_style = gss;
	for(int i = 0; i < n_curves; ++i)
	{
		curves[i]->setPen(GraphStyles::GetGraphStyle(gss, line_width, i));
	}
	UpdateAxesDrawer();// меняется вид осей, нужно учесть. перерисовка плота внутри
	//	plot->replot();
}

void	GraphWindow::SetWindowTitle(const QString &title)
{
	setWindowTitle(title);
}

void	GraphWindow::SetGraphLabels(const QString &title, const QString &in_y_label, const QString &in_x_label)
{
	// задаем заголовок
	//plot->setTitle(QDialog::tr(title.c_str()));

	x_label = in_x_label;
	y_label = in_y_label;

	SetAxesLabels(yLabelTransformed(y_label), x_label);

	setWindowTitle(title);
}

void	GraphWindow::SetLegend(bool legend_on)
{
	//	создаем/удаляем легенду-расшифровку для кривой.
	if(legend_on && !legend)
	{
		legend = new QwtLegend;
		legend->setDefaultItemMode(QwtLegendData::ReadOnly);

		plot->insertLegend(legend, QwtPlot::TopLegend);
	}
	else if(!legend_on && legend)
	{
		DestroyObject(legend);
	}
	plot->updateLegend();
}

void GraphWindow::SetScale(const range2_F64 &gs)
{
	main_zoom->SetBounds(gs, true);
	UpdateScale();
}

void GraphWindow::ResetScale()
{
	main_zoom->RestoreBounds();
	UpdateScale();
}

range2_F64 GraphWindow::GetScale() const
{
	return main_zoom->GetScale();
}

void	DrawArrow(QPainterPath &axes_path, const range2_F64 &points, double w, double h, double reference_lw)
{
	//  рисует красивую стрелку на конце оси. почему-то штатными средствами Qt такую, как надо, сделать не удалось
	point2_F64	delta = points.delta();

	double	angle = atan2(delta.y(), delta.x());
	// отступы по ширине
	double	wx = reference_lw*w*sin(angle);
	double	wy = reference_lw*w*cos(angle);

	double	p = reference_lw>1 ? -0.5 : -0.875;
	double	arrow_len_factor = 32.*pow(reference_lw, p);
	double	arrow_wid_factor = 8.*pow(reference_lw, p);

	point2_F64	arrow_vec = point2_F64(-sin(angle), cos(angle)) * arrow_len_factor * h * reference_lw;
	point2_F64	haft_end = points.p2() - arrow_vec;// конец "черенка" перед стрелкой

	// ставим точку на ось в начало линии
	axes_path.moveTo(points.x1(), points.y1());

	// рисуем начало черенка
	axes_path.lineTo(points.x1()+wx, points.y1()+wy);
	axes_path.lineTo(haft_end.x() + wx, haft_end.y()+wy);

	// рисуем стрелку
	axes_path.lineTo(haft_end.x() + wx*arrow_wid_factor, haft_end.y()+wy*arrow_wid_factor);
	axes_path.lineTo(points.x2(), points.y2());
	axes_path.lineTo(haft_end.x() -wx*arrow_wid_factor, haft_end.y()-wy*arrow_wid_factor);

	// возврат к черенку
	axes_path.lineTo(haft_end.x() -wx, haft_end.y()-wy);
	axes_path.lineTo(points.x1()-wx, points.y1()-wy);

	// замыкаем контур
	axes_path.closeSubpath();
}

void	GraphWindow::UpdateAxesDrawer()
{
	if(!main_zoom || !axes_drawer || !n_curves) return;

	int	one_pix = 1;
	point2_F64 p0 = main_zoom->InvertTransformCoordinates(point2_I32(0, 0));
	point2_F64 p1 = main_zoom->InvertTransformCoordinates(point2_I32(one_pix, one_pix));// - p0;
	point2_F64 p2 = main_zoom->InvertTransformCoordinates(point2_I32(-one_pix, -one_pix));// - p0;

	range2_F64	gs = GetScale();

	QPainterPath	axes_path;

	double		w = (p1.x() -p2.x())/4;
	double		h = (p1.y()-p2.y())/4;

	point2_F64	start_pointX(gs.y1(), 0);
	point2_F64	end_pointX(gs.y2(), 0);

	point2_F64	start_pointY(0, gs.x1());
	point2_F64	end_pointY(0, gs.x2());

	DrawArrow(axes_path, range2_F64(start_pointX, end_pointX), w, h, line_width);
	DrawArrow(axes_path, range2_F64(start_pointY, end_pointY), h, w, line_width);

	axes_drawer->setShape(axes_path);
	plot->replot();
}

void GraphWindow::SetGridVisibility(bool visible)
{
	grid->setVisible(visible);
	plot->replot();
}

void	GraphWindow::UpdateScale()
{
	range2_F64	auto_scale;

	try
	{
		auto_scale.x1() = MFVectorMinValue(data_x);
		auto_scale.x2() = MFVectorMaxValue(data_x);
		auto_scale.y2() = MFVectorMaxValue(data_y_transformed);
		auto_scale.y1() = MFVectorMinValue(data_y_transformed);
	}
	catch(...)
	{
		auto_scale = range2_F64(0);
	}

	if(auto_scale.y2() == auto_scale.y1())
	{
		++auto_scale.y2();
		--auto_scale.y1();
	}
	if(auto_scale.x2() == auto_scale.x1())
	{
		++auto_scale.x2();
		--auto_scale.x1();
	}

	main_zoom->SetAutoBounds(auto_scale);

	if(!main_zoom->custom_scale)
	{
		// если включено автомасштабирование, задаем новый масштаб
		main_zoom->SetBounds(auto_scale, false);
	}
	else
	{
		// просто перерисовываем картинку
		plot->replot();
	}
	UpdateAxesDrawer();
}

//--------------------------------------------------------------
//
//	изменяет график с номером curve_no, подставляя ему новые данные
//
//	если in_curve_no = -1, добавляет новый график в конец списка
//	если in_curve_no соответствует одной из кривых, изменяет ее в соответствии
//	с переданными массивами.
//	если при указатель на какой-либо массив нулевой, использует ранее определенные данные.
//	если при корректном номере кривой размер массива равен нулю,
//	удаляет соответствующую кривую
//
//	сейчас довольно запутанное ветвление, сделать яснее!
//

RealFunctionF64	SafeValuesCurve(const RealFunctionF64 &in_values)
{
	RealFunctionF64	result(in_values);
	RealFunctionF64::iterator it = result.begin();
	RealFunctionF64::iterator ie = result.end();
	for(; it<ie; ++it)
	{
		if(!(fabs(*it)<AbsoluteScaleLimitation()))
		{
			if(*it>0)
			{
				*it = AbsoluteScaleLimitation();
			}
			else if(*it<0)
			{
				*it = -AbsoluteScaleLimitation();
			}
			// nan оставляем
		}
	}

	return result;
}

void	GraphWindow::SetupCurve(int curve_no,
		const DataArray<double> &in_data_y,
		const DataArray<double> &in_data_x,
		const QString &graph_name)
{
	//	корректный номер кривой, оба массива точек пустые или разные
	//	выполняем удаление соответствующей кривой
	if(in_range(curve_no, 0, n_curves-1) && (in_data_y.empty() || in_data_y.size() != in_data_x.size()))
	{
		EraseCurve(curve_no);
		UpdateScale();
		return;
	}

	//	некорректный номер кривой
	//	создаем новую кривую, если для этого достаточно данных
	if(!in_range(curve_no, 0, n_curves-1))
	{

		// если номер отрицательный, добавляем кривую в конец списка
		if(curve_no < 0)
		{
			curve_no = AddCurve();
		}
		// Для положительного номера добавляем пустые кривые, и создаем ее под нужным номером
		else while(curve_no >= n_curves)
		{
			AddCurve();
		}
	}

	++n_incomplete_curves;

	data_x[curve_no].MakeCopy(in_data_x);
	data_y[curve_no].MakeCopy(in_data_y);
	data_y_transformed[curve_no].realloc(in_data_y.size());
	UpdateTransformedCurve(curve_no);


	graph_labels[curve_no] = graph_name;
	curves[curve_no]->setTitle(graph_name);

	//передаем кривой подготовленные данные
	SetCurveValues(curve_no);

	UpdateScale();
	--n_incomplete_curves;
}

void	GraphWindow::UpdateTransformedCurve(size_t curve_no)
{
	data_y_transformed[curve_no].CopyData(data_y[curve_no]);

	auto	deriv = [](auto &y, auto &x)
	{
		for(int i = int(y.size())-1; i > 0; --i)
		{
			double divisor = x[i] - x[i-1];
			y[i] -= y[i-1];
			if(fabs(divisor)) y[i] /= divisor;
			else y[i] = nan("");
		}
		if(y.size()>1) y[0] = y[1];
	};

	auto	integral = [](auto &y, auto &x)
	{
		double	y_previous = y[0];
		for(size_t i = 1; i < y.size(); ++i)
		{
			double result = y[i-1] + 0.5*(y_previous + y[i])*(x[i] - x[i-1]);
			y_previous = y[i];
			y[i] = result;
		}
	};

	auto	log_transform = [](const auto &x)
	{
		return range(log(fabs(x)), -1000, infinity());
	};

	switch(transform)
	{
		default:
		case e_transform_none:
			break;

		case e_uniform_scale:
			{
				double cmin = MinValue(data_y[curve_no]);
				double cmax = MaxValue(data_y[curve_no]);
				data_y_transformed[curve_no] -= cmin;
				if(cmin!=cmax) data_y_transformed[curve_no] *= (100/(cmax-cmin));
			}
			break;

		case e_log_compress:
			std::transform(data_y[curve_no].begin(), data_y[curve_no].end(), data_y_transformed[curve_no].begin(),log_transform);
			break;
		case e_2nd_derivative:
			deriv(data_y_transformed[curve_no], data_x[curve_no]);

		case e_1st_derivative:
			deriv(data_y_transformed[curve_no], data_x[curve_no]);
			break;

		case e_integral:
			integral(data_y_transformed[curve_no], data_x[curve_no]);
			break;

// 		e_integral
	}
}

void	GraphWindow::UpdateTransformedCurves()
{
	for(int i = 0; i < n_curves; ++i) UpdateTransformedCurve(i);
}

void	GraphWindow::SetCurveValues(size_t curve_no)
{
	RealFunctionF64	display_data_x = SafeValuesCurve(data_x[curve_no]);
	RealFunctionF64	display_data_y = SafeValuesCurve(data_y_transformed[curve_no]);

	curves[curve_no]->setSamples(&display_data_x[0], &display_data_y[0], int(display_data_x.size()));
	curves[curve_no]->setTitle(yLabelTransformed(graph_labels[curve_no]));
}

void	GraphWindow::EraseCurve(int curve_no)
{
	if(in_range(curve_no, 0, n_curves-1))
	{
		--n_curves;
		data_x.erase(data_x.begin()+curve_no);
		data_y.erase(data_y.begin()+curve_no);
		data_y_transformed.erase(data_y_transformed.begin()+curve_no);
		graph_labels.erase(graph_labels.begin()+curve_no);
		DestroyObject(curves[curve_no]);
		curves.erase(curves.begin()+curve_no);

		//	если кривая одна, то легенда ни к чему, удаляем ее
		if(n_curves <= 1) SetLegend(false);

		UpdateScale();
	}
}

int	GraphWindow::AddCurve()
{
	// добавляем новую кривую в конец списка
	int	curve_no = n_curves;
	data_x.push_back(RealFunctionF64());
	data_y.push_back(RealFunctionF64());
	data_y_transformed.push_back(RealFunctionF64());
	graph_labels.push_back(QString());
	curves.push_back(new QwtPlotCurve());

	//задаем атрибуты кривой, помещаем ее на график
	curves[curve_no]->setRenderHint(QwtPlotItem::RenderAntialiased, true); //сглаживание по прорисовке
	curves[curve_no]->setPen(GraphStyles::GetGraphStyle(line_style, line_width, curve_no));
	curves[curve_no]->attach(plot);

	// атрибуты отображения кривой на легенде
	curves[curve_no]->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
	curves[curve_no]->setLegendAttribute(QwtPlotCurve::LegendShowBrush, true);
	curves[curve_no]->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
	curves[curve_no]->setLegendIconSize(QSize(32, 16));
	// LegendNoAttribute 	QwtPlotCurve tries to find a color representing the curve and paints a rectangle with it.
	// LegendShowLine 	If the style() is not QwtPlotCurve::NoCurve a line is painted with the curve pen().
	// LegendShowSymbol If the curve has a valid symbol it is painted.
	// LegendShowBrush If the curve has a brush a rectangle filled with the curve brush() is painted.

	++n_curves;

	if(n_curves > 1) SetLegend(true);

	// возвращаем номер добавленной кривой
	return curve_no;
}

void GraphWindow::closeEvent(QCloseEvent *event)
{
	QDialog::closeEvent(event);
}

QString	GraphWindow::yLabelTransformed(const QString &in_label)
{
	switch(transform)
	{
		default:
		case e_transform_none:
			return in_label;

		case e_uniform_scale:
			return "% [" + in_label + "]";

		case e_log_compress:
			return "log [" + in_label + "]";

		case e_1st_derivative:
			return "d/dx [" + in_label + "]";

		case e_2nd_derivative:
			return wstring_to_qstring(L"d²/dx² [") + in_label + "]";

		case e_integral:
			return wstring_to_qstring(L"∫dx [") + in_label + "]";
	}
}

void GraphWindow::slotSetTransform(int in_transform)
{
	transform = in_transform;
	for(int curve_no = 0; curve_no < n_curves; ++curve_no)
	{
		UpdateTransformedCurve(curve_no);
		SetCurveValues(curve_no);
	}

	ResetScale();
	SetAxesLabels(yLabelTransformed(y_label), x_label);
	UpdateScale();
}

void GraphWindow::slotSetSymbol(bool checked)
{
	for(int i = 0; i < n_curves; ++i)
	{
		if(checked)
		{
			QwtSymbol *symbol=new QwtSymbol;
			symbol->setStyle(QwtSymbol::Ellipse);
			symbol->setPen(QColor(Qt::black));
			symbol->setSize(2);
			curves[i]->setSymbol(symbol);
		}
		else
		{
			curves[i]->setSymbol(NULL);
		}
	}
	//перестраиваем график
	plot->replot();
}

void GraphWindow::slotGraphStyle()
{
	line_width = sbLineWidth->value();
	graph_line_style gss = line_style_from_listbox_index(cbLineStyle->currentIndex());

	SetLineStyle(gss, line_width);
}

// возвращает тип файла для функции рендера

void GraphWindow::slotSaveRawData()
{
	const char *prompt = "Save raw data";
	const char *type = "Text file (*.txt)";
	QString qfile_name = GetSaveFileName(QFileDialog::tr(prompt), QFileDialog::tr(type));
	if(qfile_name.isEmpty()) return;
	wstring	file_name = qstring_to_wstring(qfile_name);//.toStdWString();
	try
	{
		text_file_writer	file_container(file_name, text_encoding::utf8);

		ustring	ubuffer;
		ubuffer = wstring_to_ustring(qstring_to_wstring(windowTitle()));
		file_container.printf_("graph title=\"%s\"\n", ubuffer.c_str());
		ubuffer = wstring_to_ustring(qstring_to_wstring(x_label));
		file_container.printf_("x_label=\"%s\"\n", ubuffer.c_str());
		ubuffer = wstring_to_ustring(qstring_to_wstring(yLabelTransformed(y_label)));
		file_container.printf_("y_label=\"%s\"\nn_graphs = %d\n", ubuffer.c_str(), n_curves);

		for(int i = 0; i < n_curves; ++i)
		{
			ubuffer = convert_to_ustring(qstring_to_wstring(yLabelTransformed(graph_labels[i])));
			file_container.printf_("[%s]\nn_samples = %d\n", ubuffer.c_str(), data_x[i].size());
			for(size_t j = 0; j < data_x[i].size(); ++j)
			{
				file_container.printf_("%g", data_x[i][j]);
				if(j+1 < data_x[i].size())file_container.printf_("\t");
				else file_container.printf_("\n");

			}
			for(size_t j = 0; j < data_y_transformed[i].size(); ++j)
			{
				file_container.printf_("%g", data_y_transformed[i][j]);
				if(j+1 < data_y_transformed[i].size()) file_container.printf_("\t");
				else file_container.printf_("\n");
			}
		}
	}
	catch(...)
	{
	}
}

bool SupportedExportFormat(const QString &format)
{
	return IsVectorImageFormat(format) || IsRasterImageFormat(format);
}

void GraphWindow::SavePicture(QString filename)
{
	if(filename.isEmpty()) return;

	QwtPlotRenderer	renderer;
	renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);
	//	renderer.setDiscardFlag(QwtPlotRenderer::DiscardLegend, true);
	renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame, true);

	QString	format = FormatByFileExtension(filename);
	bool	raster_format = IsRasterImageFormat(format);
	bool	vector_format = IsVectorImageFormat(format);
	if(raster_format||vector_format)
	{
		// приходится делать равностороннюю геометрию, чтобы сохранить толщину осей!
		const QRect	store_geometry = geometry();
		//		QRect	cg = plot->canvas()->geometry();
		QRect	cg = plot->geometry();
		// для этого меняем размер всего окна таким образом, чтобы размер плота был new_size x new_size

		int	new_size = max(cg.width(), cg.height());
		QRect	new_geometry(geometry().x(), geometry().y(),
							 new_size + geometry().width()-cg.width(),
							 new_size + geometry().height()-cg.height());
		setGeometry(new_geometry);
		UpdateAxesDrawer();

		if(vector_format)
			renderer.renderDocument(plot, filename, format, QSizeF(100, 100), 100);
		else
			// 			renderer.renderDocument(plot, file_name, format, QSizeF(300,300), 100);
			renderer.renderDocument(plot, filename, format, QSizeF(100, 100), 300);
		// для растрового формата увеличиваем разрешение.
		// для векторных этого не делаем, т.к. нарушается толщина
		// кривой на легенде

		setGeometry(store_geometry);
		UpdateAxesDrawer();
	}

}


void GraphWindow::slotSavePicture()
{
	const char *prompt = "Save picture";
	const char *type = "png (*.png);;pdf (*.pdf);;svg (*.svg);;jpeg (*.jpg);;bmp (*.bmp)";
	QString filename = GetSaveFileName(QFileDialog::tr(prompt), QFileDialog::tr(type));

	SavePicture(filename);
}

// Обработчик всех событий
bool GraphWindow::eventFilter(QObject *target, QEvent *event)
{

	transform = cbTransform->currentIndex(); //#то ли это?

	// если событие произошло для графика, то

	if(target == plot)
	{
		// если произошло одно из событий от мыши, то
		switch(event->type())
		{
			case QEvent::MouseButtonPress:
			case QEvent::MouseMove:
			case QEvent::MouseButtonRelease:
				procMouseEvent(event);
				break;

			case QEvent::Resize:
			case QEvent::Show:
				// 			case QEvent::UpdateRequest:
				// 			case QEvent::Paint:
				UpdateAxesDrawer();
			default:
				break;
		};
	}
	// передаем управление стандартному обработчику событий
	return QObject::eventFilter(target, event);
}

void GraphWindow::keyPressEvent(QKeyEvent *event)
{
	if(event->type()==QEvent::KeyPress)
	{
		switch(event->key())
		{
			case Qt::Key_S:
				if(event->modifiers() == Qt::ControlModifier) slotSavePicture();
				break;

			case Qt::Key_Escape:
				emit signal_esc();
				break;
		};
	}
	return QWidget::keyPressEvent(event);
}



// Обработчик обычных событий от мыши
void GraphWindow::procMouseEvent(QEvent *event)
{
	// создаем указатель на событие от мыши
	QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);

	// в зависимости от типа события вызываем соответствующий обработчик
	switch(event->type())
	{
		// нажата кнопка мыши
		case QEvent::MouseButtonPress:
			StartCurrentValueDraw(mEvent);
			break;
			// перемещение мыши
		case QEvent::MouseMove:
			DrawCurrentValue(mEvent);
			break;
			// отпущена кнопка мыши
		case QEvent::MouseButtonRelease:
			EndCurrentValueDraw(mEvent);
			break;
			// для прочих событий ничего не делаем
		default:;
	}
}

// включаем режим рисования текущего значения
void GraphWindow::StartCurrentValueDraw(QMouseEvent *mEvent)
{
	if((mEvent->modifiers() == Qt::ShiftModifier)) return;

	// определяем текущее положение курсора (относительно канвы графика)
	if(main_zoom->EventBelongsToCanvas(mEvent))
		if(mEvent->button() == Qt::LeftButton)
		{
			// запоминаем текущий курсор и устанавливаем курсор Cross
			//main_zoom->SetCursor(Qt::CrossCursor);
			canvas->setCursor(Qt::CrossCursor);
			// создаем виджет, на котором будут нарисованы перпендикулярные линии, пересеающиеся с графиком
			graph_value_drawer = new GraphValueDrawer(plot->parentWidget(), n_curves, line_style, line_width);
			int gx0 = plot->geometry().x() + canvas->geometry().x();
			int gy0 = plot->geometry().y() + canvas->geometry().y();


			graph_value_drawer->setGeometry(QRect(gx0, gy0, canvas->geometry().width(), canvas->geometry().height()));
			graph_value_drawer->setVisible(true);

			DrawCurrentValue(mEvent); //отображаем прямые
		}
}

// рисование текущего значения
void GraphWindow::DrawCurrentValue(QMouseEvent *mEvent)
{
	if(mEvent->modifiers() == Qt::ShiftModifier)
		return;
	bool print_x_data = mEvent->modifiers() == Qt::ControlModifier;

	try
	{
		// положение курсора при перемещении мыши (в пикселах относительно канвы графика)
		point2_I32	cursor_position = main_zoom->RelativeCursorPosition(mEvent);
		point2_F64	cursor_values = main_zoom->InvertTransformCoordinates(cursor_position);

		wstring	current_value_string = ssprintf(L"cursor [%ls=%g; %ls=%g] ",
												x_label.isEmpty()?
													L"x":
													qstring_to_wstring(x_label).c_str(),

												cursor_values.x(),
												y_label.isEmpty()?
													qstring_to_wstring(yLabelTransformed("y")).c_str():
													qstring_to_wstring(yLabelTransformed(y_label)).c_str(),
												cursor_values.y());

		if(graph_value_drawer)
		{
			// крайние значения для обеих прямых
			graph_value_drawer->bounds = range2_I32(0, 0, canvas->geometry().height(), canvas->geometry().width());
			graph_value_drawer->x_current_pix = cursor_position.x();
		}


		for(int graph_no = 0, lines_added = 0; graph_no < n_curves; ++graph_no)
		{
			// 			if(in_range(index_x, 0u, data_x[graph_no].size()-1u))
			if(data_x.size()) try
			{
				int index_x = FindIndex(data_x[graph_no], data_y_transformed[graph_no], cursor_values.x(), cursor_values.y());
				point2_F64 data(data_y_transformed[graph_no][index_x], data_x[graph_no][index_x]);
				point2_I32 line = main_zoom->TransformCoordinates(data);

				if(graph_value_drawer)
				{
					//if(in_range(line.y(), 0, canvas->geometry().height()) && in_range(cursor_position.x(), 0, canvas->geometry().width()))
					//{
					//	graph_value_drawer->y_current_pix[graph_no] = line.y();
					//}
					graph_value_drawer->y_current_pix[graph_no] = range(line.y(), 0, canvas->geometry().height()-1);
				}
				auto label = qstring_to_wstring(yLabelTransformed(graph_labels[graph_no]));
				if (print_x_data)
				{
					label = ssprintf(L"%ls(%g)", label.c_str(), data.x());
				}
				if(!lines_added)
				{
					if(n_curves == 1)
					{
						current_value_string += ssprintf(L"data: [%ls=%g", label.c_str(), data.y());
					}
					else
					{
						current_value_string += ssprintf(L"graphs: [%ls=%g", label.c_str(), data.y());
					}
				}
				else
				{
					current_value_string += ssprintf(L", %ls=%g", label.c_str(), data.y());
				}
				++lines_added;
			}
			//			else
			catch(not_found &)
			{
				if(graph_value_drawer)
				{
					graph_value_drawer->y_current_pix[graph_no] = -1;
				}
			}
		}
		current_value_string += L"]";
		if(graph_value_drawer)
		{
			// перерисовываем виджет
			graph_value_drawer->repaint();
		}

		graph_values_label->setText(wstring_to_qstring(current_value_string));
	}
	catch(...)
	{
	}
}

// отключаем режим рисования текущего значения
void GraphWindow::EndCurrentValueDraw(QMouseEvent *mEvent)
{
	// если отпущена левая кнопка мыши, то
	if(mEvent->button() == Qt::LeftButton)
	{
		// восстанавливаем курсор
		canvas->setCursor(Qt::ArrowCursor);
		// 		main_zoom->ResetCursor();
		// удаляем виджет
		DestroyObject(graph_value_drawer);
	}
	graph_values_label->setText("");
}



//--------------------------------------------------------------

// class PaintWidget

//--------------------------------------------------------------

//GraphValueDrawer(QWidget *parent, int in_n_graphs, graph_line_style in_gss, double in_line_weight);

GraphValueDrawer::GraphValueDrawer(QWidget *parent, int in_n_graphs, graph_line_style in_gss, double in_line_width)
	: QWidget(parent), n_graphs(in_n_graphs), y_current_pix(in_n_graphs), line_style(in_gss), line_width(in_line_width)
{
}

void GraphValueDrawer::paintEvent(QPaintEvent * /* event */)
{
	QPainter  painter(this);
//	QPen pen;
//	pen.setStyle(Qt::DashLine);
//	pen.setColor(Qt::white);
//	pen.setColor(Qt::black);
//	pen.setWidthF(1.5);
//	pen.setColor(Qt::gray);
//	painter.setPen(pen);
//	painter.setPen(palette().dark().color());
//	painter.setBrush(Qt::NoBrush);

	painter.drawLine(x_current_pix, bounds.y1(), x_current_pix, bounds.y2());

	for(int i = 0; i < n_graphs; ++i)
	{
		painter.setPen(GraphStyles::GetGraphStyle(line_style, line_width*0.5, i));
		painter.drawLine(bounds.x1(), y_current_pix[i], bounds.x2(), y_current_pix[i]);
	}
}




}//namespace XRAD_GUI
