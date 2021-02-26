/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	Created by IRD on 05.2013
//  Version 3.0.2
//--------------------------------------------------------------
#ifndef XRAD__DialogsH
#define XRAD__DialogsH
//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include "ProgressBar.h"
#include "StayOnTopDialog.h"
#include "GraphWindow.h"
#include "ImageWindow.h"
#include "TextWindow.h"
#include "ConsoleWindow.h"
#include "XRADGUIAPIDefs.h"
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include <XRADBasic/Sources/Containers/DataArray.h>

namespace XRAD_GUI
{

XRAD_USING

//--------------------------------------------------------------

class GUIController; // TODO

//--------------------------------------------------------------

// класс предназначен для создания диалогов в GUI-потоке

class ThreadGUI:
	public QObject
{
		Q_OBJECT
	private:
		using self = ThreadGUI;
		ProgressBarManager progress_bar_manager;

	public:
		GUIController &gui_controller;

		ThreadGUI(GUIController &in_gui_globals, QObject *parent);
		~ThreadGUI();

		inline void SetMainWindow(ConsoleWindow *w) { main_window = w; }
	public:
		bool ActiveProgressBar(){return progress_bar_manager.Active();}
	private:
		void	ExecuteActiveDialog(StayOnTopDialog *dialog);
		QPointer<StayOnTopDialog> active_dialog;

	public slots:
		void	do_Pause(); // вызов формы с паузой
		//void do_ShowMessage(QMessageBox::Icon icon, QString title, QString text); // окно с сообщением

		// модальная форма отображения (в перспективе только для критических ошибок)
		void	do_ShowMessage(QMessageBox::Icon icon, QString title, QString text); // окно с сообщением

		//	немодальные формы отображения
		void	do_ShowString(QString title, QString text, display_text_dialog_status status); // окно с выводом текста
		void	do_ShowIntegral(QString title, ptrdiff_t value);
		void	do_ShowFloating(QString title, double value);

		// формы пользовательского ввода
		void	do_GetString(QString& result, QString prompt, QString default_value);
		void	do_GetIntegral(ptrdiff_t &result, QString prompt, ptrdiff_t min_value, ptrdiff_t max_value, ptrdiff_t default_value, size_t width, bool allow_out_of_range);
		void	do_GetFloating(double &result, QString prompt, double min_value, double max_value, double default_value, bool allow_out_of_range);
		// динамическая форма с командными кнопками
		void	do_GetButtonDecision(size_t &result, const QString &prompt, const vector<QString> &buttons,
				size_t default_button, size_t cancel_button);
		// динамическая форма с флажками
		void	do_GetCheckboxDecision(const QString &prompt, const vector<pair<QString, bool*> > &boxes);
		// динамическая форма с радиокнопками
		void	do_GetRadioButtonDecision(size_t &result, const QString &prompt, const vector<QString> &buttons, size_t default_button);
		// форма выбора/сохранения файла
		//(если flag = "open" - выбор имени файла для дальнейшего его открытия,
		//      flag = "save" - выбор директории и указание имени для сохранения файла)
		//	меняю со строки на enum, строковый параметр небезопасен
		bool do_GetFileName(file_dialog_mode flag, QString &return_file_name_buf, const QString &prompt, const QString &default_filename, const QString &type);

		bool do_GetFolderName(file_dialog_mode flag, QString &return_folder_name_buf, const QString &prompt, const QString &default_path);

		// функции прогресса
		void do_StartProgress(QString prompt, double count);
		void do_SetProgressPosition(double position);
		void do_EndProgress();

		void do_UpdateConsole();


		// рисование графика
		GraphWindow*	do_CreateGraph(const QString &title, const QString &y_label, const QString &x_label);
		bool	do_SetupGraphCurve(GraphWindow *graph, int curve_no, const DataArray<double> &data_y, const DataArray<double> &data_x, const QString& curve_name);
		bool	do_SetupGraphLabels(GraphWindow *graph, const QString &title, const QString &y_label, const QString &x_label);
		bool	do_SetupGraphTitle(GraphWindow *graph, const QString &title);
		bool	do_GetGraphScale(const GraphWindow *, range2_F64&);
		bool	do_SetGraphScale(GraphWindow *, const range2_F64&);
		bool	do_SetGraphStyle(GraphWindow *, graph_line_style, double in_line_width);

		bool	do_SaveGraphPicture(GraphWindow *, const QString &filename_with_extension);

		// вывод растрового изображения
		ImageWindow*	do_CreateRasterImageSet(const QString &title, int vs, int hs);
		bool	do_AddImageFrames(ImageWindow *img, size_t n_frames);
		bool	do_SetupImageFrame(ImageWindow *, int, const void*, display_sample_type);
		bool	do_SetupImageLabels(ImageWindow *img, const QString &title, const QString &z_label,const QString &y_label, const QString &x_label, const QString &value_label);
		bool	do_SetupImageDefaultRanges(ImageWindow *img, double min_value, double max_value, double gamma);
		bool	do_SetImageAxesScales(ImageWindow *img, double z0, double dz, double y0, double dy, double x0, double dx);

		TextWindow *do_CreateTextDisplayer(const QString &title);
		bool	do_SetFixedWidth(TextWindow *, bool fixed_width);
		bool	do_SetEditable(TextWindow *, bool editable);
		bool	do_SetFontSize(TextWindow *, double size);
		bool	do_SetPersistent(TextWindow *tw, bool persistent);
		bool	do_SetStayOnTop(TextWindow *tw, bool stay_on_top);
		bool	do_SetText(TextWindow *, const QString &text);
		bool	do_SetTitle(TextWindow *, const QString &title);
		QString	do_GetText(const TextWindow *);

		bool	do_ShowDataWindow(QDialog *dialog_ptr, bool suspend);
		bool	do_CloseDataWindow(QDialog *window);
		bool	do_HideDataWindow(QDialog *window);
		bool	do_SetDataWindowTitle(QDialog *window, const QString &title);


		void do_AcceptAndContinue();

		//void finishDialogs(QObject* obj);

	public:
		//! \brief Выполнить callback в потоке UI. Обрабатывает исключения
		void WorkerUICallback(function<void ()> &&action);
	private:
		struct UICallbackParams
		{
			function<void ()> action;
		};
	signals:
		void request_UICallback(UICallbackParams *params);
	private slots:
		void do_UICallback(UICallbackParams *params);

	public:
		void ExecuteDialog(StayOnTopDialog *dialog);

	public:
		/*!
			\brief Обозначить начало цикла обработки (модального) диалога

			По окончании цикла необходимо вызывать EndDialogLoop() с тем же id.

			\param id Уникальный идентификатор. Не следует использовать NULL.
		*/
		void BeginDialogLoop(const void *id,
				QDialog *dialog,
				function<void ()> &&end_dialog);

		/*!
			\brief Обозначить конец цикла обработки (модального) диалога, парная функция к
			BeginDialogLoop()

			\param id Допускается передача незарегистрированного id.
		*/
		void EndDialogLoop(const void *id);

	private:
		ConsoleWindow *main_window = nullptr; //!< Указатель на главное окно интерфейса
		vector<tuple<const void*, QPointer<QDialog>, function<void ()>,
				ConsoleWindow::ThreadState>> active_dialogs;
};



}//namespace XRAD_GUI

//--------------------------------------------------------------

Q_DECLARE_METATYPE(std::string)

#endif // XRAD__DialogsH
