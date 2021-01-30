#include "pre.h"
#include "PainterWindow.h"

#include "ThreadGUI.h"
#include "GUIController.h"
#include "FileSaveUtils.h"
#include <XRADQt/QtStringConverters.h>
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



PainterWindow::PainterWindow(const QString &title, size_t in_vsize, size_t in_hsize, GUIController &gc)
	:
	DataDisplayWindow(gc)
{
	try
	{
//		setupUi(this);

		// задаем положение окна
		auto corner = GetCornerPosition();
		setGeometry(QRect(QPoint(corner.x(), corner.y()), QPoint(in_vsize, in_hsize)));

//		SetBackground(Qt::white, Qt::SolidPattern);

//TODO	Копировать сюда остальное


		// устанавливаем обработчик событий
//		plot->installEventFilter(this);

//		QObject::connect(cbShowSymbol, SIGNAL(toggled(bool)), this, SLOT(slotSetSymbol(bool)));
//		QObject::connect(cbIgnoreScale, SIGNAL(toggled(bool)), this, SLOT(slotSetTransform(bool)));


		//добавляем объект в массив диалогов
		gui_controller.AddWidget(this);
		setAttribute(Qt::WA_DeleteOnClose);
		installEventFilter(this);
	}
	catch(...)
	{
	}
}

PainterWindow::~PainterWindow()
{
	//удаляем объект из массива диалогов
	gui_controller.RemoveWidget(this);

}

void PainterWindow::closeEvent(QCloseEvent *event)
{
	QDialog::closeEvent(event);
}

/*
void PainterWindow::SavePicture(QString filename)
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

*/


void PainterWindow::slotSavePicture()
{
	const char *prompt = "Save picture";
	const char *type = "png (*.png);;pdf (*.pdf);;svg (*.svg);;jpeg (*.jpg);;bmp (*.bmp)";
	QString filename = GetSaveFileName(QFileDialog::tr(prompt), QFileDialog::tr(type));

	SavePicture(filename);
}

// Обработчик всех событий
bool PainterWindow::eventFilter(QObject *target, QEvent *event)
{


	// если событие произошло для графика, то

	//if(target == plot)
	//{
	//	// если произошло одно из событий от мыши, то
	//	switch(event->type())
	//	{
	//		case QEvent::MouseButtonPress:
	//		case QEvent::MouseMove:
	//		case QEvent::MouseButtonRelease:
	//			procMouseEvent(event);
	//			break;

	//		case QEvent::Resize:
	//		case QEvent::Show:
	//			// 			case QEvent::UpdateRequest:
	//			// 			case QEvent::Paint:
	//			UpdateAxesDrawer();
	//		default:
	//			break;
	//	};
	//}
	// передаем управление стандартному обработчику событий
	return QObject::eventFilter(target, event);
}

void PainterWindow::keyPressEvent(QKeyEvent *event)
{
	//if(event->type()==QEvent::KeyPress)
	//{
	//	switch(event->key())
	//	{
	//		case Qt::Key_S:
	//			if(event->modifiers() == Qt::ControlModifier) slotSavePicture();
	//			break;

	//		case Qt::Key_Escape:
	//			emit signal_esc();
	//			break;
	//	};
	//}
	return QWidget::keyPressEvent(event);
}



// Обработчик обычных событий от мыши
void PainterWindow::procMouseEvent(QEvent *event)
{
	// создаем указатель на событие от мыши
	QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);

	// в зависимости от типа события вызываем соответствующий обработчик
	//switch(event->type())
	//{
	//	// нажата кнопка мыши
	//	case QEvent::MouseButtonPress:
	//		StartCurrentValueDraw(mEvent);
	//		break;
	//		// перемещение мыши
	//	case QEvent::MouseMove:
	//		DrawCurrentValue(mEvent);
	//		break;
	//		// отпущена кнопка мыши
	//	case QEvent::MouseButtonRelease:
	//		EndCurrentValueDraw(mEvent);
	//		break;
	//		// для прочих событий ничего не делаем
	//	default:;
	//}
}


}//namespace XRAD_GUI
