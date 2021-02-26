//	file TextWindow.h
//	Created by KNS on 08.2013
//--------------------------------------------------------------
#ifndef XRAD__File_TextWindow_h
#define XRAD__File_TextWindow_h

//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include "ui_TextWindow.h"
#include "DataDisplayWindow.h"

//--------------------------------------------------------------

namespace XRAD_GUI
{
using namespace xrad;



class TextWindow : public DataDisplayWindow, public Ui::TextWindow
{
	Q_OBJECT

		void	init(const QString &title);

	public:

		PARENT(DataDisplayWindow);

		//	интерфейс пользователя

		TextWindow(const QString &title, GUIController &gc) : DataDisplayWindow(gc){ init(title); }
		virtual ~TextWindow();

		void	SetText(const QString &in_text);
		QString	GetText() const;
		void	SetWindowTitle(const QString &in_title);

		void	SetEditable(bool);
		void	SetFixedWidth(bool);
		void	SetFontSize(double);



	private:
		// обработчики событий

		virtual bool eventFilter(QObject *target, QEvent *event);
		virtual void closeEvent(QCloseEvent *event);
		virtual void showEvent(QShowEvent *){ UpdateFontStyle(); }
		virtual void keyPressEvent(QKeyEvent *event);

		//void	SetWindowPosition();

		//	слоты и сигналы

		protected slots:
		void SaveTextFile();
		void WrapMode();
		void UpdateFontStyle();
};



}//namespace XRAD_GUI

#endif //XRAD__File_TextWindow_h
