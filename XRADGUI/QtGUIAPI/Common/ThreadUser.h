//	file ThreadUser.h
//	Created by IRD on 05.2013
//  Version 3.0.2
/*!
* \file
* \brief Описание класса ThreadUser
* \details что-то добавить?.
* \version 3.0.2.
* \date 05.2013
* \pre инициализуеруется в классе XRADQtInterface
*/
//--------------------------------------------------------------
#ifndef XRAD__WorkThreadH
#define XRAD__WorkThreadH
//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include "TextWindow.h"
#include "GraphWindow.h"
#include "ImageWindow.h"
#include "XRADGUIAPIDefs.h"
#include <XRADGUI/Sources/Core/GUICore.h>
#include <XRADBasic/Sources/Containers/DataArray.h>
#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <atomic>

namespace XRAD_GUI
{

XRAD_USING

//--------------------------------------------------------------

class	GUIController; // TODO

//--------------------------------------------------------------

/*!
	\class ThreadUser
	\brief Класс реализует основной поток для пользовательских вычислений.
*/
class ThreadUser: public QThread
{
		Q_OBJECT

	private:
		virtual void run();

	public:
		void	WaitForNonModalDialog();

	public:
		//! \name конструктор\деструктор
		//! \{
		GUIController &gui_controller;
		ThreadUser(GUIController &in_gui_controller);
		~ThreadUser();
		//! \}
		bool	IsRunning() const{return workthread_is_running;};

		// передается в функцию Suspend в зависимости от того, какое окно останавливает рабочий поток
		enum suspend_mode
		{
			suspend_for_dialog, suspend_for_data_analyze
		};

		// окно, ради которого остановлен рабочий поток.
		// если нажато esc, это окно закрывается
		QPointer<QDialog> suspending_window;

	protected:
		QWaitCondition waitCondition; //!< переменная для координации потоков (ввод в режим ожидания,разблокирования ресурсов)
		bool	workthread_is_running;
		atomic_bool break_on_gui_return = false;

	public:
		bool BreakOnGUIReturn() { return break_on_gui_return.load(); }
		void SetBreakOnGUIReturn(bool value) { break_on_gui_return.store(value); }

		// сигналы интерфейсных функций (см. XRADGUI.h) и методы их вызова
	public:
		void Sleep(double time_in_seconds);
		//TODO все Doxygen комментарии в XRADGUI

		//! \brief Сделать необходимые обновления, но не проверять cancel, quit и т.п.:
		//! не вызывает исключений
		void ForceUpdateGUI_noexcept(const physical_time &update_interval);

		//! \brief Следующий шаг
		void ForceUpdateGUI(const physical_time &update_interval);

	signals:
		//! \name Сигналы остановки\возобновление потока
		//!	\{

		//! \brief включают команды меню, допустимые во время паузы (рабочий поток приостановлен)
		void request_InterfacePaused();
		//!	\brief включают команды меню, допустимые во время активных вычислений (рабочий поток выполняется)
		void request_InterfaceResumed();

		//!	\}

		//!	\name Сигналы интерфейсных функций
		//! \{

		//! \brief готовит интерфейс к выводу диалогового окна
		void request_Dialog();

		void request_Pause();
		void request_ShowMessage(QMessageBox::Icon icon, QString title, QString text);

		void request_ShowString(QString title, QString text, display_text_dialog_status status);
		void request_ShowIntegral(QString title, ptrdiff_t value);
		void request_ShowFloating(QString title, double value);

		//! \sa GetString
		void	request_GetString(QString& result, QString prompt, QString default_value);
		//! \sa GetIntegral
		void	request_GetIntegral(ptrdiff_t &result, QString prompt, ptrdiff_t min_value, ptrdiff_t max_value, ptrdiff_t default_value, size_t width, bool allow_out_of_range);
		//! \sa GetFloating
		void	request_GetFloating(double &result, QString prompt, double min_value, double max_value, double default_value, bool allow_out_of_range);

		//! \sa GetButtonDecision
		void	request_GetButtonDecision(size_t &result, const QString &prompt, const vector<QString> &buttons, size_t default_button, size_t cancel_button);
		//! \sa GetRadioButtonDecision
		void	request_GetRadioButtonDecision(size_t &in_result,	const QString &prompt, const vector<QString> &buttons, size_t default_button);
		//! \sa GetCheckboxDecision
		void	request_GetCheckboxDecision(const QString &prompt, const vector<pair<QString, bool*> > &boxes);
		//! \sa GetFileName
		bool request_GetFileName(file_dialog_mode flag, QString &return_file_name_buf, const QString &prompt, const QString &default_filename, const QString &type);
		//! \sa GetFolderName
		bool request_GetFolderName(file_dialog_mode flag, QString &return_folder_name_buf, const QString &prompt, const QString &default_path);
		//! \sa StartProgress
		void request_StartProgress(QString prompt, double count);
		//! \sa NextProgress
		void request_SetProgressPosition(double);
		//! \sa EndProgress
		void request_EndProgress();

		void request_UpdateConsole();

		bool	request_CloseDataWindow(QDialog *window);
		bool	request_HideDataWindow(QDialog *window);
		bool	request_SetDataWindowTitle(QDialog *window, const QString &title);

		//! \sa ShowGraph
		XRAD_GUI::GraphWindow* request_CreateGraph(const QString &title, const QString &y_label, const QString &x_label);
		bool request_SetupGraphCurve(GraphWindow *graph, int curve_no, const DataArray<double> &data_y, const DataArray<double> &data_x, const QString& curve_name);
		bool request_SetupGraphLabels(GraphWindow *graph, const QString &title, const QString &y_label, const QString &x_label);
		bool request_SetupGraphTitle(GraphWindow *graph, const QString &title);
		bool request_SetGraphScale(GraphWindow *, const range2_F64&);
		bool request_GetGraphScale(const GraphWindow *, range2_F64 &);
		bool request_SetGraphStyle(GraphWindow *, graph_line_style style, double in_line_width);
		bool request_SaveGraphPicture(GraphWindow *, const QString &filename_with_extension);

		//! \sa ShowImage
		XRAD_GUI::ImageWindow*	request_CreateRasterImageSet(const QString &title, int vs, int hs);

		bool	request_AddImageFrames(ImageWindow *img, size_t n_frames);
		bool	request_SetupImageFrame(ImageWindow *, int, const void*, display_sample_type);
		bool	request_SetupImageLabels(ImageWindow *img, const QString &title, const QString &z_label,const QString &y_label, const QString &x_label, const QString &value_label);
		bool	request_SetupImageDefaultRanges(ImageWindow *img, double min_value, double max_value, double gamma);
		bool	request_SetImageAxesScales(ImageWindow*, double, double, double, double, double, double);

		XRAD_GUI::TextWindow *request_CreateTextDisplayer(const QString &title);
		bool	request_SetFixedWidth(TextWindow *, bool fixed_width);
		bool	request_SetEditable(TextWindow *, bool editable);
		bool	request_SetFontSize(TextWindow *, double size);
		bool request_SetPersistent(TextWindow *, bool persistent);
		bool request_SetStayOnTop(TextWindow *, bool stay_on_top);
		bool	request_SetText(TextWindow *, const QString &text);
		bool	request_SetTitle(TextWindow *, const QString &title);
		QString	request_GetText(const TextWindow *);

		//! \sa ShowDialog
		bool request_ShowDataWindow(QDialog *dialog, bool suspend);

		//! \}

	public slots:
		//! \name реализация останова/возобновления работы потока
		//! \{

		//!	\brief Пауза \sa request_InterfacePaused
		void Suspend(suspend_mode mode);
		//!	\brief Возобновление работы \sa request_InterfaceResumed
		void Resume();

		//! \}
};



}//namespace XRAD_GUI

//--------------------------------------------------------------
#endif //XRAD__WorkThreadH
