﻿//	file TextWindow.h
//	Created by KNS on 08.2013
//--------------------------------------------------------------
#ifndef __TextWindow_h
#define __TextWindow_h

//--------------------------------------------------------------

#include "ui_TextWindow.h"
#include <VisualizationPlot.h>
#include "DataDisplayWindow.h"
#include "MultimodalFrameContainer.h"
#include <PixelNormalizers.h>
#include <QTimer>

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

#endif //__TextWindow_h