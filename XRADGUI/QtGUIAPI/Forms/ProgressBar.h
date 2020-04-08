//	Created by IRD on 06.2013
//  Version 3.0.4
//--------------------------------------------------------------
#ifndef XRAD__ProgressBarH
#define XRAD__ProgressBarH

//--------------------------------------------------------------

#include "ui_ProgressBar.h"
#include "StayOnTopDialog.h"
#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <string>

//--------------------------------------------------------------

// Класс виджета с индикатором прогресса
namespace XRAD_GUI
{

XRAD_USING
using namespace std;



class ProgressBar: public StayOnTopDialog, public Ui::ProgressBar
{
		Q_OBJECT
		double	max_value, current_value;
		enum class Mode { Integer, Floating, Exponential };
		Mode mode;
		physical_time	time_started, time_updated;
		const physical_time update_period;

		string	GenerateDetailedInfo();

		enum{n_indicator_divisions = 1024};
		double	indicator_quotient;//на сколько увеличивается индикатор за один шаг прогресса

		static	bool	geometry_stored;
		static	QRect	progress_geometry;
		void	SetWindowPosition();

		void	StoreGeometry();
		bool eventFilter(QObject *target,QEvent *event);
		bool	esc_pressed;

	public:
		ProgressBar(QString prompt, double count);
		~ProgressBar();
		void SetPosition(double position);
		void UpdateIndicator();
		void EndProgress();

	signals:
		void request_ResumeWorkThread();
		void request_PauseWorkThread();
		void request_Cancel();

	public slots:
		void PauseClicked();
		void CancelClicked();
		void QuitClicked();

	private:
		string	step_string(double current_value, double max_value) const;
};



class ProgressBarManager:
	public QObject
{
		Q_OBJECT
	private:
		ProgressBar	*progress;

	public:
		ProgressBarManager();
		~ProgressBarManager();
		// функции прогресса
		void Start(QString prompt, double count);
		void SetPosition(double position);
		void End();
		void Pause(){progress->PauseClicked();}

		bool Active(){return progress ? true:false;}

	private slots:
		void	do_Cancel();
};



//! \brief Остатки обработчика вложенных прогрессов. Сейчас обработка находится в "математической"
//! части
class ProgressBarCounter
{
	private:
		size_t level = 0;

	public:
		void	_start();
		//! \brief Noexcept
		void	_end();
		void _reset();
		int	_level();
};



}//namespace XRAD_GUI

#endif // XRAD__ProgressBarH
