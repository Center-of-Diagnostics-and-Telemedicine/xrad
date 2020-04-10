//	Created by IRD on 08.2013
//  Version 3.0.5
//--------------------------------------------------------------
#ifndef XRAD__ImageWindowH
#define XRAD__ImageWindowH

//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include "ui_ImageWindow.h"
#include "DataDisplayWindow.h"
#include "FrameBitmapContainer.h"
#include "MultimodalFrameContainer.h"
#include <QTimer>

//--------------------------------------------------------------

namespace XRAD_GUI
{
XRAD_USING;



class ImageWindow :  public DataDisplayWindow, public Ui::ImageWindow
{
		Q_OBJECT
	public:
		PARENT(DataDisplayWindow);
		//
		//	интерфейс пользователя
		//
		ImageWindow(QString title, size_t vs, size_t hs, GUIController &gc);
		~ImageWindow();

		void	SetAxesScales(double in_z0, double in_dz, double in_y0, double in_dy, double in_x0, double in_dx);
		void	SetDefaultBrightness(double in_black, double in_white, double in_gamma);


		void	SetFrameLegend();
		void	SetupFrame(int in_frame_no, const void* data, display_sample_type pt);
		void	AddFrames(size_t n = 1);
		virtual void	SetWindowPosition() override;
		void	SetImageLabels(const QString &in_title, const QString &in_z_label, const QString &in_y_label, const QString &in_x_label, const QString &in_value_label);
		//void	SetWindowTitle(QString title);

	private:
		enum	brightness_change_reason_t
		{
			controls_not_changed,
			bw_point_control,
			bw_range_control,
			gamma_control
		};

		size_t freehand_brithess_change_counter;
		size_t	update_count;
		// возможны повторные вызовы функций, зависящих от появления Qt событий, приводящие к порче данных.
		// (UpdateBrightness, FreehandBrightnessChange)
		// исключаем с помощью счетчиков входа. использование mutex не подходит, т.к. все происходит
		// в одном потоке (рекурсия)

		//	исходные данные для отображения (набор двумерных массивов)
		//	приватные методы доступа к этим данным

		vector<shared_ptr<MultimodalFrameContainer>> frames;

		FrameBitmapContainer	current_frame_bitmap;

		size_t	n_frames;
		int	current_frame;
		const size_t n_rows, n_columns, n_samples_total;


		//	вставка пустого кадра в конец набора (наполнение данными функция SetupFrame)
		void EraseFrame(int in_frame_no);
		bool transposed() const { return (cb_transpose->checkState()==Qt::Checked); }

		bool paused;
		QTimer	*animation_timer;

		enum	animation_mode_t
		{
			e_forward,
			e_reverse,
			e_loop
		};
		animation_mode_t animation_mode;
		int direction;

		double x_zoom() const { return transposed() ? y_zoom_box->value() : x_zoom_box->value(); }
		double y_zoom() const { return transposed() ? x_zoom_box->value() : y_zoom_box->value(); }

		double	internal_black, internal_white;

		double	find_order(double value);
		double bw_order() const { return pow(10., bw_order_box->value()); };
		double bw_range_order() const { return pow(10., bw_range_order_box->value()); };

		// если в боксе напрямую введено число 10 и более, воспринимаем это значение, игнорируя порядок.
		// на практике очень неудобно вводить диапазоны вида (-2e3--200) в виде (-2e3--.2e3)
		// исхожу еще из того, что при экспоненциальной записи не принято указывать мантиссу менее 1 и более 10. кнс
		double	box_interpret(double value, double order) const { return fabs(value) >= 10 ? value : value*order; }
		double control_black() const { return box_interpret(black_point_box->value(), bw_order()); }
		double control_white() const { return box_interpret(white_point_box->value(), bw_order()); }
		double control_bw_center() const { return box_interpret(bw_center_box->value(), bw_order()); }
		double control_bw_range() const { return box_interpret(bw_range_box->value(), bw_range_order()); }


		double control_gamma() const { return gamma_box->value(); }
		void	UpdateBrightness(double in_black, double in_white, double in_gamma);

		//	метаданные (шаг по координатам, названия координат и величины, диапазон яркости и гамма)
		QString	z_label, y_label, x_label, value_label;
		double	z0, dz, x0, dx, y0, dy;
		double	default_black, default_white, default_gamma;


		// обработчики событий

		bool eventFilter(QObject *target, QEvent *event);
		void AnalyzeBrightnessChangeControls(QObject *target, QEvent *event);
		void AnalyzeUpdateEvent(QObject *target, QEvent *event);

		void RasterEvent(QEvent *event);
		void UpdateRaster();
		void closeEvent(QCloseEvent *event);

		void KeyPressAnalyzer(QObject *target, QEvent * event);


		enum	mouse_drag_mode_t
		{
			mouse_drag_none,
			mouse_drag_value_analyze,
			mouse_drag_contrast_change
		};

		mouse_drag_mode_t	mouse_drag_mode;

		// режим отображения координат и текущего значения

		void StartCurrentValueDraw(QEvent *event);
		void DrawCurrentValue(QEvent *event);
		void EndCurrentValueDraw(QEvent *event);
		point2_I32	LocalMousePosition(QEvent *event);

		//	режим изменения яркости и контрастности

		//brightness_change_reason_t brightness_change_reason;

		point2_I32  start_cursor_position;
		double	start_bw_center, start_bw_range;
		double	start_blackpoint, start_whitepoint;

		// Изменение яркости по положению курсора мыши
		void StartFreehandBrightnessChange(QEvent *event); // включение режима
		void FreehandBrightnessChange(QEvent *event);
		void EndFreehandBrightnessChange(QEvent *event); // выключение режима

		void ResetBrightnessControlsLimits();
		void SetBrightnessControlsLimits();
		void SetBrightnessContrastCursor();

		//	слоты и сигналы

		protected slots:
		void ShowFrame(int in_frame_no);
		void RebuildPixmap();
		void AdjustBrightnessControls(brightness_change_reason_t brightness_change_reason);
		void ResetBrightnessDefault();
		void ResetBrightnessFull();
		void SaveFrame();
		void SaveVideo();
		void AnimationUpdate();
		void UpdateAnimationTimer();

	signals:
		void signal_esc();
};



}//namespace XRAD_GUI

#endif //XRAD__ImageWindowH
