//	Created by KNS on 11.2013
//  Version 3.0.5
//--------------------------------------------------------------
#ifndef XRAD__data_display_window
#define XRAD__data_display_window

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include "IGUISettingsChanged.h"
#include <list>

//--------------------------------------------------------------

namespace XRAD_GUI
{

class GUIController;

XRAD_USING

// базовый класс для всех окон показа данных.
// пока его смысл только в том, чтобы назначать
// каждому объекту уникальный номер и по этому номеру
// задавать начальное положение окна на экране (лестницей или еще как)

class	DataDisplayWindow :
	public QDialog,
	public IGUISettingsChanged
{
	Q_OBJECT
	private:
		PARENT(QDialog);

		static	int	object_count;
		static	std::list<int>	object_ids;
		int	object_id;

		static void	Register(int id);
		static void	Unregister(int id);
		static int	GenerateId();

		//TODO перенести следующие поля в DataDisplayWindow. Сделать там ссылку на QDialog(*this), все управление этими полями сделать общим для всех окон данных включая графики и растры
		//	update. Сделано. Убрать лишние детали в наследных классах
		bool b_window_persistent = false;
		bool stay_on_top_flag = false;
		bool cached_allow_stay_on_top = false;

		void init();
		virtual void	SetWindowPosition();

		//! \name IGUISettingsChanged
		//! @{
	public:
		virtual void GUISettingsChanged() override;
		//! @}
		DataDisplayWindow(GUIController	&gc) : gui_controller(gc) { init(); }
		virtual ~DataDisplayWindow();


	protected:
		virtual void keyPressEvent(QKeyEvent *event);

		void UpdateWindowFlags();

		GUIController	&gui_controller;


		point2_PDT	GetCornerPosition();
public:
	void SetPersistent(bool persistent);
	void SetStayOnTop(bool stay_on_top);

signals:
	void signal_esc();
};



}// namespace XRAD_GUI

#endif // XRAD__data_display_window
