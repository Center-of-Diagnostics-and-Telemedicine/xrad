/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "ImageWindow.h"

#include "GUIController.h"
#include "FileSaveUtils.h"

#include <XRADQt/QtStringConverters.h>
#include <XRADSystem/CFile.h>
#include <XRADSystem/System.h>
#include <XRADBasic/Sources/Utils/TimeProfiler.h>
#include <XRADGui/Sources/GUI/Keyboard.h>

namespace XRAD_GUI
{

XRAD_USING



point2_I32	ImageWindow::LocalMousePosition(QEvent *event)
{
	switch(event->type())
	{
		case QEvent::MouseButtonPress:
		case QEvent::MouseMove:
		case QEvent::MouseButtonRelease:
		{
			QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
			return	point2_I32(mouse_event->pos().y(), mouse_event->pos().x());
		}

		case QEvent::HoverEnter:
		case QEvent::HoverMove:
		case QEvent::HoverLeave:
		{
			QHoverEvent *hover_event = static_cast<QHoverEvent *>(event);
			return	point2_I32(hover_event->pos().y(), hover_event->pos().x());
		}
		default:
		{
			QPoint p = raster->mapFromGlobal(QCursor::pos());
			return point2_I32(p.y(), p.x());
		}
	}
}

//--------------------------------------------------------------
//
//	конструкторы и деструктор
//

ImageWindow::ImageWindow(QString title, size_t vs, size_t hs, GUIController &gc) :
	DataDisplayWindow(gc),
	n_frames(0), n_rows(vs), n_columns(hs), n_samples_total(vs*hs),
	current_frame(-1),
	current_frame_bitmap(vs, hs),
	update_count(0), freehand_brithess_change_counter(0)
{
	try
	{
		setupUi(this);
		// назначаем обработчик событий (фильтр событий)
		// installEventFilter(this); ушел в DataDisplayWindow


		SetWindowPosition();// нужна здесь, т.к. зависит от размеров изображения

		current_value_label->setText("");
		setWindowTitle(title);


		//добавляем объект в массив диалогов (ушло вродителя)
	// 	gui_controller.AddWidget(this);
	// 	setAttribute(Qt::WA_DeleteOnClose);

		save_frame_button->setVisible(true);
		dt_label->setVisible(false);
		dt_zoom_box->setVisible(false);
		save_video_button->setVisible(false);
		frames_slider->setVisible(false);
		frame_no_label->setVisible(false);

		//	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

		QObject::connect(frames_slider, SIGNAL(valueChanged(int)), this, SLOT(ShowFrame(int)));

		QObject::connect(x_zoom_box, SIGNAL(valueChanged(double)), this, SLOT(RebuildPixmap()));
		QObject::connect(y_zoom_box, SIGNAL(valueChanged(double)), this, SLOT(RebuildPixmap()));
		QObject::connect(cb_transpose, SIGNAL(stateChanged(int)), this, SLOT(RebuildPixmap()));

		QObject::connect(save_frame_button, SIGNAL(clicked()), this, SLOT(SaveFrame()));
		QObject::connect(save_video_button, SIGNAL(clicked()), this, SLOT(SaveVideo()));

		SetDefaultBrightness(0, 255, 1);

		dt_zoom_box->installEventFilter(this);
		x_zoom_box->installEventFilter(this);
		y_zoom_box->installEventFilter(this);

		black_point_box->installEventFilter(this);
		white_point_box->installEventFilter(this);
		bw_order_box->installEventFilter(this);
		bw_range_order_box->installEventFilter(this);

		gamma_box->installEventFilter(this);
		bw_range_box->installEventFilter(this);
		bw_center_box->installEventFilter(this);
		frames_slider->installEventFilter(this);

		raster->installEventFilter(this);
		raster->setAttribute(Qt::WA_Hover, true);

		QObject::connect(reset_brightness_button, SIGNAL(clicked()), this, SLOT(ResetBrightnessDefault()));
		QObject::connect(full_brightness_button, SIGNAL(clicked()), this, SLOT(ResetBrightnessFull()));

		animation_timer = new QTimer(this);
		paused = true;
		direction = 1;
		animation_mode = e_forward;

		QObject::connect(animation_timer, SIGNAL(timeout()), this, SLOT(AnimationUpdate()));
		QObject::connect(dt_zoom_box, SIGNAL(valueChanged(double)), this, SLOT(UpdateAnimationTimer()));

		x0 = 0;
		y0 = 0;
		dx = 1;
		dy = 1;

		mouse_drag_mode = mouse_drag_none;
		frames_slider->setFocus();

		installEventFilter(this);
	}
	catch(...)
	{
	}
}

// void	ImageWindow::SetWindowTitle(QString title)
// {
// 	setWindowTitle(title);
// }


ImageWindow::~ImageWindow()
{
	//удаляем объект из массива диалогов (ушло в родителя)
//	gui_controller.RemoveWidget(this);
}

//--------------------------------------------------------------

void ImageWindow::ResetBrightnessControlsLimits()
{
	// раздвинуть пределы контролей до максимума, без этого, не дает установить некоторые нужные значения
	// потом, естественно, пересчитать пределы (функция ниже)
	white_point_box->setMinimum(-max_double());
	white_point_box->setMaximum(max_double());
	black_point_box->setMinimum(-max_double());
	black_point_box->setMaximum(max_double());
	bw_center_box->setMaximum(max_double());
	bw_center_box->setMaximum(max_double());
	bw_range_box->setMaximum(max_double());
	bw_range_box->setMaximum(max_double());
}

void ImageWindow::SetBrightnessControlsLimits()
{
	const double	maxmin_step_divisor = 20;
	const double	gamma_step_divisor = 20;

	double	maxmin_delta = control_bw_range()/maxmin_step_divisor;
	// шаг точность листбоксов меняется в зависимости от возможной точности изменений
	int	required_bounds_precision = range(-log10(maxmin_delta/bw_order()) + 1, 1, 10);
	int	required_range_precision = range(-log10(maxmin_delta/bw_range_order()) + 1, 1, 10);

	gamma_box->setMinimum(0.0);
	gamma_box->setMaximum(max_double());
	gamma_box->setSingleStep(control_gamma()/gamma_step_divisor);


	white_point_box->setMinimum((internal_black+maxmin_delta)/bw_order());
	white_point_box->setMaximum(max_double());
	white_point_box->setSingleStep(maxmin_delta/bw_order());
	white_point_box->setDecimals(required_bounds_precision);

	black_point_box->setMinimum(-max_double());
	black_point_box->setMaximum((internal_white-maxmin_delta)/bw_order());
	black_point_box->setSingleStep(maxmin_delta/bw_order());
	black_point_box->setDecimals(required_bounds_precision);

	bw_center_box->setMinimum(-max_double());
	bw_center_box->setMaximum(max_double());
	bw_center_box->setSingleStep(maxmin_delta/bw_order());
	bw_center_box->setDecimals(required_bounds_precision);

	bw_range_box->setMinimum(maxmin_delta/(100*bw_range_order()));
	bw_range_box->setMaximum(max_double());
	bw_range_box->setSingleStep(maxmin_delta/(2*bw_range_order()));
	bw_range_box->setDecimals(required_range_precision);
}

//--------------------------------------------------------------
//
//	инициализации
//

void ImageWindow::SetFrameLegend()
{
	QString	label = QString("frame no = %1/%2\n").arg(current_frame).arg(n_frames-1) +
		QString("%1=%2").arg(z_label).arg(z0+dz*current_frame);
	frame_no_label->setText(label);
}

void ImageWindow::RebuildPixmap()
{
	if(in_range(current_frame, 0, n_frames-1))
	{
		try
		{
			frames[current_frame]->GenerateBitmap(current_frame_bitmap, internal_black, internal_white, control_gamma(), transposed());
		}
		catch(...)
		{
		}
	}

	int	new_x_size = max(32., current_frame_bitmap.hsize()*x_zoom());
	int	new_y_size = max(32., current_frame_bitmap.vsize()*y_zoom());
	raster->setFixedHeight(new_y_size);
	raster->setFixedWidth(new_x_size);

	QPixmap pixmap(new_x_size, new_y_size);
	pixmap.loadFromData((const uint8_t*)current_frame_bitmap.GetBitmapFile(), int(current_frame_bitmap.GetBitmapFileSize()), "BMP");
	QPixmap scaled_pixmap = pixmap.scaled(new_x_size, new_y_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	raster->setPixmap(scaled_pixmap);
	setFixedSize(minimumSizeHint());
}

void ImageWindow::SetAxesScales(double in_z0, double in_dz, double in_y0, double in_dy, double in_x0, double in_dx)
{
	z0 = in_z0;
	x0 = in_x0;
	y0 = in_y0;
	dz = in_dz;
	dx = in_dx;
	dy = in_dy;
}

void ImageWindow::UpdateBrightness(double in_black, double in_white, double in_gamma)
{
	if(update_count) return;
	++update_count;

	internal_black = in_black;
	internal_white = in_white;
	double	maxval = max(fabs(internal_black), fabs(internal_white));
	double	maxrange = fabs(internal_black - internal_white);

	int	internal_bw_order = find_order(maxval);
	int	internal_bw_range_order = find_order(maxrange);

	ResetBrightnessControlsLimits();
	bw_range_order_box->setValue(internal_bw_range_order);
	bw_order_box->setValue(internal_bw_order);

	bw_range_order_box->setVisible(internal_bw_range_order ? true:false);
	bw_range_order_box_label->setVisible(internal_bw_range_order ? true:false);
	bw_order_box->setVisible(internal_bw_order ? true:false);
	bw_order_box_label->setVisible(internal_bw_order ? true:false);

	gamma_box->setValue(in_gamma);

	bw_range_box->setValue((internal_white-internal_black)/bw_range_order());
	bw_center_box->setValue((internal_white+internal_black)/(2.*bw_order()));
	white_point_box->setValue(internal_white/bw_order());
	black_point_box->setValue(internal_black/bw_order());

	SetBrightnessControlsLimits();
	--update_count;
}

void ImageWindow::SetDefaultBrightness(double in_black, double in_white, double in_gamma)
{
	default_black = in_black;
	default_white = in_white;
	default_gamma = in_gamma;

	UpdateBrightness(default_black, default_white, default_gamma);
}

void ImageWindow::SetWindowPosition()
{
	auto corner = GetCornerPosition();

	// подгоняем размеры окна под изображение
	int	t, b, l, r;
	layout()->getContentsMargins(&l, &t, &r, &b);
	int	dh = 12;// наобум, чтобы картинка целиком помещалась. примерно
	// компенсирует место под лейблу что ли?
	setGeometry(QRect(QPoint(corner.x(), corner.y()), QSize(int(n_columns)+l+r, int(n_rows) + t+b + dh)));
}



//--------------------------------------------------------------
//
//	добавление, удаление и изменение кадров
//

void ImageWindow::AddFrames(size_t n)
{
	for(size_t i = 0; i < n; ++i)
	{
		frames.push_back(make_shared<MultimodalFrameContainer>());
		++n_frames;
	}

	if(n_frames > 1)
	{
		dt_label->setVisible(true);
		dt_zoom_box->setVisible(true);
		save_video_button->setVisible(true);

		frames_slider->setVisible(true);
		frame_no_label->setVisible(true);
	}
	frames_slider->setRange(0, int(n_frames)-1);
	SetFrameLegend();
}

void ImageWindow::EraseFrame(int in_frame_no)
{
	if(!in_range(in_frame_no, 0, n_frames-1)) return;
	--n_frames;
	frames.erase(frames.begin()+in_frame_no);
	if(n_frames <= 1)
	{
		dt_label->setVisible(false);
		dt_zoom_box->setVisible(false);
		save_video_button->setVisible(false);

		frames_slider->setVisible(false);
		frame_no_label->setVisible(false);
	}
	current_frame = range(current_frame, 0, n_frames-1);

	frames_slider->setRange(0, int(n_frames)-1);
	SetFrameLegend();
	RebuildPixmap();
}

void ImageWindow::SetupFrame(int in_frame_no, const void* in_data, display_sample_type pt)
{
	int	frame_no = in_frame_no;
	try
	{
		if(!in_data)
		{
			EraseFrame(frame_no);
			return;
		}
		if(!in_range(frame_no, 0, n_frames-1) && in_data)
		{
			AddFrames();
			frame_no = int(n_frames)-1;
		}

		frames[frame_no]->ImportFrame(in_data, int(n_rows), int(n_columns), pt);

		RebuildPixmap();
	}
	catch(...)
	{
	}

	if(n_frames==1 || current_frame==-1) ShowFrame(frame_no);
		// при изменении единственного кадра перерисовываем сразу
		// также принудительно перерисовываем, если это первый из сформированных кадров
}



//--------------------------------------------------------------
//
//	обработчики событий
//

void ImageWindow::AnalyzeBrightnessChangeControls(QObject *target, QEvent *event)
{
	QEvent::Type	event_type = event->type();
	bool	is_mouse_event = (
		event_type == QEvent::MouseButtonPress ||
		// 				event_type == QEvent::MouseMove ||
		event_type == QEvent::MouseButtonRelease ||
		// 				event_type == QEvent::HoverEnter ||
		// 				event_type == QEvent::HoverMove ||
		// 				event_type == QEvent::HoverLeave ||
		event_type == QEvent::Wheel
		);

	bool	is_key_event = (
		event_type == QEvent::FocusIn ||
		event_type == QEvent::FocusOut
		);

	if(event_type == QEvent::KeyPress || event_type == QEvent::KeyRelease)
	{
		int	key = static_cast<QKeyEvent*>(event)->key();
		if(key == Qt::Key_Enter || key == Qt::Key_Up || key == Qt::Key_Down) is_key_event = true;
	}


	if(is_mouse_event || is_key_event)
	{
		brightness_change_reason_t brightness_change_reason;
		if(target == black_point_box || target == white_point_box || target == bw_order_box)
		{
			brightness_change_reason = bw_point_control;
		}
		else if(target == bw_center_box || target == bw_range_box || target==bw_range_order_box)
		{
			brightness_change_reason = bw_range_control;
		}
		else if(target == gamma_box)
		{
			brightness_change_reason = gamma_control;
		}
		else
		{
			brightness_change_reason = controls_not_changed;
		}
		AdjustBrightnessControls(brightness_change_reason);
	}
}

void ImageWindow::AnalyzeUpdateEvent(QObject *target, QEvent *event)
{
	QEvent::Type	event_type = event->type();
	// если событие произошло для растра, то
	if(target == raster)
	{
		RasterEvent(event);

		// если изменились размеры изображения
		if(event_type == QEvent::Resize)
		{
			UpdateRaster();
		}
	}
	// если событие произошло для этого oкна
	if(target == this)
	{
		// если окно было отображено на экране, или изменились его размеры, то
		if(event_type == QEvent::Show || event_type == QEvent::Resize)
		{
			UpdateRaster();
		}
	}
//		AnimationUpdate();
}

void ImageWindow::KeyPressAnalyzer(QObject * /*target*/, QEvent *event)
{
	if(event->type() != QEvent::KeyPress) return;
	QKeyEvent *key_event = static_cast<QKeyEvent*>(event);

	switch(key_event->key())
	{
		case Qt::Key_Escape:
			emit signal_esc();
			break;

		case Qt::Key_F:
			frames_slider->setFocus();
			animation_mode = e_forward;
			direction = 1;
			break;

		case Qt::Key_L:
			frames_slider->setFocus();
			animation_mode = e_loop;
			break;

		case Qt::Key_R:
			frames_slider->setFocus();
			animation_mode = e_reverse;
			direction = -1;
			break;

		case Qt::Key_Space:
			frames_slider->setFocus();
			// корявая заплата, вообще говоря. управление передается сюда не один раз, приходится отсекать лишнее
			if((GetPerformanceCounter()-t0).sec() > 0.05)
			{
				paused = !paused;
				if(!paused) animation_timer->start(dt_zoom_box->value());
				else animation_timer->stop();
				t0 = GetPerformanceCounter();
			}
			break;
	}
	if(focusWidget() == frames_slider)
	{
		//			int	k = key_event->key();
		switch(key_event->key())
		{
			case Qt::Key_Left:
				if(!paused) direction=-1;
				if(animation_mode!=e_loop) animation_mode=e_reverse;
				break;

			case Qt::Key_Right:
				if(!paused) direction = 1;
				if(animation_mode!=e_loop) animation_mode=e_forward;
				break;
		}
	}
}

// Обработчик всех событий
bool ImageWindow::eventFilter(QObject *target, QEvent *event)
{
	AnalyzeBrightnessChangeControls(target, event);
	AnalyzeUpdateEvent(target, event);
	KeyPressAnalyzer(target, event);

	// передаем управление стандартному обработчику событий
	return parent::eventFilter(target, event);
}

void ImageWindow::UpdateAnimationTimer()
{
	if(!paused)
	{
		animation_timer->stop();
		animation_timer->start(dt_zoom_box->value());
	}
}

void ImageWindow::AnimationUpdate()
{
	current_frame += direction;
	switch(animation_mode)
	{
		case e_forward:
			if(current_frame > int(n_frames)-1)
			{
				current_frame = 0;
			}
			break;

		case e_reverse:
			if(current_frame < 0)
			{
				current_frame = int(n_frames)-1;
			}
			break;

		case e_loop:
			if(current_frame < 0)
			{
				current_frame = 0;
				direction = 1;
			}
			if(current_frame > int(n_frames)-1)
			{
				current_frame = int(n_frames)-1;
				direction = -1;
			}
			break;

		default:
			break;
	}
	frames_slider->setValue(current_frame);
}

void ImageWindow::RasterEvent(QEvent *event)
{
	switch(event->type())
	{
		// нажата кнопка мыши
		case QEvent::MouseButtonPress:
			if(mouse_drag_mode==mouse_drag_value_analyze) EndCurrentValueDraw(event);
			StartFreehandBrightnessChange(event);
			break;

			// отпущена кнопка мыши
		case QEvent::MouseButtonRelease:
			EndFreehandBrightnessChange(event);
			break;

		case QEvent::MouseMove:
			if(mouse_drag_mode==mouse_drag_contrast_change) FreehandBrightnessChange(event);

			break;

		case QEvent::HoverEnter:
			StartCurrentValueDraw(event);
			break;

		case QEvent::HoverLeave:
			if(mouse_drag_mode==mouse_drag_value_analyze) EndCurrentValueDraw(event);
			break;

		default:
		case QEvent::HoverMove:
			if(mouse_drag_mode==mouse_drag_none) StartCurrentValueDraw(event);
			if(mouse_drag_mode==mouse_drag_value_analyze) DrawCurrentValue(event);
			break;
	}
}



void ImageWindow::closeEvent(QCloseEvent *event)
{
	QDialog::closeEvent(event);
}



// Обновление рисунка
void ImageWindow::UpdateRaster()
{
	//TODO написать
}



//--------------------------------------------------------------
//
//	отображение текущих координат и значения изображения
//

// включаем режим отображения координат
void ImageWindow::StartCurrentValueDraw(QEvent *event)
{
	mouse_drag_mode = mouse_drag_value_analyze;
	setCursor(Qt::CrossCursor);
	DrawCurrentValue(event);
}

// отображение текущих координат курсора
void ImageWindow::DrawCurrentValue(QEvent *event)
{
	// исключается обращение к окну, в которое еще не добавлены данные
	if(!n_frames) return;

	point2_I32  current_cursor_position = LocalMousePosition(event);

	// 		if(transposed()) std::swap(current_cursor_position.x(), current_cursor_position.y());


	int	col_no = current_cursor_position.x() / x_zoom();
	int	row_no = current_cursor_position.y() / y_zoom();

	if(transposed()) std::swap(col_no, row_no);

	try
	{
		// находим значение цвета, если не вышли за пределы картинки
		if(in_range(row_no, 0, n_rows-1) && in_range(col_no, 0, n_columns-1) && in_range(current_frame, 0, frames.size()-1))
		{
			double	x = x0 + col_no*dx;
			double	y = y0 + row_no*dy;
			setCursor(Qt::CrossCursor);
			//TODO сделать, чтобы отображались и колонки, и координаты
			QString value_legend = frames[current_frame]->GetValueLegend(row_no, col_no);
			QString	label = QString("row=%1, col=%2, %3[%4]").arg(row_no).arg(col_no).arg(value_label).arg(value_legend);
			bool	linefeed(true);

			if(y0 || dy!=1 || y_label != "row")
			{
				if(linefeed) label += "\n", linefeed = false;
				label += QString("y(%1)=%2").
					arg(y_label).
					arg(y);
			}

			if(x0 || dx!=1 || x_label != "col")
			{
				if(linefeed) label += "\n", linefeed = false;
				else label += "; ";
				label += QString("x(%1)=%2").
					arg(x_label).
					arg(x);
			}

			current_value_label->setText(label);

		}
		else
		{
			current_value_label->setText(QString(""));
			setCursor(Qt::ArrowCursor);
		}
	}
	catch(...)
	{
	}
}

void	ImageWindow::SetBrightnessContrastCursor()
{
	QString	cursor_filename = ":/cursors/brightness_contrast.png";
	QFileInfo info(cursor_filename);

	if(!info.exists() /*|| !info.completeSuffix().isEmpty()*/)
	{
		setCursor(Qt::SizeAllCursor);
	}
	else
	{
		setCursor(QCursor(QPixmap(cursor_filename)));
	}
}

double	ImageWindow::find_order(double val)
{
	if(fabs(val) >= 1e3) return log10(val);
	else if(fabs(val) <= 1e-1) return log10(val)-1;
	else return 0;
}

void	ImageWindow::AdjustBrightnessControls(brightness_change_reason_t brightness_change_reason)
{
	switch(brightness_change_reason)
	{
		case bw_point_control:
			UpdateBrightness(control_black(), control_white(), control_gamma());
			RebuildPixmap();
			break;

		case bw_range_control:
			UpdateBrightness(control_bw_center() - control_bw_range()/2, control_bw_center() + control_bw_range()/2, control_gamma());
			RebuildPixmap();
			break;

		case gamma_control:
			SetBrightnessControlsLimits();
			RebuildPixmap();
	}
}

void	ImageWindow::ResetBrightnessDefault()
{
	UpdateBrightness(default_black, default_white, default_gamma);
	RebuildPixmap();
}

void	ImageWindow::ResetBrightnessFull()
{
	double	max_value = -max_double();
	double	min_value = max_double();

	for(size_t i = 0; i < n_frames; ++i)
	{
		max_value = max(frames[i]->MaxComponentValue(), max_value);
		min_value = min(frames[i]->MinComponentValue(), min_value);
	}

	UpdateBrightness(min_value, max_value, control_gamma());
	RebuildPixmap();
}



void ImageWindow::StartFreehandBrightnessChange(QEvent *event)
{
	mouse_drag_mode = mouse_drag_contrast_change;
	SetBrightnessContrastCursor();

	start_bw_center = control_bw_center();
	start_bw_range = control_bw_range();
	start_blackpoint = internal_black;
	start_whitepoint = internal_white;

	start_cursor_position = LocalMousePosition(event);
	FreehandBrightnessChange(event);
	freehand_brithess_change_counter = 0;
}

void ImageWindow::EndFreehandBrightnessChange(QEvent *event)
{
	point2_I32  current_cursor_position = LocalMousePosition(event);
	if(start_cursor_position == current_cursor_position && !freehand_brithess_change_counter)
	{
		// однократный shift-click должен приводить к сбросу изменений
		if(static_cast<QMouseEvent*>(event)->modifiers() == Qt::ShiftModifier) ResetBrightnessDefault();
	}
	setCursor(Qt::ArrowCursor);
	mouse_drag_mode = mouse_drag_none;
}

void ImageWindow::FreehandBrightnessChange(QEvent *event)
{
	try
	{
		point2_I32  current_cursor_position = LocalMousePosition(event);
		if(start_cursor_position == current_cursor_position) return;

		double	max_amplification = 1000;
		double	dm = 1. - 1./max_amplification;

		double	horizontal_offset = range(double(current_cursor_position.x()-start_cursor_position.x())/256, -dm, dm);
		double	vertical_offset = double(current_cursor_position.y()-start_cursor_position.y())/256;

		double	bw_center_factor =  -vertical_offset;
		double	bw_range_factor = horizontal_offset<0 ? (1. + horizontal_offset) : 1./(1.-horizontal_offset);

		double new_bw_center = start_bw_center + start_bw_range * bw_center_factor;
		double new_bw_range = start_bw_range * bw_range_factor;
		double new_blackpoint = new_bw_center - new_bw_range/2;
		double new_whitepoint = new_bw_center + new_bw_range/2;

		current_value_label->setText(
			QString("black inc. = %1, white inc = %2\n").arg(new_blackpoint-start_blackpoint).arg(new_whitepoint-start_whitepoint) +
			QString("bw center inc. = %1, bw range inc. = %2x").arg(new_bw_center-start_bw_center).arg(new_bw_range/start_bw_range)
		);

		UpdateBrightness(new_blackpoint, new_whitepoint, control_gamma());
		RebuildPixmap();
		++freehand_brithess_change_counter;
	}
	catch(...)
	{
	}
}

void ImageWindow::EndCurrentValueDraw(QEvent *)
{
	// восстанавливаем курсор, сбрасываем режим, очищаем текст
	setCursor(Qt::ArrowCursor);
	current_value_label->setText("");
	mouse_drag_mode = mouse_drag_none;
}



//--------------------------------------------------------------
//
//	slots
//

void	ImageWindow::ShowFrame(int in_frame_no)
{
	if(!in_range(in_frame_no, 0, n_frames-1)) return;

	//	QVector<qreal> values(n_samples_total);
	current_frame = in_frame_no;
	SetFrameLegend();
	//		frames_slider->setValue(in_frame_no);

	// заполняем данными
	RebuildPixmap();
}

void ImageWindow::SetImageLabels(const QString &in_title, const QString &in_z_label, const QString &in_y_label, const QString &in_x_label, const QString &in_value_label)
{
	z_label = in_z_label.isEmpty() ? "frame number" : in_z_label;
	x_label = in_x_label.isEmpty() ? "col" : in_x_label;
	y_label = in_y_label.isEmpty() ? "row" : in_y_label;
	value_label = in_value_label;

	setWindowTitle(in_title);
}

void ImageWindow::SaveFrame()
{
	const char *prompt = "Save picture";
	const char *type = "png (*.png);;jpeg (*.jpg);;bmp (*.bmp);;pdf (*.pdf)";
	QString file_name = GetSaveFileName(QFileDialog::tr(prompt), QFileDialog::tr(type));
	if(file_name.isEmpty()) return;
	if(FormatByFileExtension(file_name) == "bmp")
	{
		shared_cfile	file;
		file.open(qstring_to_wstring(file_name), L"wb");
		file.write(current_frame_bitmap.GetBitmapFile(), current_frame_bitmap.GetBitmapFileSize(), 1);
	}
	else
	{
		raster->pixmap()->save(file_name);
	}
}

void ImageWindow::SaveVideo()
{
	const char *prompt = "Enter folder name for export";
	const char *type = "png (*.png);;jpeg (*.jpg);;bmp (*.bmp)";
	QString folder_name = GetSaveFileName(QFileDialog::tr(prompt), QFileDialog::tr(type));
	if(folder_name.isEmpty()) return;
	string	image_file_format = FormatByFileExtension(folder_name).toStdString();

	QDir dir;
	dir.mkdir(folder_name);

	const char	*filename_format = CapsLock() ? "%5zu.%s" : "%05zu.%s";

	for(size_t i = 0; i < n_frames; ++i)
	{
//		QString	fn;
		QString file_name_with_path = folder_name + path_separator() +
				QString::asprintf(filename_format, EnsureType<size_t>(i), EnsureType<const char*>(image_file_format.c_str()));
		frames_slider->setValue(int(i));
		if(image_file_format == "bmp")
		{
			shared_cfile	file;
			file.open(qstring_to_wstring(file_name_with_path), L"wb");
			file.write(current_frame_bitmap.GetBitmapFile(), current_frame_bitmap.GetBitmapFileSize(), 1);
		}
		else
		{
			raster->pixmap()->save(file_name_with_path, image_file_format.c_str());
		}
		repaint();
	}
}

}//namespace XRAD_GUI



//--------------------------------------------------------------
// class ColorMap
//--------------------------------------------------------------

/*
к вопросу о сохранении видео
FFMPEG
void nmain() {

// input stuff
AVFormatContext *formatCtxIn=0;
AVInputFormat *formatIn=0;
AVCodecContext *codecCtxIn=0;
AVCodec *codecIn;
AVPacket *pktIn;

av_register_all();
avdevice_register_all();
avcodec_register_all();


formatIn = av_find_input_format("dshow");

if(!formatIn)
return;


AVDictionary *avoption=0;
av_dict_set(&avoption, "rtbufsize", "1000000000", NULL);

if(avformat_open_input(&formatCtxIn, "video=Integrated Camera", formatIn, &avoption)!=0)
return;

if(avformat_find_stream_info(formatCtxIn, NULL)<0)
return;

codecCtxIn = formatCtxIn->streams[0]->codec;
codecIn = avcodec_find_decoder(codecCtxIn->codec_id);

if(avcodec_open2(codecCtxIn, codecIn, NULL)<0)
return;


// end input stuff
//--------------------------------------------------------------
// output stuff

AVOutputFormat *formatOut=0;
AVFormatContext *formatCtxOut=0;
AVStream *streamOut=0;
AVFrame *frame=0;
AVCodec *codecOut=0;
AVPacket *pktOut;

const char *filename = "test.mpeg";

formatOut = av_guess_format(NULL, filename, NULL);
if(!formatOut)
formatOut = av_guess_format("mpeg", NULL, NULL);
if(!formatOut)
return;

formatCtxOut = avformat_alloc_context();
if(!formatCtxOut)
return;

formatCtxOut->oformat = formatOut;

sprintf(formatCtxOut->filename, "%s", filename);

if(formatOut->video_codec != AV_CODEC_ID_NONE) {
AVCodecContext *ctx;

codecOut = avcodec_find_encoder(formatOut->video_codec);
if(!codecOut)
return;

streamOut = avformat_new_stream(formatCtxOut, codecOut);
if(!streamOut)
return;

ctx = streamOut->codec;

ctx->bit_rate = 400000;
ctx->width    = 352;
ctx->height   = 288;
ctx->time_base.den = 25;
ctx->time_base.num = 1;
ctx->gop_size      = 12;
ctx->pix_fmt       = AV_PIX_FMT_YUV420P;

if(ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO)
ctx->max_b_frames = 2;
if(ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO)
ctx->mb_decision = 2;


if(formatCtxOut->oformat->flags & AVFMT_GLOBALHEADER)
ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

if(streamOut) {
AVCodecContext *ctx;
ctx = streamOut->codec;

if(avcodec_open2(ctx, codecOut, NULL) < 0)
return;
}

if(!(formatCtxOut->flags & AVFMT_NOFILE))
if(avio_open(&formatCtxOut->pb, filename, AVIO_FLAG_WRITE) < 0)
return;

avformat_write_header(formatCtxOut, NULL);


// doit

pktIn = new AVPacket;
pktOut = new AVPacket;
av_init_packet(pktOut);
pktOut->data = 0;

frame = avcodec_alloc_frame();
if(!frame)
return;

for(;;) {
if(av_read_frame(formatCtxIn, pktIn) >= 0) {
av_dup_packet(pktIn);

int fff;
if(avcodec_decode_video2(codecCtxIn, frame, &fff, pktIn) < 0)
std::cout << "bad frame" << std::endl;

if(!fff)
return;  // ok

static int counter=0;
SaveFrame(frame, codecCtxIn->width, codecCtxIn->height, counter++);  // work fine

// here a segmentation fault is occured.
if(avcodec_encode_video2(streamOut->codec, pktOut, frame, &fff) < 0)
std::cout << "bad frame" << std::endl;
}
}
}


// only for testing
// add to ensure frame is valid
void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
FILE *pFile;
char szFilename[32];
int y;

// Open file
sprintf(szFilename, "frame%d.ppm", iFrame);
pFile=fopen(szFilename, "wb");
if(pFile==NULL)
return;

// Write header
fprintf(pFile, "P6\n%d %d\n255\n", width, height);

// Write pixel data
for(y=0; y<height; y++)
fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

// Close file
fclose(pFile);
}

*/

/*
к вопросу об отображении raw video
взято отсюда http://www.prog.org.ru/topic_21536_0.html

Ситуация такова: у меня идет видеопоток от ffmpeg в формате RGB (все в отдельном треде). Получаю очередной фрейм, преобразую в QImage, посылаю image сигналом виджету, отвечающему за прорисовку (в основном треде). Упрощенный код таков:

if (frame.bufferType == AvFrame::BUFFER_TYPE_PLAIN_ARRAY)
{
QImage::Format format = QImage::Format_RGB32;
int width = frame.format.params.video.frameWidth;
int height = frame.format.params.video.frameHeight;
int size = frame.buffer.size();
const uint8_t * data = frame.buffer.data ();
size_t frameSize = width * height;

....

m_frameData = new uint8_t[frameSize*4];
memcpy(m_frameData, frame.buffer.data (), size);

....

QImage image(m_frameData, width, height, format );
if(!image.isNull())
{
emit newFrame(image);
}
}

Виджет (производный от QWidget*) просто присваивает полученный в слоте QImage внутренней переменной, вызывает repaint() и вот код paintEvent:

if (frame.bufferType == AvFrame::BUFFER_TYPE_PLAIN_ARRAY)
{
QImage::Format format = QImage::Format_RGB32;
int width = frame.format.params.video.frameWidth;
int height = frame.format.params.video.frameHeight;
int size = frame.buffer.size();
const uint8_t * data = frame.buffer.data ();
size_t frameSize = width * height;

....

m_frameData = new uint8_t[frameSize*4];
memcpy(m_frameData, frame.buffer.data (), size);

....

QImage image(m_frameData, width, height, format );
if(!image.isNull())
{
emit newFrame(image);
}
}

//
Вывод через QPixmap отличается от QImage несущественно.

Существенно отличается вывод при использовании QGLWidget вместо QWidget. Профилировал, там вместо функций Qt для прорисовки используются dll-ки относящиеся к видеокарте т.е. по идее происходит аппаратное ускорение.  У меня две видюхи на ноуте стоят - через встроенную вывод по скорости практически такой де как средствами Qt, на дискретную (используются ati****.dll) быстрее.
Никаких дополнительных средств и опций  OpenGL не использую, просто меняю QWidget на QGLWidget и также рисую фрейм в paintEvent через QPainter.

Интересует, как правильно использовать QGlWidget для данных целей и какие настройки задавать?

//
Вычислил я, что CPU ест. Для этого отключил обработку фрейма и его прорисовку, везде где можно поставил заглушки.
Короче, по приходе каждого фрейма я посылаю сигнал. Соединенный с ним слот в видеовиджете вызывает repaint().
Так вот, вызов repaint() и ест примерно 8-10% CPU. При этом неважно, рисуется что-то в  paintEvent() или это просто заглушка.
Не пойму, почему так. Потому что событие о перерисовке посылается всем родительским классам виджета? Тогда как это исключить?
//
По идее "key_event->accept()" должен исключить передачу события родителю.
*/

//QImage::Format format = QImage::Format_RGB32;
