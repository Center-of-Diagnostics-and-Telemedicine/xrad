#include "pre.h"
#include "ChartZoom.h"

//TODO: переименовать класс и файл, как сделано с прочими
namespace XRAD_GUI
{



// Конструктор
ChartZoom::ChartZoom(QwtPlot *qp) :
		QObject(qp),
		drag_move(NULL),
		drag_zoom(NULL),
		wheel_zoom(NULL),
		axis_zoom(NULL),
		stored_cursor(Qt::ArrowCursor),
		custom_scale(false)
	{
	// получаем главное окно
	main_window = generalParent(qp);
	// и назначаем обработчик событий (фильтр событий)
	main_window->installEventFilter(this);

	// сбрасываем признак режима
//	convType = e_conversion_none;

	// получаем компонент QwtPlot, над которым будут производиться все преобразования
	plot_component = qp;
	// устанавливаем ему свойство, разрешающее обрабатывать события от клавиатуры
	qp->setFocusPolicy(Qt::StrongFocus);

	qp->replot();   // перестраиваем график
	// Координатная сетка
	QwtPlotGrid *grid = NULL;
	// получаем список элементов графика
	QwtPlotItemList plot_item_list = qp->itemList();
	// перебираем список элементов
	for (int k=0; k < plot_item_list.count(); k++)
		{
		// получаем указатель на элемент
		QwtPlotItem *plot_item = plot_item_list.at(k);
		// если это координатная сетка, то
		if (plot_item->rtti() == QwtPlotItem::Rtti_PlotGrid)
			{
			// запоминаем указатель на нее
			grid = (QwtPlotGrid *)plot_item;
			// прекращаем просмотр списка элементов
			break;
			}
		}
	// если координатная сетка была найдена, то
	if (grid != NULL)
		{
		// выясняем к какой оси она прикреплена
		int xAx = grid->xAxis();    // из пары горизонтальных
		int yAx = grid->yAxis();    // и пары вертикальных
		// назначаем основную и дополнительную шкалу, отдавая предпочтение
		// той, к которой прикреплена сетка
		// горизонтальную
		allocAxis(xAx,QwtPlot::xBottom + QwtPlot::xTop - xAx,&x_axis);
			// вертикальную
		allocAxis(yAx,QwtPlot::yLeft + QwtPlot::yRight - yAx,&y_axis);
		}
	// иначе (координатная сетка отсутствует)
	else
		{
		// назначаем основную и дополнительную шкалу, отдавая предпочтение
			// нижней из горизонтальных
		allocAxis(QwtPlot::xBottom,QwtPlot::xTop,&x_axis);
			// и левой из вертикальных
		allocAxis(QwtPlot::yLeft,QwtPlot::yRight,&y_axis);
		}
	// запоминаем количество делений на горизонтальной шкале
	n_divisions_h = qp->axisMaxMajor(x_axis);
	// запоминаем количество делений на вертикальной шкале
	n_divisions_v = qp->axisMaxMajor(y_axis);
	// создаем контейнеры границ шкалы
// 	scale_bounds_x = new QScaleBounds(qp,x_axis);    // горизонтальной
// 	scale_bounds_y = new QScaleBounds(qp,y_axis);    // и вертикальной
	qp->replot();   // перестраиваем график

	// устанавливаем обработчик всех событий
	plot_component->installEventFilter(this);
	// для всех шкал графика
	for (int ax=0; ax < QwtPlot::axisCnt; ax++)
		{
		// назначаем обработчик событий (фильтр событий)
		plot_component->axisWidget(ax)->installEventFilter(this);
		}

	drag_zoom = new DragZoom(*this);
	drag_move = new DragMove(*this);
	axis_zoom = new AxisZoom(*this);
	wheel_zoom = new WheelZoom(*this);
	}

// Деструктор
ChartZoom::~ChartZoom()
	{
	DestroyObject(drag_move);
	DestroyObject(drag_zoom);
	DestroyObject(axis_zoom);
	DestroyObject(wheel_zoom);
	}

// Определение главного родителя
QObject *ChartZoom::generalParent(QObject *p)
	{
	// берем в качестве предыдущего родителя график
	// (возможен и другой объект в аргументе функции)
	QObject *gp = p;
	// определяем родителя на текущем уровне
	QObject *tp = gp->parent();
	// пока родитель на текущем уровне не NULL
	while (tp != NULL)
		{
		// понижаем уровень:
		// запоминаем в качестве предыдущего родителя текущий
		gp = tp;
		// определяем родителя на следующем уровне
		tp = gp->parent();
		}
	// возвращаем в качестве главного родителя предыдущий
	return gp;
	}

// Назначение основной и дополнительной шкалы
void ChartZoom::allocAxis(int pre,int alt, QwtPlot::Axis *master)
	{
	// получаем карту предпочтительной шкалы
	QwtScaleMap smp = plot_component->canvasMap(pre); // предпочтительной шкалы
	QwtScaleMap sma = plot_component->canvasMap(alt); // и альтернативной
	// если предпочтительная шкала доступна или
	// альтернативная шкала недоступна и при этом
	// границы предпочтительной шкалы не совпадают или
	// границы альтернативной шкалы совпадают, то
	if ((plot_component->axisEnabled(pre) ||
		!plot_component->axisEnabled(alt)) &&
		(smp.s1() != smp.s2() ||
		sma.s1() == sma.s2()))
		{
		// назначаем предпочтительную шкалу основной,
		*master = (QwtPlot::Axis)pre;
		}
	else    // иначе
			// (предпочтительная шкала недоступна и
			// альтернативная шкала доступна или
			// границы предпочтительной шкалы совпадают и
			// границы альтернативной шкалы не совпадают)
		{
		// назначаем альтернативную шкалу основной,
		*master = (QwtPlot::Axis)alt;
		}
	}

// Текущий режим масштабирования
// ChartZoom::chart_conversion_t ChartZoom::ZoomMode() const
// 	{
// 	return convType;
// 	}

// Переключение режима масштабирования
// void ChartZoom::SetZoomMode(ChartZoom::chart_conversion_t ct)
// 	{
// 	convType = ct;
// 	}

// указатель на опекаемый компонент QwtPlot
QwtPlot *ChartZoom::plot()
	{
	return plot_component;
	}

// Основная горизонтальная шкала
const QwtPlot::Axis &ChartZoom::masterH() const
	{
	return x_axis;
	}


// Основная вертикальная шкала
const QwtPlot::Axis &ChartZoom::masterV() const
	{
	return y_axis;
	}


// Установка цвета рамки, задающей новый размер графика
void ChartZoom::setRubberBandColor(QColor clr)
	{
	drag_zoom->setRubberBandColor(clr);
	}



// Создание списка ширины меток горизонтальной шкалы
QList<int> *ChartZoom::getLabelWidths(int xAx)
	{
	// получаем шрифт, использующийся на горизонтальной шкале
	QFont fnt = plot_component->axisFont(xAx);

	// получаем список основных меток горизонтальной шкалы
#if QWT_VERSION < 0x060000
	QwtValueList vl = plot_component->axisScaleDiv(xAx)->ticks(QwtScaleDiv::MajorTick);
#else
	QList<double> vl = plot_component->axisScaleDiv(xAx).ticks(QwtScaleDiv::MajorTick);
#endif

	// создаем список ширины меток
	QList<int> *res = new QList<int>();
	// перебираем все метки
	for (int k=0; k < vl.count(); k++)
		// и заполняем список ширины меток
		res->append(plot_component->axisScaleDraw(xAx)->labelSize(fnt,vl.at(k)).width());
	if (res->count() > 2)   // если в списке больше 2-х меток
		{
		// инициализируем
		int mn = res->at(0);    // минимальную ширину
		int mx = mn;            // и максимальную
		// перебираем оставшиеся метки
		for (int k=1; k < res->count(); k++)
			{
			// берем ширину метки
			int wk = res->at(k);
			// проверяем значение
			if (wk < mn) mn = wk;   // минимальной ширины
			if (wk > mx) mx = wk;   // и максимальной
			}
		// находим в списке минимальную ширину
		int i = res->indexOf(mn);
		// и если нашли, то удаляем ее из списка
		if (i >= 0) res->removeAt(i);
		// находим в списке максимальную ширину
		i = res->indexOf(mx);
		// и если нашли, то удаляем ее из списка
		if (i >= 0) res->removeAt(i);
		}
	// возвращаем список ширины меток
	return res;
	}

// Определение средней ширины меток горизонтальной шкалы
int ChartZoom::meanLabelWidth(int xAx)
	{
	// инициализируем среднюю ширину надписи на одну метку
	int res = 0;
	// получаем список ширины меток
	QList<int> *label_widths = getLabelWidths(xAx);
	// суммируем ширину надписей всех меток
	for (int k=0; k < label_widths->count(); k++) res += label_widths->at(k);
	// и определяем среднюю ширину
	if (label_widths->count() > 0) res = floor(double(res / label_widths->count()));
	// удаляем список ширины меток
	delete label_widths;
	// возвращаем среднюю ширину надписи на одну метку
	return res;
	}

// Обновление горизонтальной шкалы графика
#if 1
bool ChartZoom::updateHorAxis(int ,int *)
	{
	return false;
	// тут делалась какая-то автоматическая подгонка размеров шкалы.
	// надо сделать свой алгоритм и привязать его к интерфейсу
	}
bool ChartZoom::updateVerAxis(int ,int *)
	{
	return false;
	}

#else //1

bool ChartZoom::updateHorAxis(int xAx,int *hDiv)
	{
	// определяем среднюю ширину надписи на одну метку
	int mean_label_width = meanLabelWidth(xAx);
	// определяем значение ширины канвы
	int mnw = plot_component->canvas()->size().width();
	const int dw = 48;  // минимальное расстояние между метками
	// максимально допустимое число основных меток на шкале равно
	// отношению ширины канвы к средней ширине надписи на одну
	// метку (с некоторым запасом)
	int dv = floor(double(mnw/(mean_label_width+dw)));
	bool setF = false;  // обновления шкалы пока нет
	// если максимально допустимое число основных меток
	// на шкале изменилось в большую сторону, то
	if (dv > *hDiv)
		{
		// если после применения изменений окажется, что
		// средняя ширина метки увеличилась на 1 и при этом надписи
		// не поместятся на шкале с требуемым интервалом, то
		// декрементируем максимально допустимое число основных меток
		if (dv*(mean_label_width+dw+1) > mnw) dv--;
		// если количество меток все-таки увеличилось, то
		// устанавливаем флаг - требуется обновление шкалы
		if (dv > *hDiv) setF = true;
		}
	// если максимально допустимое число основных меток
	// на шкале изменилось в меньшую сторону, то
	if (dv < *hDiv)
		{
		// если после применения изменений окажется, что
		// средняя ширина метки уменьшилась на 1 и при этом надписи
		// с требуемым интервалом занимают слишком мало места на шкале, то
		// инкрементируем максимально допустимое число основных меток
		if (dv*(mean_label_width+dw-1) < mnw) dv++;
		// если количество меток все-таки уменьшилось, то
		// устанавливаем флаг - требуется обновление шкалы
		if (dv < *hDiv) setF = true;
		}
	if (setF)   // если требуется обновление шкалы, то
		{
		// устанавливаем вычисленное максимально допустимое число
		// основных меток для горизонтальной шкалы
		plot_component->setAxisMaxMajor(xAx,dv);
		*hDiv = dv;   // запоминаем его
		}
	// возвращаем флаг обновления
	return setF;
	}

// Обновление вертикальной шкалы графика
bool ChartZoom::updateVerAxis(int yAx,int *vDiv)
	{

	// получаем шрифт, использующийся на вертикальной шкале
	QFont fnt = plot_component->axisFont(yAx);
	// узнаем значение верхней границы вертикальной шкалы
	double mxl = plot_component->axisScaleDiv(yAx).upperBound();

	// определяем размер надписи, соответствующей этому значению при заданном шрифте
	#if QWT_VERSION < 0x060000
		QSize szlb = plot_component->axisScaleDraw(yAx)->labelSize(fnt,mxl);
	#else
		QSizeF szlb = plot_component->axisScaleDraw(yAx)->labelSize(fnt,mxl);
	#endif

	// максимально допустимое число основных меток на шкале равно
	// отношению высоты канвы к высоте надписи (с некоторым запасом)
	int dv = floor(double(plot_component->canvas()->size().height()/(szlb.height()+8)));
	// если вычисленное значение не совпадает с установленным, то
	if (dv != *vDiv)
		{
		// устанавливаем вычисленное максимально допустимое число
		// основных меток для вертикальной шкалы
		plot_component->setAxisMaxMajor(yAx,dv);
		*vDiv = dv;     // передаем его в вызывающую процедуру
		return true;    // и возвращаем флаг - шкала обновилась
		}
	// иначе возвращаем флаг - шкала не обновлялась
	else return false;
	}
#endif //1


// Обновление графика
void ChartZoom::updatePlot()
	{
	// обновляем горизонтальную шкалу
	bool repF = updateHorAxis(x_axis,&n_divisions_h);
	// обновляем вертикальную шкалу
	repF |= updateVerAxis(y_axis,&n_divisions_v);

	// Если какая-либо из шкал действительно обновилась
	// (т.е. изменилось максимальное количество меток на шкале),
	// то перестраивается график

	if (repF) plot_component->replot();
	}

// Обработчик всех событий
bool ChartZoom::eventFilter(QObject *target,QEvent *event)
	{
	// если событие произошло для главного окна,
	if (target == main_window)
		{
		// если окно было отображено на экране, или изменились его размеры, то
		if (event->type() == QEvent::Show ||
			event->type() == QEvent::Resize)
			updatePlot();   // обновляем график
		}
	// если событие произошло для графика, то
	if (target == plot_component)
		{
		// если изменились размеры графика, то
		if (event->type() == QEvent::Resize)
			{
			updatePlot();   // обновляем график
			}
		if (event->type() == QEvent::Wheel)
			{
			wheel_zoom->applyWheel(event);
			updatePlot();
			}
		}
	// передаем управление стандартному обработчику событий
	return QObject::eventFilter(target,event);
	}

double	limitation = max_double()/4;

double	limit_min(double y1)
	{
	if(!(fabs(y1) < AbsoluteScaleLimitation()))
		{
		return y1 > 0 ? limitation : -limitation;
		}
	else return y1;
	}

double	limit_max(double y2)
	{
	if(!(fabs(y2) < AbsoluteScaleLimitation()))
		{
		return y2 < 0 ? -limitation : limitation;
		}
	else return y2;
	}


// Восстановление исходных границ графика
void ChartZoom::RestoreBounds()
	{
	// устанавливаем запомненные ранее границы
	plot_component->setAxisScale(x_axis,limit_min(full_data_scale.x1()), limit_max(full_data_scale.x2()));
	plot_component->setAxisScale(y_axis,limit_min(full_data_scale.y1()), limit_max(full_data_scale.y2()));

	custom_scale = false;

	// перестраиваем график
	plot_component->replot();
	ScaleChanged();
	}


void ChartZoom::SetBounds( const range2_F64 &gs, bool custom)
	{

	// подправляем диапазоны во избежание зависания qwt
	double	x1 = limit_min(gs.x1()), x2 = limit_max(gs.x2());
	double	y1 = limit_min(gs.y1()), y2 = limit_max(gs.y2());


	plot_component->setAxisScale(x_axis,x1, x2);
	plot_component->setAxisScale(y_axis,y1, y2);

	custom_scale = custom;

	// перестраиваем график
	plot_component->replot();
	ScaleChanged();
	}

void ChartZoom::SetAutoBounds( const range2_F64 &gs )
	{
	full_data_scale = gs;
	ScaleChanged();
	}

range2_F64 ChartZoom::GetCurrentBounds()
	{
	QwtScaleMap scale_map_x = plot_component->canvasMap(x_axis);
	QwtScaleMap scale_map_y = plot_component->canvasMap(y_axis);

	return range2_F64(scale_map_y.s1(), scale_map_x.s1(), scale_map_y.s2(),  scale_map_x.s2());
	}


void ChartZoom::SetCursor( const QCursor new_cursor)
	{
	// устанавливаем курсор new_cursor
	stored_cursor = plot()->cursor();
	plot()->setCursor(new_cursor);
	}

void ChartZoom::ResetCursor()
	{
	plot()->setCursor(stored_cursor);
	}

range2_F64 ChartZoom::GetScale()
	{
	range2_F64 result;
	QwtScaleMap scale_map_master_h = plot()->canvasMap(x_axis);
	QwtScaleMap scale_map_master_v = plot()->canvasMap(y_axis);


	result.x1() = scale_map_master_h.s1();
	result.x2() = scale_map_master_h.s2();
	result.y1() = scale_map_master_v.s1();
	result.y2() = scale_map_master_v.s2();

	return result;
	}



}//namespace XRAD_GUI

// запасы примеров, оказавшихся ненужными

// Добавление в маску индикатора вертикальных линий сетки для меток горизонтальной шкалы
// QRegion QDragMove::addHorTicks(QRegion rw,QwtScaleDiv::TickType tt)
// 	{
// 	// получаем указатель на график
// 	QwtPlot *plt = zoom->plot();
// 	// получаем список основных меток горизонтальной шкалы
// 	QList<double> vl = plt->axisScaleDiv(zoom->masterH()).ticks(tt);
//
// 	// перебираем все метки горизонтальной шкалы
// 	for (int k=0; k < vl.count(); k++)
// 		{
// 		// вычисляем смещение метки относительно канвы
// 		int x = plt->transform(zoom->masterH(),vl.at(k));
// 		// формируем вертикальную линию сетки
// 		QRegion rs(x-1,1,1,rw.boundingRect().height()-2);
// 		// добавляем ее в маску
// 		rw = rw.united(rs);
// 		}
// 	// возвращаем измененную маску
// 	return rw;
// 	}

// Добавление в маску индикатора горизонтальных линий сетки для меток вертикальной шкалы
// QRegion QDragMove::addVerTicks(QRegion rw,QwtScaleDiv::TickType tt)
// 	{
// 	// получаем указатель на график
// 	QwtPlot *plt = zoom->plot();
// 	// получаем список основных меток вертикальной шкалы
// 	QList<double> vl = plt->axisScaleDiv(zoom->masterV()).ticks(tt);
//
// 	// перебираем все метки вертикальной шкалы
// 	for (int k=0; k < vl.count(); k++)
// 		{
// 		// вычисляем смещение метки относительно канвы
// 		int y = plt->transform(zoom->masterV(),vl.at(k));
// 		// формируем горизонтальную линию сетки
// 		QRegion rs(1,y-1,rw.boundingRect().width()-2,1);
// 		// добавляем ее в маску
// 		rw = rw.united(rs);
// 		}
// 	// возвращаем измененную маску
// 	return rw;
// 	}
